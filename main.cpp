#include <iostream>
#include <cstdlib>
#include <ctime>

#include "node.hpp"

/**
* Main entry point for app
*/
int main(int argc, char *argv[]){

    // seed random
    std::srand(std::clock());

    // Build node
    Ring_node ring;
    ring.leader_elect();

    return EXIT_SUCCESS;
}
