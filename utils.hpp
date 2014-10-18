#ifndef UTILS_HPP
#define UTILS_HPP

#include <csignal>

inline void debug_break(){
    raise(SIGINT);
}

#endif
