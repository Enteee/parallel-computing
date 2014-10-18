#include "node.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>

#include "msg.hpp"
#include "utils.hpp"

Node::Node(){
    // set rank
    node_rank = std::rand();
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
    do {
        // send message to ring
        world.send(next,msg.tag(),msg);
        world.recv(prev, msg.tag(), msg);
        if(msg.leader_node_rank < node_rank
            || ( msg.leader_node_rank == node_rank && msg.leader < world.rank())){
            msg.leader              = world.rank();
            msg.leader_node_rank    = node_rank;
        }
    }while(msg.sender != world.rank()); // message went around
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
        std::random_shuffle( nodes.begin(), nodes.end() );
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

}
