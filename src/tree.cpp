#include "static/tree.h"

Tree::Tree()
: root(nullptr), max_depth(-1), min_samples_split(2) {}

Tree::Tree(unsigned int max_depth_in, unsigned int min_samples_split_in)
: root(nullptr), 
  max_depth(max_depth_in), 
  min_samples_split(min_samples_split_in) {}

void Tree::destroy(Node* node) {
    if (node) {
        destroy(node->left);
        destroy(node->right);
        delete node;
    }
}

Tree::~Tree() {
    destroy(root);
}

double Tree::loss(unsigned int num_rows, double* y) {
    if (num_rows == 0) {
        return 0;
    }

    // Find mean
    double* ptr = y;
    double sum = 0;
    for (double* end = y + num_rows; ptr != end; ++ptr)
        sum += *ptr;

    double mean = sum / num_rows;

    // Find MSE
    double se = 0;
    ptr = y;
    for (double* end = y + num_rows; ptr != end; ++ptr) {
        double diff = mean - *ptr;
        se += diff * diff;
    }

    return se; // Don't divide, since we use a weighted sum afterward anyways
}

SplitInfo Tree::best_split(unsigned int num_rows, unsigned int num_cols, 
                           double** x, double* y) {
    double lowest_mse = 999999999; // TODO: figure out if I can use <limits>
    SplitInfo split;

    // Loop through all possible splits
    double** row_ptr = x;
    for (double** row_end = x + num_rows; row_ptr != row_end; ++row_ptr) {
        for (unsigned int col_idx = 0; col_idx < num_cols; ++col_idx) {
            // Find threshold for split
            double threshold = (*row_ptr)[col_idx]; // TODO: use midpoints instead of boundaries (requires sorting)
            
            // Partition based on threshold
            double above_y[num_rows];
            double below_y[num_rows];
            unsigned int above_idx = 0;
            unsigned int below_idx = 0;
            
            for (unsigned int row_idx = 0; row_idx < num_rows; ++row_idx) {
                if (x[row_idx][col_idx] < threshold) {
                    below_y[below_idx] = y[row_idx];
                    below_idx++;
                }
                else {
                    above_y[above_idx] = y[row_idx];
                    above_idx++;
                }
            }

            // Calculate weighted MSE of the partition
            double total_mse = (loss(below_idx, below_y)
                               + loss(above_idx, above_y)) / num_rows;
            
            // Save the best split
            if (total_mse < lowest_mse) {
                lowest_mse = total_mse;
                split.attribute = col_idx;
                split.threshold = threshold;
            }
        }
    }

    return split;
}

SplitInfo Tree::best_split(Node* node) {
    return best_split(node->num_rows, node->num_cols, node->x, node->y);
}

void Tree::split_node(unsigned int depth, Node* parent) {
    // Check for base cases
    if (depth == max_depth) {
        parent->calc_mean();
        return;
    }
    if (parent->num_rows < min_samples_split) {
        parent->calc_mean();
        return;
    }

    // Find the best split
    parent->split = best_split(parent);
    unsigned int attribute = parent->split.attribute;
    double threshold = parent->split.threshold;

    // Set references to make this section exactly the same as in
    // best_split. Yes, it's code duplication but idk if it's worth
    // making a partition function that uses 4 more dynamic arrays.
    unsigned int& num_rows = parent->num_rows;
    unsigned int& num_cols = parent->num_cols;
    double**& x = parent->x;
    double*& y = parent->y;

    // Partition based on threshold
    double* above_x[num_rows];
    double above_y[num_rows];
    double* below_x[num_rows];
    double below_y[num_rows];
    unsigned int above_idx = 0;
    unsigned int below_idx = 0;
    
    for (unsigned int row_idx = 0; row_idx < num_rows; ++row_idx) {
        // If below threshold, go left
        if (x[row_idx][attribute] < threshold) {
            below_x[below_idx] = x[row_idx];
            below_y[below_idx] = y[row_idx];
            below_idx++;
        }
        // If above threshold, go right
        else {
            above_x[above_idx] = x[row_idx];
            above_y[above_idx] = y[row_idx];
            above_idx++;
        }
    }

    // Set up children
    parent->left = new Node;
    parent->left->x = below_x;
    parent->left->y = below_y;
    parent->left->num_rows = below_idx;
    parent->left->num_cols = parent->num_cols;

    parent->right = new Node;
    parent->right->x = above_x;
    parent->right->y = above_y;
    parent->right->num_rows = above_idx;
    parent->right->num_cols = parent->num_cols;

    
    // Recursive step
    split_node(depth + 1, parent->left);
    split_node(depth + 1, parent->right);
}

void Tree::fit(unsigned int num_rows, unsigned int num_cols, double** x, double* y) {
    // Set up root
    root = new Node;
    root->x = x;
    root->y = y;
    root->num_rows = num_rows;
    root->num_cols = num_cols;
    
    // Start recursion
    split_node(0, root);
}

double Tree::predict_row(double* row, Node* node) {
    // Check for base case
    if (node->is_leaf()) {
        return node->mean;
    }

    // Compare split attribute to go left or right
    if (row[node->split.attribute] < node->split.threshold) {
        return predict_row(row, node->left);
    }
    return predict_row(row, node->right);
}

double* Tree::predict(unsigned int num_rows, double** x) {
    double* prediction = new double[num_rows];
    
    // Predict each row individually
    for (unsigned int i = 0; i < num_rows; ++i) {
        prediction[i] = predict_row(x[i], root);
    }
    
    return prediction;
}