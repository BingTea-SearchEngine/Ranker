#ifndef NETWORK_H
#define NETWORK_H

#include "static/vector.h"
#include "static/deque.h"
#include <iostream>
#include <limits>

class SparseNetwork {
    private:
        bool delete_communities = true;

        unsigned n;
        unsigned m;
        unsigned num_communities;

        unsigned** to_from;
        unsigned** from_to;
        unsigned** weights_to_from;
        unsigned** weights_from_to;
        unsigned* in_degrees;
        unsigned* out_degrees;
        unsigned* in_weights;
        unsigned* out_weights;
        Deque<unsigned>* communities;
        unsigned* reverse_communities;
        //unsigned* communities;
        unsigned* community_in_weights;
        unsigned* community_out_weights;
        // Maybe move communities and its related functions to
        // Louvain. This might require more getter functions or making
        // the privates public. I could friend, but that kind of feels
        // bad, since I want this class to be universal. 

        // Maybe add a private function that returns a dynamic array of
        // indices that refer to the neighbors within the same
        // community. This avoid code duplication 
    public:
        SparseNetwork();
        SparseNetwork(unsigned n_in, unsigned m_in,
                      unsigned* first_in, unsigned* second_in);
        ~SparseNetwork();
        int has_edge(unsigned node1, unsigned node2);
        double modularity();
        bool same_community(unsigned node1, unsigned node2);
        unsigned* get_successors(unsigned node);
        unsigned* get_predecessors(unsigned node);
        unsigned degree(unsigned node, bool out);
        unsigned node_weight(unsigned node, bool out);
        unsigned node_community_weight(unsigned node, bool out);
        void add_to_community(unsigned node, unsigned community);
        unsigned remove_from_community(unsigned community);
        double community_modularity(unsigned community);
        void set_communities(unsigned* reverse_communities_in);
        double modularity_diff(unsigned node, unsigned community);
};
    
#endif