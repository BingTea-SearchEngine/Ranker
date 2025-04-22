#ifndef LOUVAIN_H
#define LOUVAIN_H

#include "static/network.h"
#include "static/vector.h"
#include "static/algorithm.h"
#include <string>
#include <thread>

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
        void phase1(double resolution);
        void phase2();
        void merge_communities();
        void reindex_communities();
        bool check_finished();
    public:
        Louvain();
        Louvain(const std::string& filename);
        Louvain(const std::string& from_to_filename,
                const std::string& hash_filename);
        Louvain(const unsigned n, const unsigned m,
                unsigned** from_to, unsigned* out_degrees);
        Louvain(unsigned const n, const unsigned m,
                unsigned* first, unsigned* second);
        ~Louvain();
        void partition(double resolution);
        void set_communities(unsigned* reverse_communities);
        void set_communities(const std::string& filename);
        const Vector<Vector<unsigned>>& get_communities() const;
        const Vector<unsigned>& get_reverse_communities() const;
        void dump_from_to(const std::string& filename);
        void dump_to_from(const std::string& filename);
        void dump_communities(const Vector<std::string>& filenames); // THESE REQUIRE LOUVAIN VARIABLES, SO NO SET_COMMUNITIES()!!!
        void dump_reverse_communities(const std::string& filename); // THESE REQUIRE LOUVAIN VARIABLES, SO NO SET_COMMUNITIES()!!!
        void dump_partitions(const Vector<std::string>& filenames);
        double modularity();
        unsigned num_communities();
};

#endif