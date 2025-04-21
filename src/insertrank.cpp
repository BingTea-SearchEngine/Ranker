#include "static/hash.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>

namespace fs = std::filesystem;
using std::cout, std::endl;

int main(int argc, char**argv) {
    if (argc < 5) {
        cout << "Not enough arguments" << endl;
        exit(EXIT_FAILURE);
    }

    std::string htmlDirPath = std::string(argv[1]);
    std::string pageRankFilePath = std::string(argv[2]);
    std::string cheiRankFilePath = std::string(argv[3]);
    std::string hashFilePath = std::string(argv[4]);

    URLHash hash(hashFilePath, true);

    std::vector<float> pageranks;
    pageranks.reserve(37941676);
    {
    std::ifstream pageRankFile(pageRankFilePath);
    std::string line;
    while(std::getline(pageRankFile, line)) {
        float rank = std::stof(line);
        pageranks.push_back(rank);
    }
    }
    cout << pageranks.size() << endl;

    std::vector<float> cheiranks;
    cheiranks.reserve(37941676);
    {
    std::string line;
    std::ifstream cheiRankFile(cheiRankFilePath);
    while(std::getline(cheiRankFile, line)) {
        float rank = std::stof(line);
        cheiranks.push_back(rank);
    }
    }
    cout << cheiranks.size() << endl;
    //

    int count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(htmlDirPath)) {
        if (!entry.is_regular_file()) continue;
        std::fstream f(entry.path(), std::ios::in | std::ios::out);
        std::string line;
        std::getline(f, line);

        std::regex url_doc_regex(R"(URL:\s*(\S+)\s+Doc number:\s*(\d+))");
        std::smatch match;
        std::string url;
        if (std::regex_search(line, match, url_doc_regex)) {
            url = match[1];
        } else {
            continue;
        }
        cout << url << endl;
        unsigned row = hash[url];
        cout << "Page rank " << pageranks[row] << endl;
        cout << "Chei rank " << cheiranks[row] << endl;
        f.clear();
        f.seekp(0, std::ios::end);
        f << "<prank>\n";
        f << pageranks[row] << "\n";
        f << "</prank>\n";
        f << "<crank>\n";
        f << cheiranks[row] << "\n";
        f << "</crank>\n";
        f.close();

        cout << count << endl;
        count++;
    }
    return 0;
}
