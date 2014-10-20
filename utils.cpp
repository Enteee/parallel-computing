#include "utils.hpp"

#include <iostream>
#include <csignal>
#include <cstdlib>

inline void debug_break(){
    raise(SIGINT);
}

// random generator function:
int myrandom (int i) { 
    return std::rand() % i;
}

void print_random(){
    for(int i=0;i<10;i++){
        std::cout << rand() << "";
    }
    std::cout << std::endl;
}

