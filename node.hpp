#ifndef NODE_HPP
#define NODE_HPP

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include "msg.hpp"

#define NODE_MAX_RANK 50

using namespace boost;

/**
* Basic node
*/
class Node {
private:

protected:
    mpi::environment env;
    mpi::communicator world;
    int node_rank;
    virtual std::string get_info() = 0;

public:
    Node();
    void print();
    /*
    * Select the one with the lowest node_rank && world.rank() as leader
    */
    virtual void leader_elect() = 0;
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
    void leader_elect();

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
    class Leader_elect_node {
    public:
        int node_rank;
        MSG_tree_leader_elect elect_message;
        bool got_message;
        mpi::request req;
    };

    std::vector < int > connected;

protected:
    std::string get_info();

public:
    Tree_node();
    void leader_elect();
    void matrix_calc();

};

#endif
