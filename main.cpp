#include <iostream>
#include <cstdlib>
#include <ctime>

#include <boost/graph/adjacency_list.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

using namespace boost;

class MSG_ring_leader_elect{
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & sender;
        ar & leader;
        ar & leader_node_rank;
    }
public:
    int sender;
    int leader;
    int leader_node_rank;
    int tag(){
        return 0;
    }
};
BOOST_IS_MPI_DATATYPE(MSG_ring_leader_elect);

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

Ring_node::Ring_node(){
    // set rank
    node_rank = std::rand();
    next = (world.rank() + 1) % world.size();
    prev = (world.rank() == 0) ? world.size() - 1 : (world.rank() - 1) % world.size();
    print();
}

void Ring_node::print(){
    std::cout << "[" << world.rank() << "] rank: " << node_rank << " next: " << next << " prev: "<< prev << std::endl;
}

void Ring_node::leader_elect(){
    // perpare message
    MSG_ring_leader_elect msg;
    msg.sender              = world.rank();
    msg.leader              = world.rank();
    msg.leader_node_rank    = node_rank;
    do {
        // send message to ring
        world.send(next,msg.tag(),msg);
        world.recv(prev, msg.tag(), msg);
        if(msg.leader_node_rank < node_rank
            || ( msg.leader_node_rank == node_rank && msg.leader < world.rank())){
            msg.leader              = world.rank();
            msg.leader_node_rank    = node_rank;
        }
    }while(msg.sender != world.rank()); // message went around
    std::cout << "Leader: " << msg.leader << std::endl;
}

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
