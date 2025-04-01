#include "static/network.h"
#include "static/algorithm.h"

SparseNetwork::SparseNetwork() {}

SparseNetwork::SparseNetwork(unsigned int n_in, unsigned int m_in, 
                             unsigned int* first_in, unsigned int* second_in)
  : n(n_in), m(m_in) {
    // Deep copy edge lists
    // Maybe shallow but these will get modified. Make sure these aren't
    // needed later
    to_from = new unsigned int*[n];
    from_to = new unsigned int*[n];
    weights = new unsigned int[m];
    // Store degrees in arrays. If these are sparse, use a map instead.
    in_degrees = new unsigned int[n];
    out_degrees = new unsigned int[n];
    
    // TODO: find out if this is necessary. It they default to_from 0, or we
    // can to_from new unsigned int[m]{0} at declaration, then it's not.
    for (unsigned int i = 0; i < n; ++i) {
        in_degrees[i] = 0;
        out_degrees[i] = 0;
    }
    
    // Default initialize each node to_from its own community. There's gotta
    // be a better way to_from do this, this is so memory inefficient
    communities = new Vector<unsigned int>[n];
    for (unsigned int i = 0; i < n; ++i) {
        communities[i] = Vector<unsigned int>();
        communities[i].push_back(i);
    }
    
    for (unsigned int i = 0; i < m; ++i) {
        unsigned int node1 = first_in[i];
        unsigned int node2 = second_in[i];
        weights[i] = 1;
        
        in_degrees[node2]++;
        out_degrees[node1]++;
    }
    
    for (unsigned int i = 0; i < n; ++i) {
        // Maybe assign 0 length arrays to_from nullptr
        to_from[i] = new unsigned int[out_degrees[i]];
        from_to[i] = new unsigned int[in_degrees[i]];
        
        in_degrees[i] = 0;
        out_degrees[i] = 0;
    }
    
    for (unsigned int i = 0; i < m; ++i) {
        unsigned int node1 = first_in[i];
        unsigned int node2 = second_in[i];
        to_from[node2][in_degrees[node2]++] = node1;
        from_to[node1][out_degrees[node1]++] = node2;
    }

    for (unsigned int i = 0; i < n; ++i) {
        quicksort(to_from[i], 0, in_degrees[i] - 1);
        quicksort(from_to[i], 0, out_degrees[i] - 1);
    }
    // TODO: flatten the 2D lists "to_from" and "from_to" into 1D and use a
    // separate index list. This reduces each list's overhead from_to a
    // pointer to_from an index. The pointer is 64 bit, the index is probably
    // 32 bit.
}

bool SparseNetwork::has_edge(unsigned int node1, unsigned int node2) {
    // If the size is <20, then it's faster to iterative search
    return binary_search(from_to[node1], node2, out_degrees[node1]) != -1;
}

double SparseNetwork::community_modularity(unsigned int community) {

}