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
    // set random order
    rnd_order = std::rand() % NODE_MAX_RND_ORDER;
    // re-seed with default seed because mpi messes up random
    std::srand(SEED);
}

void Node::print(){
    std::cout << "[" << world.rank() << "] order: " << rnd_order << " " << get_info() << std::endl;
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

void Ring_node::leader_elect(MSG_ring_leader_elect& msg){
    // perpare message
    msg.sender              = world.rank();
    msg.leader              = world.rank();
    msg.leader_rnd_order    = rnd_order;
    MSG_ring_leader_elect msg_ring;
    msg_ring.sender             = world.rank();
    msg_ring.leader             = world.rank();
    msg_ring.leader_rnd_order   = rnd_order;
    do {
        // send message to ring
        world.send(next,msg_ring.tag(), msg_ring);
        world.recv(prev, msg_ring.tag(), msg_ring);
        msg_ring.merge(msg);
        msg.merge(msg_ring);
    }while(msg_ring.sender != world.rank()); // message went around
    std::cout << "Leader: " << msg.leader << std::endl;
}

Tree_node::Tree_node(int scenario){
    // rank 0 build tree
    if(world.rank() == 0){
        switch(scenario){
            case -1:{
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
            break;
            default:
            case 0:
                // do nothing
                // send information to all nodes
                for(int node=0;node<world.size();node++){
                    MSG_tree_connect msg;
                    world.send(node,msg.tag(),msg);
                }
            break;
        }
    }
    // listen for tree information from rank 0
    MSG_tree_connect msg;
    world.recv(0, msg.tag(), msg);
    connected = msg.connected;
}

std::string Tree_node::get_info(){
    std::ostringstream oss;
    oss << "connected to: ";
    for (std::vector< int >::iterator it = connected.begin(); it != connected.end(); ++it){
        oss << "( " << *it << " )";
    }
    
    return oss.str();
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

Graph_node::Graph_node(int scenario){
    if(world.rank() == 0){
        switch(scenario){
            case -1:{
                // rank 0 build graph
                std::vector< std::vector< Graph_edge > > edges(world.size());
                for(int from=0;from<world.size();++from){
                    for(int to=0;to<world.size();++to){
                        if( from != to ){ // no self connections
                            // check if this edge already exists
                            bool edge_exists = false;
                            std::vector< Graph_edge >& edges_from = edges[from];
                            if(edges_from.size() > 0){
                                for(std::vector< Graph_edge >::iterator it = edges_from.begin(); it != edges_from.end(); ++it){
                                    Graph_edge& edge_from = *it;
                                    if(edge_from.to == to){
                                        edge_exists = true;
                                        break;
                                    }
                                }
                            }
                            // add new random edges
                            if( !edge_exists &&
                                (std::rand() / (double)RAND_MAX) < GRAPH_EDGE_ADD_CHANCE){
                                int weight = std::rand() % EDGE_MAX_WEIGHT;
                                // add edge for from
                                Graph_edge edge_from;
                                edge_from.to = to;
                                edge_from.weight = weight;
                                edges[from].push_back(edge_from);
                                // add edge for to
                                Graph_edge edge_to;
                                edge_to.to = from;
                                edge_to.weight = weight;
                                edges[to].push_back(edge_to);
                            }
                        }
                    }
                }
                // send information to all nodes
                for(int node=0;node<world.size();node++){
                    std::vector< Graph_edge > node_edges = edges[node];
                    MSG_graph_connect msg;
                    msg.edges = node_edges;
                    world.send(node,msg.tag(),msg);
                }
            }
            break;
            default:
            case 0:
                // do nothing
                // send information to all nodes
                for(int node=0;node<world.size();node++){
                    MSG_graph_connect msg;
                    world.send(node,msg.tag(),msg);
                }
            break;
            case 1:{
                /*
                * our default graph:   
                *         [0]    [1]             [2]    
                *        /  |   /    \          /  |
                *      7   16  2      10      13   9
                *     /     | /          \   /     |
                *  [3]__3__[4]__5__[5]_18_[6]__12__[7] 
                *     \     | \     |    / \        \
                *      1   15  6    17  4   19      14
                *        \  |    \  | /      \        \ 
                *          [8]_13__[9]__8__[10]__11__[11]
                *
                *
                *  weight matrix 
                * (similar to adjacency matrix but with the
                * following meaning: 
                * if (m[i][k] == 0) --> no edge between node i and node k
                * else there is an edge with weight  = m[i][k]
                */
                
                int admat[12][12] = { 
                    // 0   1   2   3   4   5   6   7   8   9  10  11
                    {  0,  0,  0,  7, 16,  0,  0,  0,  0,  0,  0,  0}, // 0
                    {  0,  0,  0,  0,  2,  0, 10,  0,  0,  0,  0,  0}, // 1
                    {  0,  0,  0,  0,  0,  0, 13,  9,  0,  0,  0,  0}, // 2
                    {  7,  0,  0,  0,  3,  0,  0,  0,  1,  0,  0,  0}, // 3 
                    { 16,  2,  0,  3,  0,  5,  0,  0, 15,  6,  0,  0}, // 4
                    {  0,  0,  0,  0,  5,  0, 18,  0,  0, 17,  0,  0}, // 5
                    {  0, 10, 13,  0,  0, 18,  0, 12,  0,  4, 19,  0}, // 6
                    {  0,  0,  9,  0,  0,  0, 12,  0,  0,  0,  0, 14}, // 7
                    {  0,  0,  0,  1, 15,  0,  0,  0,  0, 13,  0,  0}, // 8
                    {  0,  0,  0,  0,  6, 17,  4,  0, 13,  0,  8,  0}, // 9
                    {  0,  0,  0,  0,  0,  0, 19,  0,  0,  8,  0, 11}, //10
                    {  0,  0,  0,  0,  0,  0,  0, 14,  0,  0, 11,  0}, //11
                };  
                if(world.size() > 12){
                    std::cerr << "MPI cluster too big for this scenario";
                    exit(EXIT_FAILURE);
                }
                for(int node=0;node<world.size();node++){
                    MSG_graph_connect msg;
                    for(int to_node=0;to_node<world.size();to_node++){
                        int weight = admat[node][to_node];
                        if(weight > 0){
                            Graph_edge edge;
                            edge.to = to_node;
                            edge.weight = weight;
                            msg.edges.push_back(edge);
                        }
                    }
                    world.send(node,msg.tag(),msg);
                }
            }
            break;
        }
    }
    // listen for tree information from rank 0
    MSG_graph_connect msg;
    world.recv(0, msg.tag(), msg);
    edges = msg.edges;
}

std::string Graph_node::get_info(){
    std::ostringstream oss;
    oss << "Connected to: ";
    for (std::vector< Graph_edge >::iterator it = edges.begin(); it != edges.end(); ++it){
        Graph_edge& edge = *it;
        oss << "( " << edge.to << " ; " << edge.weight << " )";
    }
    
    return oss.str();
}

void Graph_node::boruvka_mst(){
    // listen on all graph edges for mst_grow messages
    class Mst_candidate {
        public:
            Graph_edge edge;
            bool connected;
    };
    std::vector< Mst_candidate > mst_candidates(edges.size());
    for(unsigned int i = 0; i < edges.size();++i){
        Mst_candidate& mst_candidate    = mst_candidates[i];
        mst_candidate.edge              = edges[i];
        mst_candidate.connected         = false;
    }
    // call constructor which does nothing
    Tree_node tree_node(0);
    MSG_graph_leader_elect msg;
    do {
        // check if we have still candidates not connected
        Graph_edge min_edge;
        min_edge.to     = -1;
        min_edge.weight = -1;
        for(std::vector< Mst_candidate >::iterator it = mst_candidates.begin();it != mst_candidates.end();++it){
            Mst_candidate& mst_candidate = *it;
            if(msg.tree_nodes.count(mst_candidate.edge.to) > 0){
                mst_candidate.connected = true;
            }else if(mst_candidate.connected == false){
                if( min_edge.to == -1 // first
                    || mst_candidate.edge.weight < min_edge.weight
                ){
                    min_edge = mst_candidate.edge;
                }
            }
        }
        msg.min_edge                    = min_edge;
        msg.min_edge_min_node_rank      = (world.rank() < min_edge.to ) ? world.rank() : min_edge.to;
        msg.tree_nodes.insert(world.rank());
        // search where to grow mst
        std::cout << "Tree:" << std::endl;
        tree_node.print();
        tree_node.leader_elect(msg);
        // grow mst if i'm the leader
        if( msg.min_edge.to != -1
            && msg.leader == world.rank() ){
            MSG_graph_mst_grow grow_message;
            // do i have to make the first step?
            if(msg.min_edge_min_node_rank == world.rank()){
                // yes i'm the man!
                std::cout << "Growing mst (master) to: " << min_edge.to << std::endl;
                world.send(min_edge.to, grow_message.tag(), grow_message);
                world.recv(min_edge.to, grow_message.tag(), grow_message);
            }else{
                // nope, others have to buy me drinks.
                std::cout << "Growing mst (slave) to: " << min_edge.to << std::endl;
                world.recv(min_edge.to, grow_message.tag(), grow_message);
                world.send(min_edge.to, grow_message.tag(), grow_message);
            }
            // add node
            msg.tree_nodes.insert(min_edge.to);
            tree_node.connected.push_back(min_edge.to);
        }
    }while(msg.min_edge.to != -1);
    std::cout << "Graph:" <<std::endl;
    print();
    std::cout << "MST:" << std::endl;
    tree_node.print();
}

