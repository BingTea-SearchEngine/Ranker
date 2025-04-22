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
                   SparseNetwork* network, double resolution, unsigned start, unsigned end, unsigned idx) {
    *new_community = (*communities)[start++]; // TODO: maybe get rid of it
    if (*new_community == -1) {
        *max_diff = -1;
        return;
    }
    unsigned mod = (end - start) / 100 + 1;
    unsigned start_original = start;
    *max_diff = network->static_modularity_diff(node, *new_community, resolution);
    for (; start < end; ++start) {
        if ((*communities)[start - 1] == (*communities)[start])
            continue;
        if ((*communities)[start] == -1)
            continue;

        unsigned other_comm = (*communities)[start];
        double diff = network->static_modularity_diff(node, other_comm, resolution);
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

void phase1_helper_desperate(unsigned* new_community, double* max_diff,
                             unsigned node, Vector<unsigned>* communities,
                             SparseNetwork* network, double resolution, unsigned start, unsigned end, 
                             unsigned idx) {
    /*
    Currently, iterate through communities and get the modularity diff
    for each once. Calculating modularity diff for a single community
    loops through all connections.

    Instead, iterate through all connections a single time and
    increment scores in a single giant array.
    */ 

    double* modularity_diffs = new double[end - start];
    for (unsigned i = 0; i < end - start; ++i)
        modularity_diffs[i] = 0;

    for (unsigned i = 0; i < network->out_degrees[node]; ++i) {
        unsigned other = network->from_to[node][i];
        unsigned other_community = network->reverse_communities[other];
        if (other_community < start || other_community >= end)
            continue;
        if (other_community == -1)
            continue; // should be equivalent to break, since it'll be at the end

        modularity_diffs[other_community - start] += network->weights_from_to[node][i];
    }

    for (unsigned i = 0; i < network->in_degrees[node]; ++i) {
        unsigned other = network->to_from[node][i];
        unsigned other_community = network->reverse_communities[other];
        if (other_community < start || other_community >= end)
            continue;
        if (other_community == -1)
            break; // should be equivalent to break, since it'll be at the end

        modularity_diffs[other_community - start] += network->weights_to_from[node][i];
    }

    for (unsigned i = 0; i < end - start; ++i) {
        unsigned community = i + start;
        double expected = double(network->out_weights[node] * network->community_in_weights[community]
                    + network->in_weights[node] * network->community_out_weights[community]) / network->m;
        modularity_diffs[i] = (modularity_diffs[i] - expected) / network->m;
    }

    for (unsigned i = 0; i < end - start; ++i) {
        if (modularity_diffs[i] > *max_diff) {
            *max_diff = modularity_diffs[i];
            *new_community = i + start;
        } 
    }
    delete[] modularity_diffs;
}

void Louvain::phase1(double resolution) {
    bool verbose = true;
    if (verbose)
        std::cout << "Starting phase 1" << std::endl;
    bool repeat = false;
    
    //double old_modularity;
    //double new_modularity = network.modularity();
    for (unsigned iteration = 0; iteration < 20; ++iteration) {
        //old_modularity = new_modularity;
        // This groups nodes by community when , which is worse than 
        // 24690764
        unsigned bar_length = 150;
        unsigned mod = max(1u, network.n / bar_length);
        std::cout << '[';
        for (unsigned i = 0; i < bar_length; ++i) {
            std::cout << ' ';
        }
        std::cout << ']' << std::endl;
        std::cout << ' ';

        bool repeat = false;
        double total_diff = 0;
        for (unsigned i = 0; i < network.n; ++i) {
            //std::cout << i << " / " << network.n << std::endl;
            //std::cout << network.modularity() << std::endl;
            Vector<unsigned> communities;
            unsigned community = network.reverse_communities[i];
            unsigned node = network.remove_from_community(community);
            double old_diff = network.static_modularity_diff(node, community, resolution);
            communities.push_back(community);
            for (unsigned j = 0; j < network.out_degrees[node]; ++j) {
                communities.push_back(network.reverse_communities[network.from_to[node][j]]);
            }
            for (unsigned j = 0; j < network.in_degrees[node]; ++j) {
                communities.push_back(network.reverse_communities[network.to_from[node][j]]);
            }
            quicksort(communities.data(), 0, communities.size() - 1);
            Vector<unsigned> unique_communities;
            unique_communities.reserve(communities.size());
            unique_communities.push_back(communities.front());
            for (unsigned j = 1; j < communities.size(); ++j) {
                if (communities[j - 1] != communities[j])
                    unique_communities.push_back(communities[j]);
            }
            communities.clear();
            communities.shrink_to_fit();
            unsigned new_community = -1;
            double new_diff = -1;
            /*
            phase1_helper_desperate(&new_community, &max_diff, 
                node, &unique_communities, &network, 0, network.num_communities, 0);
            */
            
            if (unique_communities.size() >= 3000) {
                phase1_helper_desperate(&new_community, &new_diff, node, 
                                        &unique_communities, &network, resolution, 0, 
                                        network.num_communities, 0);
                if (new_diff - old_diff > 1e-12)
                    repeat = true;
                /*
                unsigned base = unique_communities.size() / 8;
                unsigned new_communities[8];
                double max_diffs[8];
                unsigned starts[8]{0, base, 2 * base, 3 * base, 4 * base, 5 * base, 6 * base, 7 * base};
                unsigned ends[8]{base, 2 * base, 3 * base, 4 * base, 5 * base, 6 * base, 7 * base, communities.size()};
                std::thread t0(phase1_helper, new_communities, max_diffs, 
                               node, &unique_communities, &network, resolution, starts[0], ends[0], 0);
                std::thread t1(phase1_helper, new_communities + 1, max_diffs + 1, 
                               node, &unique_communities, &network, resolution, starts[1], ends[1], 1);
                std::thread t2(phase1_helper, new_communities + 2, max_diffs + 2, 
                               node, &unique_communities, &network, resolution, starts[2], ends[2], 2);
                std::thread t3(phase1_helper, new_communities + 3, max_diffs + 3, 
                               node, &unique_communities, &network, resolution, starts[3], ends[3], 3);
                std::thread t4(phase1_helper, new_communities + 4, max_diffs + 4, 
                               node, &unique_communities, &network, resolution, starts[4], ends[4], 4);
                std::thread t5(phase1_helper, new_communities + 5, max_diffs + 5, 
                               node, &unique_communities, &network, resolution, starts[5], ends[5], 5);
                std::thread t6(phase1_helper, new_communities + 6, max_diffs + 6, 
                               node, &unique_communities, &network, resolution, starts[6], ends[6], 6);
                std::thread t7(phase1_helper, new_communities + 7, max_diffs + 7, 
                               node, &unique_communities, &network, resolution, starts[7], ends[7], 7);
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
                */
            }
            else {
                new_community = unique_communities.front();
                new_diff = network.static_modularity_diff(node, new_community, resolution);
                //unsigned mod = unique_communities.size() / 100 + 1;
                for (unsigned j = 1; j < unique_communities.size(); ++j) {
                    if (unique_communities[j] == -1)
                        continue;
                    unsigned other_comm = unique_communities[j];
                    double diff =  network.static_modularity_diff(node, other_comm, resolution);
                    if (diff > new_diff) {
                        new_diff = diff;
                        new_community = other_comm;
                    }
                    //if (j % mod == 0)
                    //    std::cout << (j / mod) << '%' << std::endl;
                }
            }
            total_diff += new_diff - old_diff;
            
            // 9.04513e-05
            network.add_to_community(node, new_community);
            if (network.communities[community].size() < network.communities[community].capacity() / 4) {
                network.communities[community].shrink_to_fit();
            }
            if (i % mod == 0)
                std::cout << "█" << std::flush;
        }
        std::cout << "\nModularity increase: " << total_diff << std::endl;
        unsigned* temp_reverse_communities = new unsigned[original_n];
        for (unsigned i = 0; i < original_n; ++i) {
            temp_reverse_communities[i] = network.reverse_communities[final_reverse_communities[i]];
        }
        //dump_1D(temp_reverse_communities, original_n, "../data/temp.rcomm");
        delete[] temp_reverse_communities;
        //new_modularity = network.modularity();
        //if (verbose)
        //    std::cout << "Modularity: " << new_modularity << std::endl;
        if (!repeat)
            break;
    }

    for (unsigned i = 0; i < original_n; ++i) {
        final_reverse_communities[i] = network.reverse_communities[final_reverse_communities[i]];
    }
}

void Louvain::phase2() {
    bool verbose = true;
    if (verbose) {
        std::cout << "Starting phase 2" << std::endl;
    }
    reindex_communities();
    unsigned old_num = -1;
    if (verbose) {
        std::cout << "Reindexed" << std::endl;
        old_num = network.num_communities;
    }
    merge_communities();
    if (verbose)
        std::cout << "Reduced communities from " << old_num << " to " << network.num_communities << std::endl;
}

void Louvain::merge_communities() {
    // Either modify data directly here (current doing this), or make a
    // member function that modifies, or just delete the old network and
    // build a new one
    unsigned** to_from = new unsigned*[new_num_comm]();
    unsigned** from_to = new unsigned*[new_num_comm]();
    unsigned** weights_to_from = new unsigned*[new_num_comm]();
    unsigned** weights_from_to = new unsigned*[new_num_comm]();
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

    /*
    For each edge
        Get the out-node
        Get the out-node's community
        Increment the out-community's degree
        Get the in-node
        Get the in-node's community
        Increment the in_community's degree

    Create from_to
    Create to_from

    For each edge
        Get the edge weight
        Get the out-node
        Get the out-node's community
        Get the in-node
        Get the in-node's community
        Increment the from_to by the edge weight
        Increment the to_from by the edge weight
    */
    // Calculate in and out degrees for resulting communities
    for (unsigned i = 0; i < network.n; ++i) {
        unsigned community = network.reverse_communities[i];
        out_degrees[map[community]] += network.out_degrees[i];
    }
    for (unsigned i = 0; i < network.n; ++i) {
        unsigned community = network.reverse_communities[i];
        in_degrees[map[community]] += network.in_degrees[i];
    }

    unsigned** temp_from_to = new unsigned*[new_num_comm];
    unsigned** temp_to_from = new unsigned*[new_num_comm];
    unsigned** temp_weights_from_to = new unsigned*[new_num_comm];
    unsigned** temp_weights_to_from = new unsigned*[new_num_comm];
    for (unsigned i = 0; i < new_num_comm; ++i) {
        temp_from_to[i] = new unsigned[out_degrees[i]];
        temp_to_from[i] = new unsigned[in_degrees[i]];
        temp_weights_from_to[i] = new unsigned[out_degrees[i]];
        temp_weights_to_from[i] = new unsigned[in_degrees[i]];
        for (unsigned j = 0; j < out_degrees[i]; ++j) { // TODO: is this necessary
            temp_from_to[i][j] = 0;
            temp_weights_from_to[i][j] = 0;
        }
        for (unsigned j = 0; j < in_degrees[i]; ++j) {
            temp_to_from[i][j] = 0;
            temp_weights_to_from[i][j] = 0;
        }
        out_degrees[i] = 0;
        in_degrees[i] = 0;
    }

    for (unsigned i = 0; i < network.n; ++i) {
        unsigned out_community = network.reverse_communities[i];
        unsigned new_out_community = map[out_community];
        for (unsigned j = 0; j < network.out_degrees[i]; ++j) {
            unsigned in_community = network.reverse_communities[network.from_to[i][j]];
            unsigned new_in_community = map[in_community];
            unsigned weight = network.weights_from_to[i][j];
            temp_from_to[new_out_community][out_degrees[new_out_community]] = new_in_community;
            temp_to_from[new_in_community][in_degrees[new_in_community]] = new_out_community;
            auto asdf = out_degrees[new_out_community];
            auto qwer = temp_weights_from_to[new_out_community];
            temp_weights_from_to[new_out_community][out_degrees[new_out_community]] = weight;
            temp_weights_to_from[new_in_community][in_degrees[new_in_community]] = weight;
            out_degrees[new_out_community]++;
            in_degrees[new_in_community]++;
        }
    }

    for (unsigned i = 0; i < new_num_comm; ++i) {
        if (out_degrees[i] != 0) {
            unsigned num_unique = 1;
            unsigned* indices = new unsigned[out_degrees[i]];
            for (unsigned j = 0; j < out_degrees[i]; ++j)
                indices[j] = j;
            argsort(indices, temp_from_to[i], 0, out_degrees[i] - 1);
            // Count the number of unique communities
            for (unsigned j = 1; j < out_degrees[i]; ++j) {
                if (temp_from_to[i][indices[j - 1]] != temp_from_to[i][indices[j]])
                    num_unique++;
            }
            from_to[i] = new unsigned[num_unique];
            weights_from_to[i] = new unsigned[num_unique];
            //for (unsigned j = 0; j < out_degrees[i]; ++j)
            //    std::cout << j << ' ' << indices[j] << std::endl;
            from_to[i][0] = temp_from_to[i][indices[0]];
            weights_from_to[i][0] = temp_weights_from_to[i][indices[0]];
            out_weights[i] = temp_weights_from_to[i][indices[0]];
    
            num_unique = 0;
            for (unsigned j = 1; j < out_degrees[i]; ++j) {
                out_weights[i] += temp_weights_from_to[i][indices[j]];
                if (temp_from_to[i][indices[j - 1]] == temp_from_to[i][indices[j]]) {
                    weights_from_to[i][num_unique] += temp_weights_from_to[i][indices[j]];
                }
                else {
                    from_to[i][++num_unique] = temp_from_to[i][indices[j]];
                    weights_from_to[i][num_unique] = temp_weights_from_to[i][indices[j]];
                }
            }
            num_unique++;
            delete[] indices;
            out_degrees[i] = num_unique;
        }

        if (in_degrees[i] != 0) {
            unsigned num_unique = 1;
            unsigned* indices = new unsigned[in_degrees[i]];
            for (unsigned j = 0; j < in_degrees[i]; ++j)
                indices[j] = j;
            argsort(indices, temp_to_from[i], 0, in_degrees[i] - 1);
            for (unsigned j = 1; j < in_degrees[i]; ++j) {
                if (temp_to_from[i][indices[j - 1]] != temp_to_from[i][indices[j]])
                    num_unique++;
            }
            to_from[i] = new unsigned[num_unique];
            weights_to_from[i] = new unsigned[num_unique];
            to_from[i][0] = temp_to_from[i][indices[0]];
            weights_to_from[i][0] = temp_weights_to_from[i][indices[0]];
            in_weights[i] = temp_weights_to_from[i][indices[0]];
    
            num_unique = 0;
            for (unsigned j = 1; j < in_degrees[i]; ++j) {
                in_weights[i] += temp_weights_to_from[i][indices[j]];
                if (temp_to_from[i][indices[j - 1]] == temp_to_from[i][indices[j]]) {
                    weights_to_from[i][num_unique] += temp_weights_to_from[i][indices[j]];
                }
                else {
                    to_from[i][++num_unique] = temp_to_from[i][indices[j]];
                    weights_to_from[i][num_unique] = temp_weights_to_from[i][indices[j]];
                }
            }
            num_unique++;
            delete[] indices;
            in_degrees[i] = num_unique;
        }
        delete[] temp_from_to[i];
        delete[] temp_to_from[i];
        delete[] temp_weights_from_to[i];
        delete[] temp_weights_to_from[i];
    }
    delete[] temp_from_to;
    delete[] temp_to_from;
    delete[] temp_weights_from_to;
    delete[] temp_weights_to_from;
    /*
    unsigned new_comm1 = map[i];
    unsigned new_comm2 = map[j];
    if (weight != 0) {
        from_to[new_comm1][out_degrees[new_comm1]] = new_comm2;
        to_from[new_comm2][in_degrees[new_comm2]] = new_comm1;
        weights_from_to[new_comm1][out_degrees[new_comm1]++] = weight;
        weights_to_from[new_comm2][in_degrees[new_comm2]++] = weight;

        out_weights[new_comm1] += weight;
        in_weights[new_comm2] += weight;
    }
    */
    /*
    unsigned mod = max(1u, network.num_communities / 50);
    for (unsigned i = 0; i < network.num_communities; ++i) {
        auto& community1 = network.communities[i];
        if (community1.empty())
            continue;
        unsigned new_comm1 = map[i];
        for (unsigned j = 0; j < network.num_communities; ++j) {
            auto& community2 = network.communities[j];
            if (community2.empty())
                continue;
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
        if (i % mod == 0)
            std::cout << (i / mod) << '%' << std::endl;
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
        if (community1.empty())
            continue;
        unsigned new_comm1 = map[i];
        for (unsigned j = 0; j < network.num_communities; ++j) {
            auto& community2 = network.communities[j];
            if (community2.empty())
                continue;
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
        if (i % mod == 0)
            std::cout << (i / mod + 50) << '%' << std::endl;
    }
    */
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

void Louvain::partition(double resolution) {
    while (true) {
        phase1(resolution);
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
    std::cout << "num communities: " << network.num_communities << std::endl;
    new_num_comm = 0;
    for (unsigned i = 0; i < original_n; ++i) {
        final_reverse_communities[i] = i;
        new_num_comm = max(new_num_comm, reverse_communities[i]);
    }
    new_num_comm++;
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
    communities.shrink_to_fit();

    unsigned* raw_communities = new unsigned[communities.size()];
    for (unsigned i = 0; i < communities.size(); ++i) {
        raw_communities[i] = communities[i];
    }

    network.set_communities(raw_communities, true);
    new_num_comm = 0; // THIS IS CODE DUPLICATION
    std::cout << "num communities: " << network.num_communities << std::endl;
    for (unsigned i = 0; i < original_n; ++i) {
        final_reverse_communities[i] = i;
        //new_num_comm = max(new_num_comm, raw_communities[i]);
    }
    network.print(false);
}

double Louvain::modularity() {
    return network.modularity();
}

unsigned Louvain::num_communities() {
    return network.num_communities;
}