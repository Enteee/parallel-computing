#ifndef NODE_HPP
#define NODE_HPP

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include "msg.hpp"
#include "graph.hpp"

#define NODE_MAX_RND_ORDER 50
#define GRAPH_EDGE_ADD_CHANCE 0.9
#define EDGE_MAX_WEIGHT 50

using namespace boost;

/**
* Basic node
*/
class Node {
private:

protected:
    mpi::environment env;
    mpi::communicator world;
    virtual std::string get_info() = 0;
    int rnd_order;

public:
    Node();
    void print();
};

/**
* Node in a ring topology
*
* 1 -> 2 -> 3
* ^         |
* |         v
* 6 <- 5 <- 4
*/
class Ring_node : public Node {
private:
    int next;
    int prev;

protected:
    std::string get_info();

public:
    Ring_node();
    void leader_elect(MSG_ring_leader_elect& msg);
};

/**
* Node in a tree topology
* 1 - 2 - 3
* |
* 4 - 5      10
* |           |
* 6 - 7 - 8 - 9
*/
class Tree_node : public Node{
private:

protected:
    std::string get_info();

public:
    std::vector < int > connected;
    Tree_node(int scenario);
    Tree_node() : Tree_node(-1){};
    
    template<class MLE> void leader_elect(MLE& msg);
    void matrix_calc();
};

template<class MLE> void Tree_node::leader_elect(MLE& msg){
    class Leader_elect_node {
    public:
        int node_rank;
        MLE elect_message;
        bool got_message;
        mpi::request req;
    };

    msg.leader                      = world.rank();
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
        typename std::vector< Leader_elect_node >::iterator it = std::find_if(nodes.begin(), nodes.end(), [](Leader_elect_node& n ) { return ! n.got_message; });
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
                MLE& msg_in = node.elect_message;
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

/**
* A Node in a graph topology
* 1 - 2 - 3
* |   |
* 4 - 5 - 11 - 10
* |       |    |
* 6 - 7 - 8    9
*/
class Graph_node : public Node{
private:
    std::vector < Graph_edge > edges;

protected:
    std::string get_info();

public:
    Graph_node(int scenario);
    Graph_node() : Graph_node(-1){};
    void boruvka_mst();
};
#endif
