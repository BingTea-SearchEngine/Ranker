#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#include "static/tree.h"

using namespace std;

int main(int argc, char* argv[]) {
    std::vector<std::string> filenames;
    filenames.reserve(argc - 1);
    for (int i = 1; i < argc; ++i)
        filenames.push_back(argv[i]);

    srand(static_cast<unsigned>(time(0)));
    auto train_x = load_2D(filenames[0]);
    auto train_y = load_1D(filenames[1]);
    auto test_x = load_2D(filenames[2]);
    auto test_y = load_1D(filenames[3]);

    Tree tree;
    tree.fit(14, 6, train_x, train_y);
    auto qwer = tree.predict(14, test_x);
    for (int i = 0; i < 14; ++i) {
        std::cout << qwer[i] << std::endl;
    }
    
    return 0;
}