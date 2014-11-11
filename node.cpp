#include "node.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <list>
#include <iterator>
#include <algorithm>

#include <boost/mpi/nonblocking.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/range/adaptor/filtered.hpp>

#include "main.hpp"
#include "utils.hpp"
#include "matrix.hpp"

Node::Node(){
    // set rank
    node_rank = std::rand() % NODE_MAX_RANK;
#ifdef SEED
    // re-seed with default seed because mpi messes up random
    std::srand(SEED);
#endif
}

void Node::print(){
    std::cout << "[" << world.rank() << "] rank: " << node_rank << " " << get_info() << std::endl;
}

Ring_node::Ring_node(){
    next = (world.rank() + 1) % world.size();
    prev = (world.rank() == 0) ? world.size() - 1 : (world.rank() - 1) % world.size();
}

std::string Ring_node::get_info(){
    std::ostringstream oss;
    oss << "next: " << next << " prev: "<< prev;
    return oss.str();
}

void Ring_node::leader_elect(){
    // perpare message
    MSG_ring_leader_elect msg;
    msg.sender              = world.rank();
    msg.leader              = world.rank();
    msg.leader_node_rank    = node_rank;
    MSG_ring_leader_elect msg_ring;
    msg_ring.sender              = world.rank();
    msg_ring.leader              = world.rank();
    msg_ring.leader_node_rank    = node_rank;
    do {
        // send message to ring
        world.send(next,msg_ring.tag(), msg_ring);
        world.recv(prev, msg_ring.tag(), msg_ring);
        msg_ring.print();
        msg_ring.merge(msg);
        msg.merge(msg_ring);
    }while(msg_ring.sender != world.rank()); // message went around
    std::cout << "Leader: " << msg.leader << std::endl;
}

Tree_node::Tree_node(){
    if(world.rank() == 0){
        // rank 0 build tree
        std::vector< std::vector< int > > levels;
        std::vector< int > nodes;
        std::vector< std::vector<int> > connections(world.size());
        for(int i=0;i<world.size();i++){
            nodes.push_back(i);
        }
        std::random_shuffle( nodes.begin(), nodes.end(), myrandom );
        // level 0 must only have 1 node
        std::vector< int > l0;
        l0.insert(l0.begin(),nodes.front());
        levels.insert(levels.begin(),l0); 
        // iterate over all other nodes
        for (std::vector< int >::iterator it = nodes.begin()+1; it != nodes.end(); ++it){
            int add_level = rand() % levels.size();
            if(add_level == 0){ // we can't add to root -> add new level
                std::vector < int > l_new;
                levels.insert(levels.end(),l_new);
                add_level = (levels.size()-1);
            }
            levels[add_level].push_back(*it);
        }
        // connect levels
        for (std::vector< std::vector< int > >::reverse_iterator levels_it = levels.rbegin(); (levels_it+1) != levels.rend(); ++levels_it){
            std::vector< int > level_l = *levels_it;
            std::vector< int > level_h = *(levels_it+1);
            for (std::vector< int >::iterator nodes_it = level_l.begin(); nodes_it != level_l.end(); ++nodes_it){
                // connect node to a random node from higher level
                int node_l = *nodes_it;
                int node_h = level_h[std::rand() % level_h.size()];
                connections[node_l].push_back(node_h);
                connections[node_h].push_back(node_l);
            }
        }
        // send information to all nodes
        for (std::vector< int >::iterator it = nodes.begin(); it != nodes.end(); ++it){
            int node = *it;
            MSG_tree_connect msg;
            msg.connected = connections[node];
            world.send(node,msg.tag(),msg);
        }
        
    }
    // listen for tree information from rank 0
    MSG_tree_connect msg;
    world.recv(0, msg.tag(), msg);
    connected = msg.connected;
}

std::string Tree_node::get_info(){
    std::ostringstream oss;
    oss << "Connected to: ";
    for (std::vector< int >::iterator it = connected.begin(); it != connected.end(); ++it){
        oss << "( " << *it << " )";
    }
    
    return oss.str();
}

void Tree_node::leader_elect(){
    MSG_tree_leader_elect msg;
    msg.leader                      = world.rank();
    msg.leader_node_rank            = node_rank;
    // vecot for all connected nodes
    std::vector< Leader_elect_node > nodes(connected.size());
    // read messages from all peers
    for (unsigned int i=0;i < connected.size();++i){
        Leader_elect_node& node = nodes[i];
        node.node_rank          = connected[i];
        node.got_message        = false;
        node.req                = world.irecv(node.node_rank, node.elect_message.tag(), node.elect_message);
    }
    // set initial size for elect messages
    int elect_messages_left = nodes.size();
    std::cout << "Start collecting leader_elect messages" << std::endl;
    // operator used for selecting nodes from which we didn't got a message
    while(elect_messages_left > 1){
        // get all the outstanding requests
        std::vector< Leader_elect_node >::iterator it = std::find_if(nodes.begin(), nodes.end(), [](Leader_elect_node& n ) { return ! n.got_message; });
/*
        std::list< mpi::request > reqs;
        for(auto & node : nodes){
            if(!node.got_message){
                reqs.push_back(node.req);
            }
        }
        // wait until we get something
        mpi::wait_some(reqs.begin(), reqs.end());
*/
//debug_break();
        for (; it != nodes.end(); ++it){
            Leader_elect_node& node = *it;
            if(node.req.test()){
                MSG_tree_leader_elect& msg_in = node.elect_message;
                // we got something
                std::cout << "Msg from: "<< node.node_rank << std::endl;
                msg_in.print();
                // merge result 
                msg.merge(msg_in);
                // one more
                node.got_message = true;
                elect_messages_left--;
            }
        }
    }
    std::cout << "Stop collecting leader_elect messages" << std::endl;
    // do I know the result?
    if(elect_messages_left > 0){
        // nope: not yet
        // get leftover peer
        Leader_elect_node *leftover_node;
        std::list < mpi::request > leftover_reqs;
        for (unsigned int i=0;i < nodes.size();++i){
            Leader_elect_node& node = nodes[i];
            if(node.got_message == false){
                leftover_node = &node;
                leftover_reqs.push_front(node.req);
            }
        }
        std::cout << "Sending my message to: " << leftover_node->node_rank << std::endl;
        msg.print();
        world.isend(leftover_node->node_rank, msg.tag(), msg);
        // receive propagate
        // wait for something to happen if we didn't already get something
        if(!leftover_node->req.test()){
            mpi::wait_all(leftover_reqs.begin(), leftover_reqs.end());
        }
        // did we get a leader elect message?
        std::cout << "Got election msg" << std::endl;
        msg.merge(leftover_node->elect_message);
        // propagete to sub-nodes
        for (std::vector< int >::iterator it = connected.begin(); it != connected.end(); ++it){
            if(*it != leftover_node->node_rank){
                std::cout << "Propagate to: " << *it << std::endl;
                world.send(*it, msg.tag(), msg);
            }
        }
    }else{
        // yes: my result is the real result
        // broadcast result to all connected
        for (std::vector< int>::iterator it = connected.begin(); it != connected.end(); ++it){
            std::cout << "Propagate to: " << *it << std::endl;
            world.send(*it, msg.tag(), msg);
        }
    }
    std::cout << "Leader: " << msg.leader << std::endl;
}

void Tree_node::matrix_calc(){
    // get a random matrix
    Matrix_2x2 matrix;
    matrix.rnd();
    matrix.print();
    Matrix_2x2 matrix_out;
    // register collective
    mpi::reduce(world,matrix,matrix_out,std::multiplies<Matrix_2x2>(),0);
    // print result
    if(world.rank() == 0){
        matrix_out.print();
    }
}
