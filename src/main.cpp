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

int main() {
    unsigned* first = new unsigned[11]{0, 1, 2, 3, 0, 4, 5, 6, 7, 1, 6};
    unsigned* second = new unsigned[11]{1, 2, 3, 0, 2, 5, 6, 7, 4, 4, 2};
    //SparseNetwork network(8, 11, first, second);
    //unsigned* communities = new unsigned[8]{2, 0, 0, 0, 1, 1, 1, 1};
    //network.set_communities(communities);
    Louvain louvain(8, 11, first, second);
    louvain.phase1();
    louvain.phase2();
    //auto modularity = network.modularity();
    //network.add_to_community(0, 0);
    //std::cout << "modularity " << modularity << std::endl;
    delete[] first;
    delete[] second;
    //delete[] communities;
}

/*

0  1  1  0  0  0  0  0
0  0  1  0  1  0  0  0
0  0  0  1  0  0  0  0
1  0  0  0  0  0  0  0
0  0  0  0  0  1  0  0
0  0  0  0  0  0  1  0
0  0  1  0  0  0  0  1
0  0  0  0  1  0  0  0

2 6 1 6

*/