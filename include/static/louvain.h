#ifndef LOUVAIN_H
#define LOUVAIN_H

#include "static/network.h"

class Louvain {
    private:
        unsigned original_n;
        SparseNetwork network;
        //Deque<unsigned>* final_communities;
        unsigned* final_reverse_communities;
        unsigned* map;
        unsigned new_num_comm;
        void phase1();
        void phase2();
        void merge_communities();
        void reindex_communities();
        bool check_finished();
    public:
        Louvain();
        Louvain(const unsigned n, const unsigned m,
                unsigned** from_to, unsigned* out_degrees);
        Louvain(unsigned const n, const unsigned m,
                unsigned* first, unsigned* second);
        ~Louvain();
        void partition();
};

#endif