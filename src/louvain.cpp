#include "static/louvain.h"
#include "static/algorithm.h"

Louvain::Louvain() {}

Louvain::Louvain(const std::string& filename)
  : network(filename) {
    original_n = network.n;
    final_reverse_communities.resize(network.n);
    final_reverse_communities.shrink_to_fit();
    for (unsigned i = 0; i < network.n; ++i) {
        final_reverse_communities[i] = i;
    }
    network.print(true);
    //network.save("qwer");
    new_num_comm = -1;
}

Louvain::Louvain(const unsigned n_in, const unsigned m_in,
                 unsigned** from_to, unsigned* out_degrees)
  : original_n(n_in), network(n_in, m_in, from_to, out_degrees), 
    final_reverse_communities(n_in), new_num_comm(-1) {
    //final_communities = new Deque<unsigned>[n_in];
    final_reverse_communities.shrink_to_fit();
    for (unsigned i = 0; i < n_in; ++i) {
        final_reverse_communities[i] = i;
    }
    network.print(true);
    //network.save("qwer");
}

Louvain::Louvain(unsigned const n_in, const unsigned m_in,
                 unsigned* first_in, unsigned* second_in)
  : original_n(n_in), network(n_in, m_in, first_in, second_in), 
    final_reverse_communities(n_in), new_num_comm(-1) {
    //final_communities = new Deque<unsigned>[n_in];
    final_reverse_communities.shrink_to_fit();
    for (unsigned i = 0; i < n_in; ++i) {
        final_reverse_communities[i] = i;
    }
    network.print(true);
    //network.save("qwer");
}

Louvain::~Louvain() {}

void Louvain::phase1() {
    bool repeat = false;
    RNG rng(1);
    unsigned* indices = new unsigned[network.n];
    for (unsigned i = 0; i < network.n; ++i) {
        indices[i] = i;
    }
    
    double old_modularity = network.modularity();
    double new_modularity = network.modularity();
    do {
        rng.shuffle(indices, network.n);
        old_modularity = new_modularity;
        // This groups nodes by community when , which is worse than 
        for (unsigned i = 0; i < network.n; ++i) {
            unsigned community = network.reverse_communities[indices[i]];
            unsigned node = network.remove_from_community(community);
            double max_diff = -1;
            unsigned new_community = -1;
            for (unsigned j = 0; j < network.n; ++j) {
                double diff = network.modularity_diff(node, j);
                if (diff > max_diff) {
                    max_diff = diff;
                    new_community = j;
                }
            }
            network.add_to_community(node, new_community);
        }
        new_modularity = network.modularity();

    } while (new_modularity > old_modularity);
 
    delete[] indices;

    for (unsigned i = 0; i < original_n; ++i) {
        final_reverse_communities[i] = network.reverse_communities[final_reverse_communities[i]];
    }

    std::cout << new_modularity << std::endl;
    network.print(true);
}

void Louvain::phase2() {
    reindex_communities();
    merge_communities();
    network.print(true);
}

void Louvain::merge_communities() {
    // Either modify data directly here (current doing this), or make a
    // member function that modifies, or just delete the old network and
    // build a new one
    unsigned** to_from = new unsigned*[new_num_comm];
    unsigned** from_to = new unsigned*[new_num_comm];
    unsigned** weights_to_from = new unsigned*[new_num_comm];
    unsigned** weights_from_to = new unsigned*[new_num_comm];
    unsigned* in_degrees = new unsigned[new_num_comm];
    unsigned* out_degrees = new unsigned[new_num_comm];
    unsigned* in_weights = new unsigned[new_num_comm];
    unsigned* out_weights = new unsigned[new_num_comm];
    Deque<unsigned>* communities = new Deque<unsigned>[new_num_comm];
    unsigned* reverse_communities = new unsigned[new_num_comm];
    unsigned* community_in_weights = new unsigned[new_num_comm];
    unsigned* community_out_weights = new unsigned[new_num_comm];

    for (unsigned i = 0; i < new_num_comm; ++i) {
        in_degrees[i] = 0;
        out_degrees[i] = 0;
        in_weights[i] = 0;
        out_weights[i] = 0;
    }

    for (unsigned i = 0; i < network.num_communities; ++i) {
        auto& community1 = network.communities[i];
        unsigned new_comm1 = map[i];
        for (unsigned j = 0; j < network.num_communities; ++j) {
            auto& community2 = network.communities[j];
            unsigned new_comm2 = map[j];
            bool has_edge = false;
            for (unsigned k = 0; k < community1.size(); ++k) {
                unsigned node1 = community1[k];
                for (unsigned l = 0; l < community2.size(); ++l) {
                    unsigned node2 = community2[l];
                    int index = network.has_edge(node1, node2);
                    if (index != -1) {
                        has_edge = true;
                        break;
                    }
                }
                if (has_edge) {
                    break;
                }
            }
            if (has_edge) {
                out_degrees[new_comm1]++;
                in_degrees[new_comm2]++;
            }
        }
    }

    for (unsigned i = 0; i < new_num_comm; ++i) {
        to_from[i] = new unsigned[in_degrees[i]];
        from_to[i] = new unsigned[out_degrees[i]];
        weights_to_from[i] = new unsigned[in_degrees[i]];
        weights_from_to[i] = new unsigned[out_degrees[i]];

        in_degrees[i] = 0;
        out_degrees[i] = 0;
    }

    for (unsigned i = 0; i < network.num_communities; ++i) {
        auto& community1 = network.communities[i];
        unsigned new_comm1 = map[i];
        for (unsigned j = 0; j < network.num_communities; ++j) {
            auto& community2 = network.communities[j];
            unsigned new_comm2 = map[j];
            unsigned weight = 0;
            for (unsigned k = 0; k < community1.size(); ++k) {
                unsigned node1 = community1[k];
                for (unsigned l = 0; l < community2.size(); ++l) {
                    unsigned node2 = community2[l];
                    int index = network.has_edge(node1, node2);
                    if (index != -1) {
                        weight += network.weights_from_to[node1][index];
                    }
                }
            }
            if (weight != 0) {
                from_to[new_comm1][out_degrees[new_comm1]] = new_comm2;
                to_from[new_comm2][in_degrees[new_comm2]] = new_comm1;
                weights_from_to[new_comm1][out_degrees[new_comm1]++] = weight;
                weights_to_from[new_comm2][in_degrees[new_comm2]++] = weight;

                out_weights[new_comm1] += weight;
                in_weights[new_comm2] += weight;
            }
        }
    }
    delete[] map;
    network.delete_responsible();
    network.fully_responsible();

    for (unsigned i = 0; i < new_num_comm; ++i) {
        quicksort_pair(to_from[i], weights_to_from[i], 0, in_degrees[i] - 1);
        quicksort_pair(from_to[i], weights_from_to[i], 0, out_degrees[i] - 1);

        community_in_weights[i] = in_weights[i];
        community_out_weights[i] = out_weights[i];
        communities[i].push_back(i);
        reverse_communities[i] = i;
    }


    network.n = new_num_comm;
    network.num_communities = new_num_comm;
    network.to_from = to_from;
    network.from_to = from_to;
    network.weights_to_from = weights_to_from;
    network.weights_from_to = weights_from_to;
    network.in_degrees = in_degrees;
    network.out_degrees = out_degrees;
    network.in_weights = in_weights;
    network.out_weights = out_weights;
    network.communities = communities;
    network.reverse_communities = reverse_communities;
    network.community_in_weights = community_in_weights;
    network.community_out_weights = community_out_weights;
}

void Louvain::reindex_communities() {
    new_num_comm = 0;
    map = new unsigned[network.num_communities];
    for (unsigned i = 0; i < network.num_communities; ++i) {
        map[i] = -1;
    }
    
    for (unsigned i = 0; i < network.num_communities; ++i) {
        unsigned current_community = network.reverse_communities[i];
        if (map[current_community] == -1) {
            map[current_community] = new_num_comm++;
        }
    }

    for (unsigned i = 0; i < original_n; ++i) {
        final_reverse_communities[i] = map[final_reverse_communities[i]];
    }
}

bool Louvain::check_finished() {
    for (unsigned i = 0; i < network.num_communities; ++i) {
        if (network.communities[i].size() != 1)
            return false;
    }
    return true;
}

void Louvain::partition() {
    while (true) {
        phase1();
        bool finished = check_finished();
        if (finished)
            break;
        phase2();
    }
    
    for (unsigned i = 0; i < new_num_comm; ++i) {
        final_communities.push_back(Vector<unsigned>());
    }
    final_communities.shrink_to_fit();

    for (unsigned i = 0; i < original_n; ++i) {
        final_communities[final_reverse_communities[i]].push_back(i);
    }

    for (unsigned i = 0; i < new_num_comm; ++i) {
        final_communities[i].shrink_to_fit();
    }
}

const Vector<Vector<unsigned>>& Louvain::get_communities() const {
    return final_communities;
}

const Vector<unsigned>& Louvain::get_reverse_communities() const {
    return final_reverse_communities;
}