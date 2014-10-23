#ifndef MESSAGES_H
#define MESSAGES_H
#include <iostream>
#include <cstdlib>

#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

using namespace boost;

/**
* Parent class for all leader elect messages
*/
class MSG_leader_elect{
private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & leader;
        ar & leader_node_rank;
    }

public:
    int leader;
    int leader_node_rank;
    virtual ~MSG_leader_elect(){};
    virtual int tag() = 0;
    void merge(MSG_leader_elect &msg_other){
        // merger the other message into the local one?
        if(msg_other.leader_node_rank < leader_node_rank
            || ( msg_other.leader_node_rank == leader_node_rank && msg_other.leader < leader)){
            leader              = msg_other.leader;
            leader_node_rank    = msg_other.leader_node_rank;
        }
    }
    void print(){
        std::cout << "leader: " << leader << " node_rank: " << leader_node_rank << std::endl;
    }
};
BOOST_IS_MPI_DATATYPE(MSG_leader_elect);

#define MSG_RING_LEADER_ELECT_TAG 0
class MSG_ring_leader_elect : public MSG_leader_elect {
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & boost::serialization::base_object<MSG_leader_elect>(*this);
        ar & sender;
    }
public:
    int sender;
    int tag(){
        return MSG_RING_LEADER_ELECT_TAG;
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
    std::vector <int> connected;
    int tag(){
        return MSG_TREE_CONNECT_TAG;
    }
};

#define MSG_TREE_LEADER_ELECT_TAG 2
class MSG_tree_leader_elect : public MSG_leader_elect {
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & boost::serialization::base_object<MSG_leader_elect>(*this);
    }

public:
    int tag(){
        return MSG_TREE_LEADER_ELECT_TAG;
    }
};
BOOST_IS_MPI_DATATYPE(MSG_tree_leader_elect);
//BOOST_CLASS_EXPORT(MSG_tree_leader_elect);

#endif
