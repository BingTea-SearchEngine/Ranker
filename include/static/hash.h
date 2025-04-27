#ifndef HASH_H
#define HASH_H

#include "cmph.h"
#include "static/vector.h"
#include <string>
#include <fstream>
#include <iostream>
#include <string.h>
#include <limits>
//#include <vector>

class URLHash {
    private:
        cmph_config_t* config;
        cmph_t* hash;
        cmph_io_adapter_t* source;
        unsigned nkeys;
    public:
        URLHash();
        URLHash(const std::string& filename, bool hashed);
        URLHash(const std::string& filename, unsigned num_pages);
        ~URLHash();
        void dump(const std::string& filename);
        const unsigned operator[](const char* key) const;
        const unsigned operator[](std::string key) const;
        void print(const char* filename);
        void print(const std::string& filename);
};

#endif