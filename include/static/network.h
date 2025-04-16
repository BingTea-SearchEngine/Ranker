#ifndef NETWORK_H
#define NETWORK_H

#include "static/vector.h"
#include "static/deque.h"
#include <iostream>
#include <limits>
#include <fstream>
#include <string> // idk if i should make my own string. Sounds annoying for other devs to initialize
#include <stdexcept>

class SparseNetwork {
    private:
        bool delete_from_to = true;
        bool delete_communities = true;
        
        // Maybe move communities and its related functions to
        // Louvain. This might require more getter functions or making
        // the privates public. I could friend, but that kind of feels
        // bad, since I want this class to be universal. 
        
        // Maybe add a private function that returns a dynamic array of
        // indices that refer to the neighbors within the same
        // community. This avoid code duplication 
        void read_bitstream(const std::string& filename); // This modifies n, m, num_communities, from_to, and out_degrees
        void read_txt(const std::string& filename); // This modifies n, m, num_communities, from_to, and out_degrees
        void construct_with_from_to(bool responsible);
    public:
        unsigned n;
        unsigned m;
        unsigned num_communities; 
        // This is separate from n since it may end up different when 
        // set_communities is called
        
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
        unsigned* community_in_weights;
        unsigned* community_out_weights;

        SparseNetwork();
        SparseNetwork(const std::string& filename);
        // TODO: maybe add another constructor that passes in a
        // community assignment
        SparseNetwork(const unsigned n, const unsigned m,
                      unsigned** from_to, unsigned* out_degrees);
        SparseNetwork(unsigned const n, unsigned const m,
                      unsigned* first, unsigned* second);
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
        void set_communities(unsigned* reverse_communities, bool delete_communities);
        double modularity_diff(unsigned node, unsigned community);
        void fully_responsible();
        void delete_responsible();
        void print(bool adjacency);
        void dump_from_to(const std::string& filename);
        void dump_to_from(const std::string& filename);
};
    
#endif