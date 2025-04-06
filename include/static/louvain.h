#ifndef LOUVAIN_H
#define LOUVAIN_H

#include "static/network.h"
#include "static/vector.h"
#include <string>

class Louvain {
    private:
        unsigned original_n;
        SparseNetwork network;
        Vector<Vector<unsigned>> final_communities;
        //Vector<unsigned>* final_communities;
        //unsigned* final_reverse_communities;
        Vector<unsigned> final_reverse_communities;
        unsigned* map = nullptr;
        unsigned new_num_comm;
        void phase1();
        void phase2();
        void merge_communities();
        void reindex_communities();
        bool check_finished();
    public:
        Louvain();
        Louvain(const std::string& filename);
        Louvain(const unsigned n, const unsigned m,
                unsigned** from_to, unsigned* out_degrees);
        Louvain(unsigned const n, const unsigned m,
                unsigned* first, unsigned* second);
        ~Louvain();
        void partition();
        const Vector<Vector<unsigned>>& get_communities() const;
        const Vector<unsigned>& get_reverse_communities() const;
        void save_network(const std::string& filename);
        void save_communities(const std::string& filename);
        void save_reverse_communities(const std::string& filename);
};

#endif