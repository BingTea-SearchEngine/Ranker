Community detection with Louvain
```
Louvain louvain("from_to.network");
louvain.partition();
auto communities = louvain.get_communities();
auto reverse_communities = louvain.get_reverse_communities();
louvain.save_to_from("to_from.network"); // For CheiRank
```

Computing PageRank
```
GoogleMatrix google("from_to.network");
auto pagerank = google.pagerank(0.85);
```

Computing CheiRank
```
GoogleMatrix google("to_from.network");
auto cheirank = google.pagerank(0.85);
```