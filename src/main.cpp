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
    // Open the file
    std::ifstream inputFile("../data/random_strings.txt");
    if (!inputFile) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }
    
    // Vector to store each line
    std::vector<std::string> lines;
    lines.reserve(100000);
    std::string line;
    
    // Read each line and store it in the vector
    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }
    
    // Close the file
    inputFile.close();


    // Creating a filled vector
    unsigned int i = 0;
    //const char *vector[] = {"aaaaaaaaaa", "bbbbbbbbbb", "cccccccccc", "dddddddddd", "eeeeeeeeee", 
    //    "ffffffffff", "gggggggggg", "hhhhhhhhhh", "iiiiiiiiii", "jjjjjjjjjj"};
    //unsigned int nkeys = 10;
    unsigned nkeys = lines.size();
    const char *vector[nkeys];
    for (unsigned j = 0; j < nkeys; ++j) {
        vector[j] = lines[j].c_str();
    }
    
    FILE* mphf_fd = fopen("temp.mph", "w");
    // Source of keys
    cmph_io_adapter_t *source = cmph_io_vector_adapter((char **)vector, nkeys);

    //Create minimal perfect hash function using the brz algorithm.
    cmph_config_t *config = cmph_config_new(source);
    cmph_config_set_algo(config, CMPH_CHD);
    cmph_config_set_mphf_fd(config, mphf_fd);
    cmph_t *hash = cmph_new(config);
    cmph_config_destroy(config);
    cmph_dump(hash, mphf_fd); 
    cmph_destroy(hash);	
    fclose(mphf_fd);

    //Find key
    mphf_fd = fopen("temp.mph", "r");
    hash = cmph_load(mphf_fd);
    while (i < nkeys) {
        const char *key = vector[i];
        unsigned int id = cmph_search(hash, key, (cmph_uint32)strlen(key));
        fprintf(stdout, "key:%s -- hash:%u\n", key, id);
        i++;
    }

    //Destroy hash
    cmph_destroy(hash);
    cmph_io_vector_adapter_destroy(source);
    fclose(mphf_fd);
    return 0;
    
    
    //URLHash hash;
    
    ///*
    //Vector<std::string> filenames;
    //filenames.push_back("../data/urls.txt");
    //URLHash hash("../data/url_hash.txt");
    //URLHash hash(filenames, 3031);
    //hash.dump("../data/url_hash.txt");
    //std::cout << "asdf" << std::endl;
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