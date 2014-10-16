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
        ar & leader;
    }

    int leader;
public:
    int tag(){
        return 0;
    }
};
BOOST_IS_MPI_DATATYPE(MSG_ring_leader_elect);

typedef property<vertex_index_t, int, property<vertex_name_t, std::string> > VertexProperty;
typedef boost::adjacency_list<listS, vecS, undirectedS> undirected_adjacency;

class Ring_topology {
private:
    undirected_adjacency g;
    mpi::environment env;
    mpi::communicator world;
    int topology_rank;

public:
    Ring_topology();
    void print();
    void send_to_connected(MSG_ring_leader_elect msg);
    /*
    * Select the one with the lowest topology_rank && world.rank() as leader
    */
    void leader_elect();
};

Ring_topology::Ring_topology(){
    // set rank
    topology_rank = std::rand();
    // Build world
    for(int i=0;i<world.size();i++){
        add_vertex(g);
    }
    undirected_adjacency::vertex_iterator vertex_iter, vertex_end;
    for(tie(vertex_iter, vertex_end) = vertices(g);vertex_iter != vertex_end; vertex_iter++){
        add_edge(*(vertex_iter),*(vertex_iter+1),g);
    }
    // connect last vertex to first
    add_edge(*(vertex_iter),0,g);
    print();
}

void Ring_topology::print(){
    std::cout << "W: " << world.rank() << " T: " << topology_rank << std::endl;
    // only one should print the topology
    if(world.rank() == 0){
        undirected_adjacency::vertex_iterator vertex_iter, vertex_end;
        for(tie(vertex_iter, vertex_end) = vertices(g);vertex_iter != vertex_end; vertex_iter++){
            std::cout << *vertex_iter << " ->";
            // print adjacent vertices
            undirected_adjacency::adjacency_iterator adjacent_vertex_iter, adjacent_vertex_end;
            for(tie(adjacent_vertex_iter, adjacent_vertex_end) = adjacent_vertices(*vertex_iter,g);adjacent_vertex_iter != adjacent_vertex_end;adjacent_vertex_iter++){
                std::cout << " ( " << *adjacent_vertex_iter << " ) ";
            }
            std::cout << std::endl;
        }
    }
}

void Ring_topology::leader_elect(){
    MSG_ring_leader_elect msg;
    send_to_connected(MSG_ring_leader_elect());
}

void Ring_topology::send_to_connected(MSG_ring_leader_elect msg){
    undirected_adjacency::adjacency_iterator adjacent_vertex_iter, adjacent_vertex_end;
    for(tie(adjacent_vertex_iter, adjacent_vertex_end) = adjacent_vertices(world.rank(),g);adjacent_vertex_iter != adjacent_vertex_end;adjacent_vertex_iter++){
        world.send(*adjacent_vertex_iter,msg.tag(),msg);
    }
}

/**
* Main entry point for app
*/
int main(int argc, char *argv[]){

    // seed random
    std::srand(std::clock());

    // Build topology
    Ring_topology ring;
    ring.leader_elect();

    return EXIT_SUCCESS;
}
