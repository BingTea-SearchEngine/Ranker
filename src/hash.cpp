#include "static/hash.h"

URLHash::URLHash() {}

URLHash::URLHash(const Vector<std::string>& filenames) {
    //std::vector<char*> urls;
    Vector<char*> urls;
    for (unsigned i = 0; i < filenames.size(); ++i) {
        // Open the file
        std::ifstream ifs(filenames[i]);
        if (!ifs) {
            std::cerr << "Error opening file." << std::endl;
            return;
        }
        
        std::string url;
        while (ifs >> url) {
            urls.push_back(new char[url.length() + 1]);
            strcpy(urls.back(), url.c_str());
        }
        
        // Close the file
        ifs.close();
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
        std::ifstream ifs(filenames[i]);
        if (!ifs) {
            std::cerr << "Error opening file." << std::endl;
            return;
        }
        
        std::string url;
        
        while (ifs >> url) {
            urls[index] = new char[url.length() + 1];
            strcpy(urls[index++], url.c_str());
        }
        
        // Close the file
        ifs.close();
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

const unsigned URLHash::operator[](const char* key) const {
    return cmph_search(hash, key, (cmph_uint32)strlen(key));
}

const unsigned URLHash::operator[](std::string key) const {
    const char* c_key = key.c_str();
    return operator[](c_key);
}

void URLHash::print_hash(const char* filename) {
    Vector<char*> urls;
    std::ifstream ifs(filename);
    if (!ifs) {
        std::cerr << "Error opening file." << std::endl;
        return;
    }
    
    std::string url;
    while (ifs >> url) {
        urls.push_back(new char[url.length() + 1]);
        strcpy(urls.back(), url.c_str());
    }
    
    // Close the file
    ifs.close();

    for (unsigned i = 0; i < urls.size(); ++i) {
        const char *key = urls[i];
        unsigned int id = cmph_search(hash, key, (cmph_uint32)strlen(key));
        std::cout << "key: " << key << " -- hash: " << id << '\n';
    }
}

void URLHash::print_hash(const std::string& filename) {
    print_hash(filename.c_str());
}