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
    std::string louvainCommunityPath = std::string(argv[4]);
    std::string louvainCommunityCountPath = std::string(argv[5]);
    std::string hashFilePath = std::string(argv[6]);

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
    cout << "Page rank in memory" << endl;

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
    cout << "Chei rank in memory" << endl;

    std::vector<int> communities;
    communities.reserve(37941676);
    {
    std::string line;
    std::ifstream communitiesFile(louvainCommunityPath);
    while(std::getline(communitiesFile, line)) {
        int community = std::stoi(line);
        communities.push_back(community);
    }
    }
    cout << "Communities in memory" << endl;

    std::vector<int> communityCounts;
    communityCounts.reserve(37941676);
    {
    std::string line;
    std::ifstream communitiesCountFile(louvainCommunityCountPath);
    while(std::getline(communitiesCountFile, line)) {
        int count = std::stoi(line);
        communityCounts.push_back(count);
    }
    }
    cout << "Communty counts in memory" << endl;

    int count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(htmlDirPath)) {
        if (!entry.is_regular_file()) continue;

        std::ifstream f(entry.path(), std::ios::in);
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.close();

        std::istringstream stream(content);
        std::regex url_doc_regex(R"(URL:\s*(\S+)\s+Doc number:\s*(\d+))");
        std::string line;
        std::getline(stream, line);

        std::smatch match;
        std::string url;
        if (std::regex_search(line, match, url_doc_regex)) {
            url = match[1];
        } else {
            continue;
        }

        unsigned row = hash[url];
        cout << "Url: " << url << "Row: " << row << endl;
        bool hasPrank = content.find("<prank>") != std::string::npos;
        bool hasCrank = content.find("<crank>") != std::string::npos;
        bool hasCommunity = content.find("<community>") != std::string::npos;

        std::ostringstream oss;
        oss << std::scientific << std::setprecision(6);
        if (!hasPrank) {
            oss << "<prank>\n" << pageranks[row] << "\n</prank>\n";
        }
        if (!hasCrank) {
            oss << "<crank>\n" << cheiranks[row] << "\n</crank>\n";
        }
        if (!hasCommunity) {
            oss <<  "<community>\n" << communities[row] << "\n</community>\n";
            oss <<  "<communitycount>\n" << communityCounts[row] << "\n</communitycount>\n";
        }

        std::string toAppend = oss.str();
        if (!toAppend.empty()) {
            std::ofstream out(entry.path(), std::ios::app);
            out << toAppend;
            out.close();
        }

        cout << count << "\n";
        count++;
    }
    return 0;
}
