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

    virtual std::string get_info() = 0;

public:
    int leader;
    virtual ~MSG_leader_elect(){};
    virtual int tag() = 0;
    virtual void merge(MLE& msg_other) = 0; 
    void print(){
        std::cout << "leader: " << leader << " " << get_info() << std::endl;
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

    std::string get_info(){
        std::ostringstream oss;
        oss << "leader_rnd_order: " << leader_rnd_order;
        return oss.str();
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
        ar & leader_rnd_order;
    }

    std::string get_info(){
        std::ostringstream oss;
        oss << "leader_rnd_order: " << leader_rnd_order;
        return oss.str();
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
        ar & min_edge;
        ar & min_edge_min_node_rank;
        ar & tree_nodes;
    }

    std::string get_info(){
        std::ostringstream oss;
        oss <<  "leader: " << leader << 
                "min_edge.to: " << min_edge.to <<
                "min_edge.weight: " << min_edge.weight <<
                "min_edge_min_node_rank: " << min_edge_min_node_rank <<
                "tree_nodes: ";
        for(std::set< int >::iterator it = tree_nodes.begin(); it != tree_nodes.end(); ++it){
            oss << "( " << *it << " )";
        }
        return oss.str();
    }
public:
    int leader;
    Graph_edge min_edge;
    int min_edge_min_node_rank;
    std::set < int > tree_nodes;
    int tag(){
        return MSG_GRAPH_LEADER_ELECT_TAG;
    }
    void merge(MSG_graph_leader_elect& msg_other){
        // expand mst
        tree_nodes.insert(msg_other.tree_nodes.begin(),msg_other.tree_nodes.end());
        // merge the other message into the local one?
        if( (
                // if our candidate is already in mst
                tree_nodes.count(min_edge.to) > 0
                || (
                    // or the other has a candidate
                    msg_other.min_edge.to != -1
                    // which is new
                    && tree_nodes.count(msg_other.min_edge.to) < 1
                )
            ) && (
                // and we dont have a candidate
                min_edge.to == -1
                // or candidate has min edge
                || msg_other.min_edge.weight < min_edge.weight
                // or if both the same, the one with min_node_rank wins
                || ( 
                    msg_other.min_edge.weight == min_edge.weight
                    && msg_other.min_edge_min_node_rank < min_edge_min_node_rank
                )
            )
        ){
            leader                      = msg_other.leader;
            min_edge                    = msg_other.min_edge;
            min_edge_min_node_rank      = msg_other.min_edge_min_node_rank;
        }
    }
};

#define MSG_GRAPH_MST_GROW 5
class MSG_graph_mst_grow{
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        // do nothing
    }
public:
    int tag(){
        return MSG_GRAPH_MST_GROW;
    }
};
BOOST_IS_MPI_DATATYPE(MSG_graph_mst_grow);

#endif
