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
    weights_to_from = new unsigned*[n];
    weights_from_to = new unsigned*[n];
    // Store degrees in arrays. If these are sparse, use a map instead.
    in_degrees = new unsigned[n];
    out_degrees = new unsigned[n];
    in_weights = new unsigned[n];
    out_weights = new unsigned[n];
    
    // TODO: find out if this is necessary. It they default to_from 0, or we
    // can to_from new unsigned[m]{0} at declaration, then it's not.
    for (unsigned i = 0; i < n; ++i) {
        in_degrees[i] = 0;
        out_degrees[i] = 0;
    }
    
    communities = new Vector<unsigned>[n];
    reverse_communities = new unsigned[n];
    community_in_weights = new unsigned[n];
    community_out_weights = new unsigned[n];
    for (unsigned i = 0; i < n; ++i) {
        communities[i] = Vector<unsigned>(1);
        communities[0] = i;
        reverse_communities[i] = i;
        community_in_weights[i] = 1;
        community_out_weights[i] = 1;
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
        in_weights[i] = in_degrees[i];
        out_weights[i] = out_degrees[i];
    }
    
    for (unsigned i = 0; i < n; ++i) {
        // Maybe assign 0 length arrays to_from nullptr
        to_from[i] = new unsigned[out_degrees[i]];
        from_to[i] = new unsigned[in_degrees[i]];
        weights_to_from[i] = new unsigned[out_degrees[i]];
        weights_from_to[i] = new unsigned[in_degrees[i]];
        
        in_degrees[i] = 0;
        out_degrees[i] = 0;
    }
    
    for (unsigned i = 0; i < m; ++i) {
        unsigned node1 = first_in[i];
        unsigned node2 = second_in[i];
        weights_from_to[node1][out_degrees[node1]] = 1;
        weights_to_from[node2][in_degrees[node2]] = 1;
        from_to[node1][out_degrees[node1]++] = node2;
        to_from[node2][in_degrees[node2]++] = node1;
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
        delete[] weights_to_from[i];
        delete[] weights_from_to[i];
    }

    delete[] to_from;
    delete[] from_to;
    delete[] weights_to_from;
    delete[] weights_from_to;
    delete[] in_degrees;
    delete[] out_degrees;
    delete[] in_weights;
    delete[] out_weights;

    if (delete_communities)
        delete[] communities;
}

bool SparseNetwork::has_edge(unsigned node1, unsigned node2) {
    // If the size is <20, then it's faster to iterative search
    return binary_search(from_to[node1], node2, out_degrees[node1]) != -1;
}

bool SparseNetwork::same_community(unsigned node1, unsigned node2) {
    return reverse_communities[node1] == reverse_communities[node2];
}

double SparseNetwork::modularity() {
    // TODO: this is easily multithreadable

    double total = 0;
    for (unsigned node = 0; node < n; ++node)
        total += node_modularity(node);

    return total / m;
}

unsigned* SparseNetwork::get_successors(unsigned node) {
    return from_to[node];
}

unsigned* SparseNetwork::get_predecessors(unsigned node) {
    return to_from[node];
}

unsigned SparseNetwork::degree(unsigned node, bool out) {
    if (out)
        return out_degrees[node];

    return in_degrees[node];
}

unsigned SparseNetwork::node_weight(unsigned node, bool out) {
    if (out)
        return out_weights[node];
    
    return in_weights[node];
}

unsigned SparseNetwork::node_community_weight(unsigned node, bool out) {
    // TODO: test this
    unsigned* degrees;
    unsigned** neighbors;
    unsigned** weights;
    if (out) {
        degrees = out_degrees;
        neighbors = from_to;
        weights = weights_from_to;
    }
    else {
        degrees = in_degrees;
        neighbors = to_from;
        weights = weights_to_from;
    }
    
    unsigned total = 0;
    for (unsigned i = 0; i < degrees[node]; ++i) {
        unsigned other = neighbors[node][i];
        if (same_community(node, other)) {
            total += weights[node][i];
        }
    }

    return total;
}

void SparseNetwork::set_community(unsigned node, unsigned community) {
    unsigned original_community = reverse_communities[node];
    unsigned in_weight = in_weights[node];
    unsigned out_weight = out_weights[node];
    
    community_in_weights[original_community] -= in_weight;
    community_out_weights[original_community] -= out_weight;

    community_in_weights[community] += in_weight;
    community_out_weights[community] += out_weight;

    reverse_communities[node] = community;
    // Remove from old community
    // Add to new community
}

double SparseNetwork::node_modularity(unsigned node) {
    // This may also be worth multithreading

    // THIS IS FUCKED
    // I NEED A WAY TO GET ALL PAIRS WITHIN A COMMUNITY, EVEN IF THERE'S
    // NO EDGE
    double total = 0;
    for (unsigned j = 0; j < out_degrees[node]; ++j) {
        unsigned other = from_to[node][j];
        if (same_community(node, other)) {
            total += weights_from_to[node][j] - double(out_degrees[node] * in_degrees[other]) / m;
        }
    }
    return total;
}

void SparseNetwork::set_communities(unsigned* communities_in) {
    delete[] communities;
    delete_communities = false;

    // communities = communities_in;
}

double SparseNetwork::modularity_diff(unsigned node, unsigned community) {
    // unsigned original_community = communities[node];
    communities[node] = community;
    unsigned weight = (node_community_weight(node, true) 
                       + node_community_weight(node, false));
    unsigned expected = (out_weights[node] * community_in_weights[community]
                         + in_weights[node] * community_out_weights[community]) / m;

    return (weight - expected) / m;
}