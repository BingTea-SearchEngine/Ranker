#ifndef HASH_H
#define HASH_H

#include "cmph.h"
#include "static/vector.h"
#include <string>
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>

class URLHash {
    private:
        cmph_config_t* config;
        cmph_t* hash;
        cmph_io_adapter_t* source;
        unsigned nkeys;
    public:
        URLHash();
        URLHash(const Vector<std::string>& filenames);
        URLHash(const Vector<std::string>& filenames, unsigned num_pages);
        URLHash(const std::string& filename);
        ~URLHash();
        void dump(const std::string& filename);
        void test(const Vector<std::string>& filenames, const std::string& filename1);
};

#endif