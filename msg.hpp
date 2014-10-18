#ifndef MESSAGES_H
#define MESSAGES_H

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

#endif
