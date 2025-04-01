#ifndef NETWORK_H
#define NETWORK_H

#include "static/vector.h"
#include <iostream>

class SparseNetwork {
    private:
    unsigned int n;
    unsigned int m;
    
    unsigned int** to_from;
    unsigned int** from_to;
    unsigned int* weights;
    unsigned int* in_degrees;
    unsigned int* out_degrees;
    Vector<unsigned int>* communities; // These are just vectors instead of sets, 
    // because they should stay small.
    // They communities merge to a
    // single node at every iteration.
    double community_modularity(unsigned int community);
    public:
    SparseNetwork();
    SparseNetwork(unsigned int n_in, unsigned int m_in, 
                  unsigned int* first_in, unsigned int* second_in);
    bool has_edge(unsigned int node1, unsigned int node2);
    double modularity();
};
    
#endif