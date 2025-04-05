#include "static/louvain.h"
#include "static/algorithm.h"

Louvain::Louvain() {}

Louvain::Louvain(const unsigned n_in, const unsigned m_in,
                 unsigned** from_to, unsigned* out_degrees)
  : network(n_in, m_in, from_to, out_degrees) {
    communities = new unsigned[n_in];
}

Louvain::Louvain(unsigned const n_in, const unsigned m_in,
                 unsigned* first_in, unsigned* second_in)
  : network(n_in, m_in, first_in, second_in) {
    communities = new unsigned[n_in];
}

Louvain::~Louvain() {
    delete[] communities;
}

void Louvain::phase1() {
    bool repeat = false;
    RNG rng(1);
    unsigned* indices = new unsigned[network.n];
    for (unsigned i = 0; i < network.n; ++i) {
        indices[i] = i;
    }
    
    double old_modularity = network.modularity();
    double new_modularity = network.modularity();
    do
    {
        rng.shuffle(indices, network.n);
        old_modularity = new_modularity;
        // This groups nodes by community when , which is worse than 
        for (unsigned i = 0; i < network.n; ++i) {
            unsigned community = network.reverse_communities[indices[i]];
            unsigned node = network.remove_from_community(community);
            double max_diff = -1;
            unsigned new_community = -1;
            for (unsigned j = 0; j < network.num_communities; ++j) {
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
}

void Louvain::phase2() {
    unsigned* map = new unsigned[network.num_communities];
    reindex_communities(map);
    merge_communities();
    delete[] map;
}

void Louvain::merge_communities() {
    // Either modify data directly here, or make a member function
    // that modifies, or just delete the old network and build a new one
    for (unsigned i = 0; i < network.num_communities; ++i) {
        auto& community1 = network.communities[i];
        for (unsigned j = 0; j < network.num_communities; ++j) {
            auto& community2 = network.communities[j];
            unsigned weight = 0;
            for (unsigned k = 0; k < community1.size(); ++k) {
                unsigned node1 = community1[k];
                for (unsigned l = 0; l < community2.size(); ++l) {
                    unsigned node2 = community2[l];
                    
                }
            }
        }
    }
}

void Louvain::reindex_communities(unsigned* map) {
    unsigned num_communities = 0;
    for (unsigned i = 0; i < network.num_communities; ++i) {
        map[i] = -1;
    }
    
    for (unsigned i = 0; i < network.num_communities; ++i) {
        unsigned current_community = network.reverse_communities[i];
        if (map[current_community] == -1) {
            map[current_community] = num_communities++;
        }
    }
}