#ifndef MAIN_HPP

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#define DEFAULT_SCENARIO -1

using namespace boost;

extern long SEED;

extern mpi::environment env;
extern mpi::communicator world;

#endif
