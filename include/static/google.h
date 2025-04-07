#ifndef GOOGLE_H
#define GOOGLE_H

#include "static/network.h"
#include "static/algorithm.h"
#include "vector.h"
#include <string>

// TODO: ideally this and sparsematrix would inherit from an abstract
// base class called Matrix or something
class GoogleMatrix {
    private:
        char* delete_map;
        bool delete_from_to = true;
        void read_bitstream(const std::string& filename); // This modifies n, m, num_communities, from_to, and out_degrees
        void read_txt(const std::string& filename); // This modifies n, m, num_communities, from_to, and out_degrees
        void convert_google();
        void iteration(double damping);
        Vector<double> page_ranks;
    public:
        unsigned n;
        unsigned** from_to;
        unsigned* out_degrees;
        unsigned* full_row;

        GoogleMatrix();
        GoogleMatrix(const std::string& filename);
        GoogleMatrix(const unsigned n_in, unsigned** from_to_in, 
                     unsigned* out_degrees_in);
        GoogleMatrix(unsigned const n_in, unsigned const m_in,
                     unsigned* first_in, unsigned* second_in);
        ~GoogleMatrix();
        Vector<double> pagerank(double damping);
        void print(bool adjacency);
        void save(const std::string& filename);
};

#endif