#ifndef NETWORK_H
#define NETWORK_H

class SparseNetwork {
    private:
    unsigned int n;
    unsigned int m;
    unsigned int* first;
    unsigned int* second;
    unsigned int* weights;
    unsigned int* in_degrees;
    unsigned int* out_degrees;
    unsigned int** communities; // These are just arrays, 
    // because they should stay small.
    // They communities merge to a
    // single node at every iteration.
    unsigned int* community_sizes;
    double community_modularity(unsigned int community);
    public:
    SparseNetwork();
    SparseNetwork(unsigned int n_in, unsigned int m_in, 
                  unsigned int* first_in, unsigned int* second_in);
    unsigned int find_edge(unsigned int node1, unsigned int node2);
    double modularity();
};
    
#endif