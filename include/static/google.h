#ifndef GOOGLE_H
#define GOOGLE_H

#include "static/network.h"
#include <string>

class GoogleMatrix: public SparseNetwork {
    private:
    public:
        GoogleMatrix();
        GoogleMatrix(const std::string& filename);
        GoogleMatrix(const unsigned n_in, const unsigned m_in,
            unsigned** from_to_in, unsigned* out_degrees_in);
        GoogleMatrix(unsigned const n_in, unsigned const m_in,
            unsigned* first_in, unsigned* second_in);
};

#endif

/*

SparseNetwork();
SparseNetwork(const std::string& filename);
// TODO: maybe add another constructor that passes in a
// community assignment
SparseNetwork(const unsigned n, const unsigned m,
                unsigned** from_to, unsigned* out_degrees);

*/