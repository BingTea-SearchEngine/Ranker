#include "static/hash.h"

URLHash::URLHash() {}

URLHash::URLHash(const Vector<std::string>& filenames) {
    std::vector<char*> urls;
    for (unsigned i = 0; i < filenames.size(); ++i) {
        // Open the file
        std::ifstream in(filenames[i]);
        if (!in) {
            std::cerr << "Error opening file." << std::endl;
            return;
        }
        
        std::string url;
        
        while (in >> url) {
            urls.push_back(new char[url.length() + 1]);
            strcpy(urls.back(), url.c_str());
        }
        
        // Close the file
        in.close();
    }
    urls.shrink_to_fit();
    nkeys = urls.size();

    source = cmph_io_vector_adapter((char**)urls.data(), nkeys);
    config = cmph_config_new(source);
    cmph_config_set_algo(config, CMPH_CHD);
    hash = cmph_new(config);
    cmph_io_vector_adapter_destroy(source);

    for (unsigned i = 0; i < nkeys; ++i) {
        delete[] urls[i];
    }
}

URLHash::URLHash(const Vector<std::string>& filenames, unsigned num_pages)
  : nkeys(num_pages) {
    char** urls = new char*[nkeys];
    unsigned index = 0;
    for (unsigned i = 0; i < filenames.size(); ++i) {
        // Open the file
        std::ifstream in(filenames[i]);
        if (!in) {
            std::cerr << "Error opening file." << std::endl;
            return;
        }
        
        std::string url;
        
        while (in >> url) {
            urls[index] = new char[url.length() + 1];
            strcpy(urls[index++], url.c_str());
        }
        
        // Close the file
        in.close();
    }

    source = cmph_io_vector_adapter((char**)urls, nkeys);
    config = cmph_config_new(source);
    cmph_config_set_algo(config, CMPH_CHD);
    hash = cmph_new(config);
    cmph_io_vector_adapter_destroy(source);

    for (unsigned i = 0; i < nkeys; ++i) {
        delete[] urls[i];
    }
    delete[] urls;
    std::cout << nkeys << std::endl;
}

URLHash::URLHash(const std::string& filename) {
    FILE* mphf_fd = fopen(filename.c_str(), "r");
    hash = cmph_load(mphf_fd);
    fclose(mphf_fd);
}

URLHash::~URLHash() {
    cmph_destroy(hash);
    cmph_config_destroy(config);
}

void URLHash::dump(const std::string& filename) {
    FILE* mphf_fd = fopen(filename.c_str(), "w");
    cmph_config_set_mphf_fd(config, mphf_fd);
    cmph_dump(hash, mphf_fd);
    fclose(mphf_fd);
}

void URLHash::test(const Vector<std::string>& filenames, const std::string& filename1) {
    std::vector<char*> urls;
    for (unsigned i = 0; i < filenames.size(); ++i) {
        // Open the file
        std::ifstream in(filenames[i]);
        if (!in) {
            std::cerr << "Error opening file." << std::endl;
            return;
        }
        
        std::string url;
        
        while (in >> url) {
            urls.push_back(new char[url.length() + 1]);
            strcpy(urls.back(), url.c_str());
        }
        
        // Close the file
        in.close();
    }
    urls.shrink_to_fit();
    nkeys = urls.size();

    unsigned i = 0;
    auto qwer = "asdf";
    auto asdf = cmph_search(hash, qwer, (cmph_uint32)strlen(qwer));
    fprintf(stderr, "key:%s -- hash:%u\n", qwer, asdf);
    while (i < nkeys) {
        const char *key = urls[i];
        unsigned int id = cmph_search(hash, key, (cmph_uint32)strlen(key));
        fprintf(stderr, "key:%s -- hash:%u\n", key, id);
        i++;
    }
    for (unsigned i = 0; i < urls.size(); ++i) {
        delete[] urls[i];
    }
}
