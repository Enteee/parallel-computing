#ifndef MESSAGES_H
#define MESSAGES_H
#include <iostream>
#include <cstdlib>
#include <list>

#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/set.hpp>

#include "graph.hpp"

using namespace boost;

/**
* Parent class for all leader elect messages
*/
template <class MLE>
class MSG_leader_elect{
private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & leader;
    }

public:
    int leader;
    virtual ~MSG_leader_elect(){};
    virtual int tag() = 0;
    virtual void merge(MLE& msg_other) = 0; 
    void print(){
        std::cout << "leader: " << leader << std::endl;
    }
};
//BOOST_IS_MPI_DATATYPE(MSG_leader_elect);

#define MSG_RING_LEADER_ELECT_TAG 0
class MSG_ring_leader_elect : public MSG_leader_elect<MSG_ring_leader_elect> {
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & boost::serialization::base_object<MSG_leader_elect>(*this);
        ar & sender;
        ar & leader_rnd_order;
    }
public:
    int sender;
    int leader_rnd_order;
    int tag(){
        return MSG_RING_LEADER_ELECT_TAG;
    }
    void merge(MSG_ring_leader_elect& msg_other){
        // merger the other message into the local one?
        if(msg_other.leader_rnd_order < leader_rnd_order
            || ( msg_other.leader_rnd_order == leader_rnd_order && msg_other.leader < leader)){
            // merge
            leader              = msg_other.leader;
            leader_rnd_order    = msg_other.leader_rnd_order;
        }
    }
};
BOOST_IS_MPI_DATATYPE(MSG_ring_leader_elect);

#define MSG_TREE_CONNECT_TAG 1
class MSG_tree_connect{
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & connected;
    }
public:
    std::vector < int > connected;
    int tag(){
        return MSG_TREE_CONNECT_TAG;
    }
};

#define MSG_TREE_LEADER_ELECT_TAG 2
class MSG_tree_leader_elect : public MSG_leader_elect<MSG_tree_leader_elect> {
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & boost::serialization::base_object<MSG_leader_elect>(*this);
    }
public:
    int leader_rnd_order;
    int tag(){
        return MSG_TREE_LEADER_ELECT_TAG;
    }
    void merge(MSG_tree_leader_elect& msg_other){
        // merger the other message into the local one?
        if(msg_other.leader_rnd_order < leader_rnd_order
            || ( msg_other.leader_rnd_order == leader_rnd_order && msg_other.leader < leader)){
            // merge
            leader              = msg_other.leader;
            leader_rnd_order    = msg_other.leader_rnd_order;
        }
    }
};
BOOST_IS_MPI_DATATYPE(MSG_tree_leader_elect);

#define MSG_GRAPH_CONNECT_TAG 3
class MSG_graph_connect{
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & edges;
    }
public:
    std::vector < Graph_edge > edges;
    int tag(){
        return MSG_GRAPH_CONNECT_TAG;
    }
};

#define MSG_GRAPH_LEADER_ELECT_TAG 4
class MSG_graph_leader_elect : public MSG_leader_elect<MSG_graph_leader_elect> {
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & boost::serialization::base_object<MSG_leader_elect>(*this);
        ar & leader;
        ar & min_edge_weight;
        ar & min_edge_min_node_rank;
        ar & graph_nodes;
    }
public:
    int leader;
    int min_edge_weight;
    int min_edge_min_node_rank;
    std::set < int > graph_nodes;
    int tag(){
        return MSG_GRAPH_LEADER_ELECT_TAG;
    }
    void merge(MSG_graph_leader_elect& msg_other){
        // merger the other message into the local one?
        if(msg_other.min_edge_weight < min_edge_weight
            || ( msg_other.min_edge_weight == min_edge_weight && msg_other.min_edge_min_node_rank < min_edge_min_node_rank)){
            // merge
            leader                  = msg_other.leader;
            min_edge_weight         = msg_other.min_edge_weight;
            min_edge_min_node_rank  = msg_other.min_edge_min_node_rank;
        }
        // add all nodes from the node array
        graph_nodes.insert(msg_other.graph_nodes.begin(),msg_other.graph_nodes.end());
    }
};

#endif
