#ifndef LOUVAIN_H
#define LOUVAIN_H

#include "static/network.h"

class Louvain {
    private:
        SparseNetwork network;
        void merge_communities();
        void reindex_communities();
    public:
        Louvain();
        Louvain(unsigned const n_in, const unsigned m_in,
                unsigned* first_in, unsigned* second_in);
        ~Louvain();
        void phase1();
        void phase2();
        
};

#endif