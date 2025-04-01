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
    weights = new unsigned*[n];
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
        communities[i] = 0;
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
        
        in_degrees[node2]++;
        out_degrees[node1]++;
    }
    
    for (unsigned i = 0; i < n; ++i) {
        // Maybe assign 0 length arrays to_from nullptr
        to_from[i] = new unsigned[out_degrees[i]];
        from_to[i] = new unsigned[in_degrees[i]];
        weights[i] = new unsigned[in_degrees[i]];
        
        in_degrees[i] = 0;
        out_degrees[i] = 0;
    }
    
    for (unsigned i = 0; i < m; ++i) {
        unsigned node1 = first_in[i];
        unsigned node2 = second_in[i];
        weights[node1][out_degrees[node1]] = 1;
        to_from[node2][in_degrees[node2]++] = node1;
        from_to[node1][out_degrees[node1]++] = node2;
    }

    for (unsigned i = 0; i < n; ++i) {
        // This is multithreadable. No overlapping memory accesses.
        quicksort(to_from[i], 0, in_degrees[i] - 1);
        quicksort(from_to[i], 0, out_degrees[i] - 1);
    }
    // TODO: flatten the 2D lists "to_from" and "from_to" into 1D and use a
    // separate index list. This reduces each list's overhead from_to a
    // pointer to_from an index. The pointer is 64 bit, the index is probably
    // 32 bit.
}

SparseNetwork::~SparseNetwork() {
    for (unsigned i = 0; i < n; ++i) {
        delete[] to_from[i];
        delete[] from_to[i];
        delete[] weights[i];
    }

    delete[] to_from;
    delete[] from_to;
    delete[] weights;
    delete[] in_degrees;
    delete[] out_degrees;
    delete[] communities;
}

bool SparseNetwork::has_edge(unsigned node1, unsigned node2) {
    // If the size is <20, then it's faster to iterative search
    return binary_search(from_to[node1], node2, out_degrees[node1]) != -1;
}

bool SparseNetwork::same_community(unsigned node1, unsigned node2) {
    return communities[node1] == communities[node2];
}

double SparseNetwork::modularity() {
    // TODO: this is easily multithreadable
    double total = 0;
    for (unsigned node1 = 0; node1 < n; ++node1) {
        for (unsigned j = 0; j < out_degrees[node1]; ++j) {
            unsigned node2 = from_to[node1][j];
            if (same_community(node1, node2)) {
                total += weights[node1][j] - double(out_degrees[node1] * in_degrees[node2]) / m;
            }
        }
    }

    return total / (2 * m);
}