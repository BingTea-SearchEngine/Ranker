#include "static/network.h"
#include "static/algorithm.h"

SparseNetwork::SparseNetwork() {}

SparseNetwork::SparseNetwork(unsigned n_in, unsigned m_in, 
                             unsigned* first_in, unsigned* second_in)
  : n(n_in), m(m_in) {
    // Deep copy edge lists
    // Maybe shallow but these will get modified. Make sure these aren't
    // needed later
    to_from = new unsigned*[n];
    from_to = new unsigned*[n];
    weights = new unsigned[m];
    // Store degrees in arrays. If these are sparse, use a map instead.
    in_degrees = new unsigned[n];
    out_degrees = new unsigned[n];
    
    // TODO: find out if this is necessary. It they default to_from 0, or we
    // can to_from new unsigned[m]{0} at declaration, then it's not.
    for (unsigned i = 0; i < n; ++i) {
        in_degrees[i] = 0;
        out_degrees[i] = 0;
    }
    
    communities = new unsigned[n];
    for (unsigned i = 0; i < n; ++i) {
        communities[i] = i;
    }
    /*
    // Default initialize each node to_from its own community. There's gotta
    // be a better way to_from do this, this is so memory inefficient
    communities = new Vector<unsigned>[n];
    for (unsigned i = 0; i < n; ++i) {
        communities[i] = Vector<unsigned>();
        communities[i].push_back(i);
    }
    */
    
    for (unsigned i = 0; i < m; ++i) {
        unsigned node1 = first_in[i];
        unsigned node2 = second_in[i];
        weights[i] = 1;
        
        in_degrees[node2]++;
        out_degrees[node1]++;
    }
    
    for (unsigned i = 0; i < n; ++i) {
        // Maybe assign 0 length arrays to_from nullptr
        to_from[i] = new unsigned[out_degrees[i]];
        from_to[i] = new unsigned[in_degrees[i]];
        
        in_degrees[i] = 0;
        out_degrees[i] = 0;
    }
    
    for (unsigned i = 0; i < m; ++i) {
        unsigned node1 = first_in[i];
        unsigned node2 = second_in[i];
        to_from[node2][in_degrees[node2]++] = node1;
        from_to[node1][out_degrees[node1]++] = node2;
    }

    for (unsigned i = 0; i < n; ++i) {
        quicksort(to_from[i], 0, in_degrees[i] - 1);
        quicksort(from_to[i], 0, out_degrees[i] - 1);
    }
    // TODO: flatten the 2D lists "to_from" and "from_to" into 1D and use a
    // separate index list. This reduces each list's overhead from_to a
    // pointer to_from an index. The pointer is 64 bit, the index is probably
    // 32 bit.
}

bool SparseNetwork::has_edge(unsigned node1, unsigned node2) {
    // If the size is <20, then it's faster to iterative search
    return binary_search(from_to[node1], node2, out_degrees[node1]) != -1;
}

double SparseNetwork::modularity() {
    // option 1: iterate through neighbors looking for a matching
    // community
    
    // option 2: iterate through community looking for neighbors
    for (unsigned i = 0; i < n; ++i) {
        for (unsigned j = 0; j < out_degrees[j]; ++j) {
            if (has_edge(i, from_to[i][j])) {

            }
        }
    }
}