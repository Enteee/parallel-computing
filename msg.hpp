#ifndef MESSAGES_H
#define MESSAGES_H
#include <iostream>
#include <cstdlib>
#include <list>

#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

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
        ar & leader_node_rank;
    }
public:
    int sender;
    int leader_node_rank;
    int tag(){
        return MSG_RING_LEADER_ELECT_TAG;
    }
    void merge(MSG_ring_leader_elect& msg_other){
        // merger the other message into the local one?
        if(msg_other.leader_node_rank < leader_node_rank
            || ( msg_other.leader_node_rank == leader_node_rank && msg_other.leader < leader)){
            // merge
            leader              = msg_other.leader;
            leader_node_rank    = msg_other.leader_node_rank;
        }
    }
};
BOOST_IS_MPI_DATATYPE(MSG_ring_leader_elect);
//BOOST_CLASS_EXPORT(MSG_ring_leader_elect);

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
        ar & leader_node_rank;
    }
public:
    int leader_node_rank;
    int tag(){
        return MSG_TREE_LEADER_ELECT_TAG;
    }
    void merge(MSG_tree_leader_elect& msg_other){
        // merger the other message into the local one?
        if(msg_other.leader_node_rank < leader_node_rank
            || ( msg_other.leader_node_rank == leader_node_rank && msg_other.leader < leader)){
            // merge
            leader              = msg_other.leader;
            leader_node_rank    = msg_other.leader_node_rank;
        }
    }
};
BOOST_IS_MPI_DATATYPE(MSG_tree_leader_elect);
//BOOST_CLASS_EXPORT(MSG_tree_leader_elect);

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
#endif
