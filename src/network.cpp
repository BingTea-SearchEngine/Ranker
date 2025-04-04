#include "static/network.h"
#include "static/algorithm.h"

SparseNetwork::SparseNetwork() {}

SparseNetwork::SparseNetwork(unsigned const n_in, unsigned const m_in, 
                             unsigned* first_in, unsigned* second_in)
  : n(n_in), m(m_in), num_communities(n_in) {
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
    
    // Assign each node to its own community
    communities = new Deque<unsigned>[n];
    reverse_communities = new unsigned[n];
    community_in_weights = new unsigned[n];
    community_out_weights = new unsigned[n];
    for (unsigned i = 0; i < n; ++i) {
        communities[i].push_back(i);
        reverse_communities[i] = i;
    }
    
    for (unsigned i = 0; i < m; ++i) {
        unsigned node1 = first_in[i];
        unsigned node2 = second_in[i];
        
        in_degrees[node2]++;
        out_degrees[node1]++;
    }

    for (unsigned i = 0; i < n; ++i) {
        in_weights[i] = in_degrees[i];
        out_weights[i] = out_degrees[i];
        community_in_weights[i] = in_degrees[i];
        community_out_weights[i] = out_degrees[i];
    }
    
    for (unsigned i = 0; i < n; ++i) {
        // Maybe assign 0 length arrays to_from nullptr
        to_from[i] = new unsigned[in_degrees[i]];
        from_to[i] = new unsigned[out_degrees[i]];
        weights_to_from[i] = new unsigned[in_degrees[i]];
        weights_from_to[i] = new unsigned[out_degrees[i]];
        
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
    delete[] community_in_weights;
    delete[] community_out_weights;
    delete[] in_weights;
    delete[] out_weights;

    delete[] communities;
    if (delete_communities)
        delete[] reverse_communities;
}

int SparseNetwork::has_edge(unsigned node1, unsigned node2) {
    // If the size is <20, then it's faster to iterative search
    return binary_search(from_to[node1], node2, out_degrees[node1]);
}

bool SparseNetwork::same_community(unsigned node1, unsigned node2) {
    return reverse_communities[node1] == reverse_communities[node2];
}

double SparseNetwork::modularity() {
    // TODO: this is easily multithreadable

    double total = 0;
    for (unsigned community = 0; community < num_communities; ++community)
        total += community_modularity(community);

    return total;
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

void SparseNetwork::add_to_community(unsigned node, unsigned community) {
    // I'm lazy and can't be bothered to think of an actually good
    // solution. Assume that the given node doesn't already belong in a
    // community (aka it's been placed in a temp community) so we don't
    // have to clean it up once it's taken out.
    communities[community].push_back(node);

    community_in_weights[community] += in_weights[node];
    community_out_weights[community] += out_weights[node];

    reverse_communities[node] = community;
    // Remove from old community
    // Add to new community
}

unsigned SparseNetwork::remove_from_community(unsigned community) {
    unsigned node = communities[community].front();
    communities[community].pop_front();

    reverse_communities[node] = -1;

    community_in_weights[community] -= in_weights[node];
    community_out_weights[community] -= out_weights[node];

    return node;
}

double SparseNetwork::community_modularity(unsigned community) {
    auto& target_comm = communities[community];
    double total = 0;
    for (unsigned i = 0; i < target_comm.size(); ++i) {
        for (unsigned j = 0; j < target_comm.size(); ++j) {
            unsigned node1 = target_comm[i];
            unsigned node2 = target_comm[j];

            int index = has_edge(node1, node2);
            if (index != -1) {
                total += weights_from_to[node1][index];
            }

            total -= double(out_weights[node1] * in_weights[node2]) / m;
        }
    }
    return total / m;
}

void SparseNetwork::set_communities(unsigned* reverse_communities_in) {
    delete[] reverse_communities;
    delete[] communities;
    delete[] community_in_weights;
    delete[] community_out_weights;
    delete_communities = false;
    reverse_communities = reverse_communities_in;

    for (unsigned i = 0; i < n; ++i) {
        unsigned community = reverse_communities_in[i];
        if (community >= num_communities)
            num_communities = community + 1;
    }

    communities = new Deque<unsigned>[num_communities];
    community_in_weights = new unsigned[num_communities];
    community_out_weights = new unsigned[num_communities];
    for (unsigned i = 0; i < num_communities; ++i) {
        community_in_weights[i] = 0;
        community_out_weights[i] = 0;
    }

    for (unsigned i = 0; i < n; ++i) {
        unsigned community = reverse_communities_in[i];
        if (community >= num_communities)
            num_communities = community + 1;
        
        communities[community].push_back(i);
        community_in_weights[community] += in_weights[i];
        community_out_weights[community] += out_weights[i];
    }

    for (unsigned i = 0; i < num_communities; ++i) {
        communities[i].print_inorder();
    }
}

double SparseNetwork::modularity_diff(unsigned node, unsigned community) {
    reverse_communities[node] = community;
    unsigned weight = (node_community_weight(node, true) 
                       + node_community_weight(node, false));
    double expected = double(out_weights[node] * community_in_weights[community]
                             + in_weights[node] * community_out_weights[community]) / m;
    
    return (weight - expected) / m;
}