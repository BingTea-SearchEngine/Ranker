#include "static/louvain.h"
#include "static/algorithm.h"

Louvain::Louvain() {}

Louvain::Louvain(unsigned const n_in, const unsigned m_in,
                 unsigned* first_in, unsigned* second_in)
  : network(n_in, m_in, first_in, second_in) {}

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

}

void Louvain::merge_communities() {

}

void Louvain::reindex_communities() {
    unsigned* map = new unsigned[network.num_communities];
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

    /*
    Deque<unsigned>* communities = new Deque<unsigned>[num_communities];
    for 
    
    network.num_communities = num_communities;
    */

    delete[] map;
}