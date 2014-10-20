#include "main.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include "node.hpp"
#include "utils.hpp"

BOOST_CLASS_EXPORT(MSG_ring_leader_elect);
BOOST_CLASS_EXPORT(MSG_tree_leader_elect);

/**
* Main entry point for app
*/
int main(int argc, char *argv[]){
    // seed random
#ifdef SEED
    time_t seed = SEED;
#else
    time_t seed = std::time(NULL);
#endif
    std::cout << "Seed: " << seed << std::endl;
    std::srand(seed);

    // Build node
    NODE_TYPE node;
    print_random();
    node.print();
    node.leader_elect();

    return EXIT_SUCCESS;
}
