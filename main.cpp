#include <iostream>
#include <stdlib.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

using namespace boost;

typedef boost::adjacency_list<listS, vecS, undirectedS> undirected_adjacency;
class Ring_topology {
    undirected_adjacency g;
    int world_rank;

public:
    Ring_topology(int world_size,int world_rank);
    void print();
    void leader_elect();

};

Ring_topology::Ring_topology(int world_size,int world_rank){
    this->world_rank = world_rank;
    // Build world
    for(int i=0;i<world_size;i++){
        add_vertex(g);
    }
    undirected_adjacency::vertex_iterator vertex_iter, vertex_end;
    for(tie(vertex_iter, vertex_end) = vertices(g);vertex_iter != vertex_end; vertex_iter++){
        add_edge(*(vertex_iter),*(vertex_iter+1),g);
    }
    // connect last vertex to first
    add_edge(*(vertex_iter),0,g);
}

void Ring_topology::print(){
    std::cout << ":::Topology:::\n";
    undirected_adjacency::vertex_iterator vertex_iter, vertex_end;
    for(tie(vertex_iter, vertex_end) = vertices(g);vertex_iter != vertex_end; vertex_iter++){
        std::cout << *vertex_iter << " ->";
        // print adjacent vertices
        undirected_adjacency::adjacency_iterator adjacent_vertex_iter, adjacent_vertex_end;
        tie(adjacent_vertex_iter, adjacent_vertex_end) = adjacent_vertices(*vertex_iter,g);
        for(;adjacent_vertex_iter != adjacent_vertex_end;adjacent_vertex_iter++){
            std::cout << " ( " << *adjacent_vertex_iter << " ) ";
        }
        std::cout << std::endl;
    }
}

void Ring_topology::leader_elect(){
    
}

/**
* Main entry point for app
*/
int main(int argc, char *argv[]){
    mpi::environment env;
    mpi::communicator world;

    // Build topology
    Ring_topology ring(world.size(),world.rank());
    if(world.rank() == 0){
        ring.print();
    }

    return EXIT_SUCCESS;
}
