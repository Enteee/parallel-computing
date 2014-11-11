#ifndef GRAPH_HPP
#define GRAPH_HPP

class Graph_edge {
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & to;
        ar & weight;
    }
public:
    int to;
    int weight;
};

#endif
