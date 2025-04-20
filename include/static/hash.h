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
// TODO: swap to custom Vector

class URLHash {
    private:
        cmph_config_t* config;
        cmph_t* hash;
        cmph_io_adapter_t* source;
        unsigned nkeys;
    public:
        URLHash();
        URLHash(const std::string& filename, bool hashed);
        //URLHash(const Vector<std::string>& filenames);
        URLHash(const std::string& filename, unsigned num_pages);
        //URLHash(const Vector<std::string>& filenames, unsigned num_pages);
        //URLHash(const std::string& filename);
        ~URLHash();
        void dump(const std::string& filename);
        // unsigned operator[](const char* key); // Remember this isn't a reference
        // unsigned operator[](std::string key); // Remember this isn't a reference
        const unsigned operator[](const char* key) const;
        const unsigned operator[](std::string key) const;
        void print(const char* filename);
        void print(const std::string& filename);
};

#endif