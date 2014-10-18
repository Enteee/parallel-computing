#include "node.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>

#include "msg.hpp"

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
