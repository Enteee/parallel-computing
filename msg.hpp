#ifndef MESSAGES_H
#define MESSAGES_H

#include <boost/serialization/vector.hpp>

using namespace boost;

#define MSG_RING_LEADER_ELECT_TAG 0
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
        return MSG_RING_LEADER_ELECT_TAG;
    }
};
BOOST_IS_MPI_DATATYPE(MSG_ring_leader_elect);

#define MSG_TREE_CONNECT_TAG 0
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

#endif
