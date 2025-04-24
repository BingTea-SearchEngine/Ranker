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
#include <vector>
#include <xgboost/c_api.h>
#include "static/google.h"
#include "cmph.h"
#include "static/hash.h"

int main() {
    Vector<char*> urls;
    URLHash hash("../data/links.hash", true);
    std::ifstream url_fs("../data/links.txt");
    std::string url;
    std::string line;
    while (url_fs >> url) {
        //std::cout << url << std::endl;
        unsigned id = hash[url];
        if (id + 1 > urls.size())
            urls.resize(id + 1);
        urls[id] = new char[3];
        urls[id][0] = url[8];
        urls[id][1] = url[9];
        url_fs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        //if (counter++ == 200)
        //    return 0;
    }
    url_fs.close();
    urls.shrink_to_fit();
    std::cout << urls.size() << std::endl;

    std::ifstream train_fs("../data/train_urls.txt");
    std::ofstream train_processes_fs("../data/train_urls_processed.txt");
    std::string qid_str;
    int qid;
    unsigned not_found = 0;
    while (train_fs >> qid_str >> url) {
        qid = stoi(qid_str);
        train_processes_fs << qid_str << ' ' << url + ' ';
        unsigned id = hash[url];
        if (urls[id][0] != url[8] || urls[id][1] != url[9]) {
            train_processes_fs << -1 << '\n';
            not_found++;
        }
        else {
            train_processes_fs << id << '\n';
        }
    }
    train_fs.close();

    std::cout << "total not found: " << not_found << std::endl;
    for (unsigned i = 0; i < urls.size(); ++i) {
        delete[] urls[i];
    }
}