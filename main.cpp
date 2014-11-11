#include "main.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include "node.hpp"
#include "utils.hpp"

long SEED;

/**
* Main entry point for app
*/
int main(int argc, char *argv[]){

#ifndef SET_SEED
    // from : http://stackoverflow.com/questions/8920411/possible-sources-for-random-number-seeds
    long s, pid;

    pid = getpid();
    s = time (&s); /* get CPU seconds since 01/01/1970 */

    SEED = abs(((s*181)*((pid-83)*359))%104729); 
#else
    SEED = SET_SEED;
#endif
    std::cout << "Seed: " << SEED << std::endl;
    std::srand(SEED);

    // Build node
    NODE_TYPE node;
    print_random();
    node.print();
    node.leader_elect();
    //node.matrix_calc();

    return EXIT_SUCCESS;
}
