#include <iostream>
#include <cstdlib>
#include <ctime>

#include "node.hpp"

#define NODE_TYPE Tree_node

/**
* Main entry point for app
*/
int main(int argc, char *argv[]){

    // seed random
    std::srand(std::clock());

    // Build node
    NODE_TYPE node;
    node.print();
    node.leader_elect();

    return EXIT_SUCCESS;
}
