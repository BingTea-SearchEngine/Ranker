Installation instructions
```
git clone https://github.com/BingTea-SearchEngine/Ranker.git
cd Ranker
git clone --recursive https://github.com/dmlc/xgboost
```
Setup CMake
```
mkdir build
cd build
cmake -S .. -B .
make
```

Hash (THIS NEEDS TO BE DONE SEPARATELY TO LOUVAIN, THERES SOME WEIRD
MEMORY MANAGEMENT IN CMPH. SOMETHING GETS DELETED PREMATURELY THAT
CAUSES AN ERROR)
```
Vector<std::string> filenames;
filenames.push_back("../data/config_urls.txt");
URLHash hash(filenames);
hash.dump("../data/config.hash");
```


Community detection with Louvain
```
// Load network from file
Louvain louvain("../data/real.network");
louvain.save_to_from("../data/real_tf.network"); // For CheiRank

// Partition communities
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
```

Saving paritions (THESE NEEDS TO BE DONE SEPERATELY, THE CODE FOR THIS
IS DISGUSTING AND FOR NOW ONLY WORKS IT ONLY WORKS LIKE THIS)
```
louvain("../data/real.network");
louvain.set_communities("../data/real.rcomm");
std::cout << louvain.modularity() << std::endl;
Vector<std::string> filenames;
for (unsigned i = 0; i < louvain.num_communities(); ++i) {
    filenames.push_back("../data/communities/" + std::to_string(i) + ".network");
}
louvain.save_partitions(filenames);
```

Computing PageRank of overall network
```
GoogleMatrix google("../data/real.network");
auto pagerank = google.pagerank(0.85);
```

Computing CheiRank of overall network
```
GoogleMatrix google("../data/real_tf.network");
auto cheirank = google.pagerank(0.85);
```

Computing PageRank within community
```
GoogleMatrix google("1_ft.network");
auto pagerank = google.pagerank(0.85);
```

Computing CheiRank within community\
First get to_from
```
SparseNetwork network("1_ft.network");
network.save_to_from("1_tf.network");
```
Then calculate
```
GoogleMatrix google("1_tf.network");
auto cheirank = google.pagerank(0.85);
```
