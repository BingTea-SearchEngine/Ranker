#ifndef TREE_H
#define TREE_H

#include "static/node.h"
#include <iostream>
#include <cstdlib> // TODO: probably implement MT on my own
#include <ctime>

// For reading, temp
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

class Tree {
    private:
        Node* root;
        unsigned int max_depth;
        unsigned int min_samples_split;

        double loss(unsigned int n, double sum, double sum_sq);
        // TODO: add different loss metrics
        // TODO: maybe add partition function to prevent code duplication
        SplitInfo best_split(unsigned int num_rows, unsigned int num_cols, 
                             double** x, double* y);
        SplitInfo best_split(Node* node);
        void split_node(unsigned int depth, Node* parent);
        double predict_row(double* row, Node* node);
        void destroy(Node* node);
        
    public:
        // TODO: see if ranking needs to be fast, and if tree building
        // is something worth optimizing. If so, look into
        // multithreading this. Or multithread XGB, but that may not be
        // possible.
        Tree();
        Tree(unsigned int max_depth_in, unsigned int min_samples_split_in);
        ~Tree();
        void fit(unsigned int num_rows, unsigned int num_cols, double** x, double* y);
        double* predict(unsigned int num_rows, double** x);
};

double* load_1D(std::string filename);

double** load_2D(std::string filename);

void print_1D_arr(unsigned int size, double* arr);
void print_2D_arr(unsigned int num_rows, unsigned int num_cols, double** arr);

#endif