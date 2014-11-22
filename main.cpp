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
mpi::environment env;
mpi::communicator world;

void usage(){
    std::cout << 
    "Usage: " << std::endl <<
    "   parallel MODE [SCENARIO]" << std::endl <<
    std::endl <<
    "Arguments:" << std::endl <<
    "   MODE : operation mode of program" << std::endl <<
    "           RLE : Ring leader elect" << std::endl <<
    "           TLE : Tree leader elect" << std::endl <<
    "           MST : Minimal spannig tree" << std::endl <<
    "optional Arguments:" << std::endl <<
    "   SCENARIO : scenario used, only for TLE and MST" << std::endl <<
    "           -1 : Random scenario (default)" << std::endl <<
    "           0  : Empty scenario" << std::endl <<
    "           >0 : Custom scenario" << std::endl;
}

/**
* Main entry point for app
*/
int main(int argc, char *argv[]){

    if(argc < 2){
        usage();
        return EXIT_FAILURE;
    }
    std::string mode = std::string(argv[1]);
    std::cout << "Mode: " << mode << std::endl;
    int scenario = DEFAULT_SCENARIO;
    if(argc > 2){
        scenario = std::stoi(std::string(argv[2]));
    }
    std::cout << "Scenario: " << scenario << std::endl;

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
        Tree_node node(scenario);
        MSG_tree_leader_elect msg;
        node.print();
        msg.leader_rnd_order = node.rnd_order;
        node.leader_elect(msg);
    }else if(mode.compare("MST") == 0){
        Graph_node node(scenario);
        node.print();
        node.boruvka_mst();
    }else{
        usage();
        return EXIT_FAILURE;
    }

    std::cout << "Shutting down..." << std::endl;
    return EXIT_SUCCESS;
}
