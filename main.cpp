#include "main.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#include <string>

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include "node.hpp"
#include "utils.hpp"

long SEED;

void usage(){
    std::cout << 
    "Usage: " << std::endl <<
    "   parallel MODE" << std::endl <<
    std::endl <<
    "Arguments:" << std::endl <<
    "   MODE : operation mode of program" << std::endl <<
    "           RLE : Ring leader elect" << std::endl <<
    "           TLE : Tree leader elect" << std::endl <<
    "           MST : Minimal spannig tree" << std::endl;
}

/**
* Main entry point for app
*/
int main(int argc, char *argv[]){

    if(argc != 2){
        usage();
        return EXIT_FAILURE;
    }
    std::string mode = std::string(argv[1]);

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

    if(mode.compare("RLE") == 0){
        Ring_node node;
        MSG_ring_leader_elect msg;
        node.print();
        node.leader_elect(msg);
    }else if(mode.compare("TLE") == 0){
        Tree_node node;
        MSG_tree_leader_elect msg;
        node.print();
        node.leader_elect(msg);
    }else if(mode.compare("MST") == 0){
        Graph_node node;

    }else{
        usage();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
