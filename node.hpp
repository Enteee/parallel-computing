#ifndef NODE_HPP
#define NODE_HPP

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

using namespace boost;

class Ring_node {
private:
    mpi::environment env;
    mpi::communicator world;
    int node_rank;
    int next;
    int prev;

public:
    Ring_node();
    void print();
    /*
    * Select the one with the lowest node_rank && world.rank() as leader
    */
    void leader_elect();
};

#endif
