#include <iostream>

#include <chrono>

#include "static/tree.h"
#include "static/vector.h"
#include "static/network.h"
#include "static/algorithm.h"
#include "static/deque.h"
#include "static/louvain.h"
#include <cassert>
#include <cstdlib>
#include <deque>
#include <xgboost/c_api.h>
#include "static/google.h"
#include "cmph.h"
#include "static/hash.h"

int main() {
    //URLHash hash;
    
    ///*
    Vector<std::string> filenames;
    filenames.push_back("../data/random_strings.txt");
    //URLHash hash("../data/url_hash.txt");
    URLHash hash(filenames, 100000);
    hash.dump("../data/url_hash.txt");
    std::cout << "asdf" << std::endl;
    //hash.test(filenames, "");
    //*/
    /*
    Louvain louvain("../data/real.network");
    louvain.save_to_from("../data/real_tf.network"); // For CheiRank

    louvain.partition();
    std::cout << "modularity " << louvain.modularity() << std::endl;
    auto communities = louvain.get_communities();
    auto reverse_communities = louvain.get_reverse_communities();

    // Save communities to file
    louvain.save_reverse_communities("../data/real.rcomm");
    Vector<std::string> filenames;
    for (unsigned i = 0; i < communities.size(); ++i) {
        filenames.push_back("../data/communities/" + std::to_string(i) + ".comm");
    }
    louvain.save_communities(filenames);
    */
    

    /*
    Louvain louvain("../data/real.network");
    louvain.set_communities("../data/real.rcomm");
    std::cout << louvain.modularity() << std::endl;
    Vector<std::string> filenames;
    for (unsigned i = 0; i < louvain.num_communities(); ++i) {
        filenames.push_back("../data/communities/" + std::to_string(i) + ".network");
    }
    louvain.save_partitions(filenames);
    */

    /*
    GoogleMatrix google("../data/real.network");
    auto pagerank = google.pagerank(0.85);
    for (unsigned i = 0; i < pagerank.size(); ++i) {
        std::cout << pagerank[i] << std::endl;
    }
    */

    /*
    GoogleMatrix google("../data/real_tf.network");
    auto cheirank = google.pagerank(0.85);
    for (unsigned i = 0; i < cheirank.size(); ++i) {
        std::cout << cheirank[i] << std::endl;
    }
    */
}