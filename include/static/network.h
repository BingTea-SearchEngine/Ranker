#ifndef NETWORK_H
#define NETWORK_H

#include "static/vector.h"
#include <iostream>

class SparseNetwork {
    private:
        unsigned n;
        unsigned m;
        
        unsigned** to_from;
        unsigned** from_to;
        unsigned* weights;
        unsigned* in_degrees;
        unsigned* out_degrees;
        unsigned* communities;
        // Vector<unsigned>* communities; // These are just vectors instead of sets, 
        // because they should stay small.
        // They communities merge to a
        // single node at every iteration.
    public:
        SparseNetwork();
        SparseNetwork(unsigned n_in, unsigned m_in, 
                      unsigned* first_in, unsigned* second_in);
        bool has_edge(unsigned node1, unsigned node2);
        double modularity();
};
    
#endif