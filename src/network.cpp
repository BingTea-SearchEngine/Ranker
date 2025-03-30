#include "static/network.h"

SparseNetwork::SparseNetwork() {}

SparseNetwork::SparseNetwork(int num_edges, int* first, int* second)
  : n(num_edges) {}