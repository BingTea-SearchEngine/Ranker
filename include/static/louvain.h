#ifndef LOUVAIN_H
#define LOUVAIN_H

#include "static/network.h"

class Louvain {
    private:
        SparseNetwork network;
        unsigned* communities;
        void merge_communities();
        void reindex_communities(unsigned* map);
    public:
        Louvain();
        Louvain(const unsigned n, const unsigned m,
                unsigned** from_to, unsigned* out_degrees);
        Louvain(unsigned const n, const unsigned m,
                unsigned* first, unsigned* second);
        ~Louvain();
        void phase1();
        void phase2();
        
};

#endif