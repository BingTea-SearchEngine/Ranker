#include "static/louvain.h"

unsigned SparseNetwork::n;
unsigned SparseNetwork::m;
unsigned SparseNetwork::num_communities; 
unsigned** SparseNetwork::to_from;
unsigned** SparseNetwork::from_to;
unsigned** SparseNetwork::weights_to_from;
unsigned** SparseNetwork::weights_from_to;
unsigned* SparseNetwork::in_degrees;
unsigned* SparseNetwork::out_degrees;
unsigned* SparseNetwork::in_weights;
unsigned* SparseNetwork::out_weights;
Deque<unsigned>* SparseNetwork::communities;
unsigned* SparseNetwork::reverse_communities;
unsigned* SparseNetwork::community_in_weights;
unsigned* SparseNetwork::community_out_weights;

Louvain::Louvain() {}

Louvain::Louvain(const std::string& filename)
  : network(filename) {
    original_n = network.n;
    final_reverse_communities.resize(network.n);
    final_reverse_communities.shrink_to_fit();
    for (unsigned i = 0; i < network.n; ++i) {
        final_reverse_communities[i] = i;
    }
    new_num_comm = -1;
}

Louvain::Louvain(const std::string& from_to_filename,
                 const std::string& hash_filename)
  : network(from_to_filename, hash_filename) {
    original_n = network.n;
    final_reverse_communities.resize(network.n);
    final_reverse_communities.shrink_to_fit();
    for (unsigned i = 0; i < network.n; ++i) {
        final_reverse_communities[i] = i;
    }
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
}

Louvain::~Louvain() {}

void phase1_helper(unsigned* new_community, double* max_diff,
                   unsigned node, Vector<unsigned>* communities,
                   SparseNetwork* network, unsigned start, unsigned end, unsigned idx) {
    *new_community = (*communities)[start++];
    if (*new_community == -1) {
        *max_diff = -1;
        return;
    }
    unsigned mod = (end - start) / 100 + 1;
    unsigned start_original = start;
    *max_diff = network->static_modularity_diff(node, *new_community);
    for (; start < end; ++start) {
        if ((*communities)[start - 1] == (*communities)[start])
            continue;
        if ((*communities)[start] == -1)
            continue;

        unsigned other_comm = (*communities)[start];
        double diff = network->static_modularity_diff(node, other_comm);
        if (diff > *max_diff) {
            *max_diff = diff;
            *new_community = other_comm;
        }
        if (start % mod == 0) {
            for (unsigned i = 0; i < idx; ++i)
                std::cout << "    ";
            std::cout << ((start - start_original) / mod) << '%' << std::endl;
        }
    }
}

void Louvain::phase1() {
    bool verbose = true;
    if (verbose)
        std::cout << "Starting phase 1" << std::endl;
    bool repeat = false;
    
    double old_modularity = network.modularity();
    double new_modularity = network.modularity();
    do {
        old_modularity = new_modularity;
        // This groups nodes by community when , which is worse than 
        for (unsigned i = 0; i < network.n; ++i) {
            std::cout << i << " / " << network.n << std::endl;
            //std::cout << network.modularity() << std::endl;
            Vector<unsigned> communities;
            unsigned community = network.reverse_communities[i];
            unsigned node = network.remove_from_community(community);
            communities.push_back(community);
            for (unsigned j = 0; j < network.out_degrees[node]; ++j) {
                communities.push_back(network.reverse_communities[network.from_to[node][j]]);
            }
            for (unsigned j = 0; j < network.in_degrees[node]; ++j) {
                communities.push_back(network.reverse_communities[network.to_from[node][j]]);
            }
            quicksort(communities.data(), 0, communities.size() - 1);
            unsigned new_community = -1;
            std::cout << communities.size() << std::endl;
            if (communities.size() >= 50000) {
                unsigned base = communities.size() / 8;
                unsigned new_communities[8];
                double max_diffs[8];
                unsigned starts[8]{0, base, 2 * base, 3 * base, 4 * base, 5 * base, 6 * base, 7 * base};
                unsigned ends[8]{base, 2 * base, 3 * base, 4 * base, 5 * base, 6 * base, 7 * base, communities.size()};
                std::thread t0(phase1_helper, new_communities, max_diffs, 
                               node, &communities, &network, starts[0], ends[0], 0);
                std::thread t1(phase1_helper, new_communities + 1, max_diffs + 1, 
                               node, &communities, &network, starts[1], ends[1], 1);
                std::thread t2(phase1_helper, new_communities + 2, max_diffs + 2, 
                               node, &communities, &network, starts[2], ends[2], 2);
                std::thread t3(phase1_helper, new_communities + 3, max_diffs + 3, 
                               node, &communities, &network, starts[3], ends[3], 3);
                std::thread t4(phase1_helper, new_communities + 4, max_diffs + 4, 
                               node, &communities, &network, starts[4], ends[4], 4);
                std::thread t5(phase1_helper, new_communities + 5, max_diffs + 5, 
                               node, &communities, &network, starts[5], ends[5], 5);
                std::thread t6(phase1_helper, new_communities + 6, max_diffs + 6, 
                               node, &communities, &network, starts[6], ends[6], 6);
                std::thread t7(phase1_helper, new_communities + 7, max_diffs + 7, 
                               node, &communities, &network, starts[7], ends[7], 7);
                t0.join();
                t1.join();
                t2.join();
                t3.join();
                t4.join();
                t5.join();
                t6.join();
                t7.join();
                double max_diff = -1;
                for (unsigned j = 0; j < 8; ++j) {
                    if (max_diffs[j] > max_diff) {
                        max_diff = max_diffs[j];
                        new_community = new_communities[j];
                    }
                }
            }
            else {
                new_community = communities.front();
                double max_diff = network.static_modularity_diff(node, new_community);
                unsigned mod = communities.size() / 100 + 1;
                for (unsigned j = 1; j < communities.size(); ++j) {
                    if (communities[j - 1] == communities[j])
                        continue;
                    if (communities[j] == -1)
                        continue;
                    unsigned other_comm = communities[j];
                    double diff =  network.static_modularity_diff(node, other_comm);
                    if (diff > max_diff) {
                        max_diff = diff;
                        new_community = other_comm;
                    }
                    if (j % mod == 0)
                        std::cout << (j / mod) << '%' << std::endl;
                }
            }
            network.add_to_community(node, new_community);
            if (network.communities[community].size() < network.communities[community].capacity() / 4) {
                network.communities[community].shrink_to_fit();
            }
        }
        new_modularity = network.modularity();
        if (verbose)
            std::cout << "Modularity: " << new_modularity << std::endl;

    } while (new_modularity > old_modularity);

    for (unsigned i = 0; i < original_n; ++i) {
        final_reverse_communities[i] = network.reverse_communities[final_reverse_communities[i]];
    }
}

void Louvain::phase2() {
    bool verbose = true;
    reindex_communities();
    merge_communities();
    if (verbose)
        std::cout << "Num communities: " << new_num_comm << std::endl;
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

void Louvain::dump_from_to(const std::string& filename) {
    network.dump_from_to(filename);
}

void Louvain::dump_to_from(const std::string& filename) {
    network.dump_to_from(filename);
}

void Louvain::dump_communities(const Vector<std::string>& filenames) {
    if (filenames.size() != final_communities.size())
        throw std::invalid_argument("Invalid number of filenames.");

    for (unsigned i = 0; i < filenames.size(); ++i) {
        dump_1D(final_communities[i].data(), final_communities[i].size(), filenames[i]);
    }
}

void Louvain::dump_reverse_communities(const std::string& filename) {
    dump_1D(final_reverse_communities.data(), final_reverse_communities.size(), filename);
}

// THIS REQUIRES THE COMMUNITIES TO HAVE BEEN SET BY SET_COMMUNITIES,
// BECAUSE RUNNING LOUVAIN NORMALLY MERGES THE NODES SO IT'S IMPOSSIBLE
// TO GET THE ORIGINAL NETWORK BACK. I PURPOSEFULLY MODIFY THE ORIGINAL
// TO DUMP MEMORY. SET_COMMUNITIES DOESN'T SET LOUVAIN VARIABLES, ONLY
// THE UNDERLYING NETWORK VARIABLES. THIS IS A BAD IDEA.
void Louvain::dump_partitions(const Vector<std::string>& filenames) {
    // this is kind of terrible. I can't be bothered to make it not
    // terrible. I think this works, but I haven't extensively tested
    // it.
    if (filenames.size() != network.num_communities) {
        std::cout << filenames.size() << ' ' << network.num_communities << std::endl;
        throw std::invalid_argument("Invalid number of filenames.");
    }

    for (unsigned i = 0; i < network.num_communities; ++i) {
        unsigned community_size = network.communities[i].size();
        unsigned* map = new unsigned[network.n];
        for (unsigned j = 0; j < community_size; ++j) {
            auto qwer = network.communities[i][j];
            map[network.communities[i][j]] = j;
        }
        
        unsigned* out_degrees = new unsigned[community_size];
        for (unsigned j = 0; j < community_size; ++j) {
            out_degrees[j] = 0;
            unsigned node = network.communities[i][j];
            for (unsigned k = 0; k < network.out_degrees[node]; ++k) {
                unsigned other_node = network.from_to[node][k];
                if (network.reverse_communities[node] != network.reverse_communities[other_node])
                    continue;
                
                out_degrees[j]++;
            }
        }
        
        Vector<unsigned>* from_to = new Vector<unsigned>[community_size];
        for (unsigned j = 0; j < community_size; ++j) {
            unsigned node = network.communities[i][j];
            for (unsigned k = 0; k < network.out_degrees[node]; ++k) {
                unsigned other_node = network.from_to[node][k];
                if (network.reverse_communities[node] != network.reverse_communities[other_node])
                    continue;
                from_to[j].push_back(map[other_node]);
            }
            from_to[j].shrink_to_fit();
        }

        unsigned** raw_from_to = new unsigned*[community_size];
        for (unsigned j = 0; j < community_size; ++j) {
            raw_from_to[j] = from_to[j].data();
        }

        dump_2D(raw_from_to, community_size, out_degrees, filenames[i]);
        delete[] raw_from_to;
        delete[] from_to;
        delete[] map;
        delete[] out_degrees;
    }
}

// FOR NOW THIS IS ONLY USED BEFORE dump_PARTITIONS. IT ONLY SETS THE
// BARE NECESSITIES LOUVAIN WON'T BE FUNCTIONAL AFTER THIS IS CALLED
void Louvain::set_communities(unsigned* reverse_communities) {
    network.set_communities(reverse_communities, false);
}

void Louvain::set_communities(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        // handle error
        throw std::runtime_error("file does not exist, is empty, or is badly formatted");
    }

    Vector<unsigned> communities;
    while (ifs.eof() == false) {
        unsigned value;
        ifs.read(reinterpret_cast<char*>(&value), sizeof(value));
        communities.push_back(value);
    }

    unsigned* raw_communities = new unsigned[communities.size()];
    for (unsigned i = 0; i < communities.size(); ++i) {
        raw_communities[i] = communities[i];
    }

    network.set_communities(raw_communities, true);
}

double Louvain::modularity() {
    return network.modularity();
}

unsigned Louvain::num_communities() {
    return network.num_communities;
}