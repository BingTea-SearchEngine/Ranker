Community detection with Louvain
```
// Load network from file
Louvain louvain("from_to.network");

// Partition communities
louvain.partition();
auto communities = louvain.get_communities();
auto reverse_communities = louvain.get_reverse_communities();

// Save communities to file
louvain.save_to_from("to_from.network"); // For CheiRank
louvain.save_reverse_communities("example.rcomm");
Vector<std::string> filenames{"1.comm", "2.comm", ...};
louvain.save_communities(filenames);
```

Saving paritions (THESE NEEDS TO BE DONE SEPERATELY, THE CODE FOR THIS
IS DISGUSTING AND FOR NOW ONLY WORKS IT ONLY WORKS LIKE THIS)
```
Louvain louvain("from_to.network");
louvain.set_communities("example.rcomm");
Vector<std::string> filenames{"1_ft.network", "2_ft.network", ...};
louvain.save_partitions(filenames);
```

Computing PageRank of overall network
```
GoogleMatrix google("from_to.network");
auto pagerank = google.pagerank(0.85);
```

Computing CheiRank of overall network
```
GoogleMatrix google("to_from.network");
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

