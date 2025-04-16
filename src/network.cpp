#include "static/network.h"
#include "static/algorithm.h"

SparseNetwork::SparseNetwork() {}

SparseNetwork::SparseNetwork(const std::string& filename) {
    read_bitstream(filename);
    construct_with_from_to(true);
}

SparseNetwork::SparseNetwork(unsigned const n_in, unsigned const m_in,
                             unsigned** from_to_in, unsigned* out_degrees_in)
  : n(n_in), m(m_in), num_communities(n_in), from_to(from_to_in), out_degrees(out_degrees_in) {
    construct_with_from_to(false);
}

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

        // Maybe assign 0 length arrays to_from nullptr
        to_from[i] = new unsigned[in_degrees[i]];
        from_to[i] = new unsigned[out_degrees[i]];
        weights_to_from[i] = new unsigned[in_degrees[i]];
        weights_from_to[i] = new unsigned[out_degrees[i]];

        for (unsigned j = 0; j < in_degrees[i]; ++j)
            weights_to_from[i][j] = 1;
        for (unsigned j = 0; j < out_degrees[i]; ++j)
            weights_from_to[i][j] = 1;
        
        in_degrees[i] = 0;
        out_degrees[i] = 0;
    }
    
    for (unsigned i = 0; i < m; ++i) {
        unsigned node1 = first_in[i];
        unsigned node2 = second_in[i];
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
    delete_responsible();
}

int SparseNetwork::has_edge(unsigned node1, unsigned node2) {
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

void SparseNetwork::set_communities(unsigned* reverse_communities_in, bool delete_communities_in) {
    delete[] reverse_communities;
    delete[] communities;
    delete[] community_in_weights;
    delete[] community_out_weights;
    delete_communities = delete_communities_in;
    reverse_communities = reverse_communities_in;
    num_communities = 0;

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
        communities[community].push_back(i);
        community_in_weights[community] += in_weights[i];
        community_out_weights[community] += out_weights[i];
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

void SparseNetwork::fully_responsible() {
    delete_from_to = true;
    delete_communities = true;
}

void SparseNetwork::delete_responsible() {
    // TODO: I haven't tested for edge cases
    for (unsigned i = 0; i < n; ++i) {
        delete[] to_from[i];
        if (delete_from_to)
            delete[] from_to[i];
        delete[] weights_to_from[i];
        delete[] weights_from_to[i];
    }

    delete[] to_from;
    if (delete_from_to)
        delete[] from_to;
    delete[] weights_to_from;
    delete[] weights_from_to;
    delete[] in_degrees;
    if (delete_from_to)
        delete[] out_degrees;
    delete[] community_in_weights;
    delete[] community_out_weights;
    delete[] in_weights;
    delete[] out_weights;

    delete[] communities;
    if (delete_communities)
        delete[] reverse_communities;
}

void SparseNetwork::print(bool adjacency) {
    if (adjacency) {
        std::cout << "adjacency matrix:\n";
        for (unsigned i = 0; i < n; ++i) {
            unsigned next = -1;
            unsigned counter = 0;
            if (out_degrees[i] != 0) {
                next = from_to[i][counter];
            }
            for (unsigned j = 0; j < n; ++j) {
                if (j == next) {
                    std::cout << weights_from_to[i][counter++] << ' ';
                    if (counter < out_degrees[i])
                        next = from_to[i][counter];
                }
                else
                    std::cout << "0 ";
            }
            std::cout << '\n';
        }
    }
    
    std::cout << "n: " << n << '\n';
    std::cout << "m: " << m << '\n';
    std::cout << "num_communities: " << num_communities << '\n';
    
    std::cout << "to_from:\n";
    for (unsigned i = 0; i < n; ++i) {
        std::cout << i << " -> ";
        for (unsigned j = 0; j < in_degrees[i]; ++j) {
            std::cout << to_from[i][j] << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "from_to:\n";
    for (unsigned i = 0; i < n; ++i) {
        std::cout << i << " -> ";
        for (unsigned j = 0; j < out_degrees[i]; ++j) {
            std::cout << from_to[i][j] << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "weights_to_from:\n";
    for (unsigned i = 0; i < n; ++i) {
        std::cout << i << " -> ";
        for (unsigned j = 0; j < in_degrees[i]; ++j) {
            std::cout << weights_to_from[i][j] << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "weights_from_to:\n";
    for (unsigned i = 0; i < n; ++i) {
        std::cout << i << " -> ";
        for (unsigned j = 0; j < out_degrees[i]; ++j) {
            std::cout << weights_from_to[i][j] << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "in_degrees: ";
    for (unsigned i = 0; i < n; ++i) {
        std::cout << in_degrees[i] << ' ';
    }
    std::cout << '\n';

    std::cout << "out_degrees: ";
    for (unsigned i = 0; i < n; ++i) {
        std::cout << out_degrees[i] << ' ';
    }
    std::cout << '\n';

    std::cout << "in_weights: ";
    for (unsigned i = 0; i < n; ++i) {
        std::cout << in_weights[i] << ' ';
    }
    std::cout << '\n';

    std::cout << "out_weights: ";
    for (unsigned i = 0; i < n; ++i) {
        std::cout << out_weights[i] << ' ';
    }
    std::cout << '\n';

    std::cout << "communities:\n";
    for (unsigned i = 0; i < n; ++i) {
        std::cout << i << " -> ";
        for (unsigned j = 0; j < communities[i].size(); ++j) {
            std::cout << communities[i][j] << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "reverse_communities: ";
    for (unsigned i = 0; i < n; ++i) {
        std::cout << reverse_communities[i] << ' ';
    }
    std::cout << '\n';

    std::cout << "community_in_weights: ";
    for (unsigned i = 0; i < n; ++i) {
        std::cout << community_in_weights[i] << ' ';
    }
    std::cout << '\n';

    std::cout << "community_out_weights: ";
    for (unsigned i = 0; i < n; ++i) {
        std::cout << community_out_weights[i] << ' ';
    }
    std::cout << '\n';
}

void SparseNetwork::read_bitstream(const std::string& filename) {
    unsigned** raw_rows = nullptr;
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        // handle error
        throw std::runtime_error("file does not exist, is empty, or is badly formatted");
    }
    
    m = 0;

    Vector<unsigned*> rows;
    Vector<unsigned> degrees;
    while (ifs.peek() != EOF) {
        Vector<unsigned> row;
        unsigned value;
        // Read until sentinel is encountered
        while (ifs.read(reinterpret_cast<char*>(&value), sizeof(value))) {
            if (value == -1) {
                break;
            }
            row.push_back(value);
            
        }
        m += row.size();
        degrees.push_back(row.size());
        unsigned* raw_row = new unsigned[row.size()];
        for (unsigned i = 0; i < row.size(); ++i) {
            raw_row[i] = row[i];
        }
        rows.push_back(raw_row);
    }
    
    n = rows.size();
    num_communities = n;
    from_to = new unsigned*[n];
    out_degrees = new unsigned[n];
    for (unsigned i = 0; i < n; ++i) {
        from_to[i] = rows[i];
        out_degrees[i] = degrees[i];
    }
}

void SparseNetwork::read_txt(const std::string& filename) {

}

void SparseNetwork::dump_from_to(const std::string& filename) {
    dump_2D(from_to, n, out_degrees, filename);
}

void SparseNetwork::dump_to_from(const std::string& filename) {
    dump_2D(to_from, n, in_degrees, filename);
}

void SparseNetwork::construct_with_from_to(bool responsible) {
    delete_from_to = responsible;

    to_from = new unsigned*[n];
    weights_to_from = new unsigned*[n];
    weights_from_to = new unsigned*[n];
    // Store degrees in arrays. If these are sparse, use a map instead.
    in_degrees = new unsigned[n];
    in_weights = new unsigned[n];
    out_weights = new unsigned[n];

    // Assign each node to its own community
    communities = new Deque<unsigned>[n];
    reverse_communities = new unsigned[n];
    community_in_weights = new unsigned[n];
    community_out_weights = new unsigned[n];
    for (unsigned i = 0; i < n; ++i) {
        communities[i].push_back(i);
        reverse_communities[i] = i;
    }

    // TODO: find out if this is necessary. It they default to_from 0, or we
    // can to_from new unsigned[m]{0} at declaration, then it's not.
    for (unsigned i = 0; i < n; ++i) {
        in_degrees[i] = 0;
    }
    
    for (unsigned i = 0; i < n; ++i) {
        for (unsigned j = 0; j < out_degrees[i]; ++j) {
            unsigned node = from_to[i][j];
            in_degrees[node]++;
        }
    }

    for (unsigned i = 0; i < n; ++i) {
        in_weights[i] = in_degrees[i];
        out_weights[i] = out_degrees[i];
        community_in_weights[i] = in_degrees[i];
        community_out_weights[i] = out_degrees[i];
        
        // Maybe assign 0 length arrays to_from nullptr
        to_from[i] = new unsigned[in_degrees[i]];
        weights_to_from[i] = new unsigned[in_degrees[i]];
        weights_from_to[i] = new unsigned[out_degrees[i]];

        for (unsigned j = 0; j < in_degrees[i]; ++j)
            weights_to_from[i][j] = 1;
        for (unsigned j = 0; j < out_degrees[i]; ++j)
            weights_from_to[i][j] = 1;
        
        in_degrees[i] = 0;
    }

    for (unsigned i = 0; i < n; ++i) {
        for (unsigned j = 0; j < out_degrees[i]; ++j) {
            unsigned node = from_to[i][j];
            to_from[node][in_degrees[node]++] = i;
        }
    }

    for (unsigned i = 0; i < n; ++i) {
        // This is multithreadable. No overlapping memory accesses.
        quicksort(to_from[i], 0, in_degrees[i] - 1);
        quicksort(from_to[i], 0, out_degrees[i] - 1);
        //quicksort_pair(to_from[i], weights_to_from[i], 0, in_degrees[i] - 1);
        //quicksort_pair(from_to[i], weights_from_to[i], 0, out_degrees[i] - 1);
        //Technically these should be sorted like twins, but weights are
        //always initialized to 1, so there's no difference. But I feel
        //like this should still be here.
    }
}