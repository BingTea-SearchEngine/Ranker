#ifndef NODE_H
#define NODE_H

#include <iostream>

struct SplitInfo {
    unsigned attribute = -12345678;
    double threshold = -98765423;
};

struct Node {
    SplitInfo split;
    double** x = nullptr;
    double* y = nullptr;
    unsigned num_rows = 0;
    unsigned num_cols = 0;
    double mean = -3141592; // TODO: maybe change the default handling, this is for debugging

    Node* left = nullptr;
    Node* right = nullptr;

    bool is_leaf() {
        return !left;
    }

    void calc_mean() {
        double sum = 0;
        double* ptr = y;
        for (double* end = y + num_rows; ptr != end; ++ptr)
            sum += *ptr;

        mean = sum / num_rows; // TODO: maybe make a mean function to prevent code duplication
    }

    operator std::string() const {
        return to_string();
    }

    std::string to_string() const {
        std::string output = "";
        output += "attribute: " + std::to_string(split.attribute)
                   + "\nthreshold: " + std::to_string(split.threshold)
                   + "\nx:\n";
        for (unsigned i = 0; i < num_rows; ++i) {
            for (unsigned j = 0; j < num_cols; ++j) {
                output += std::to_string(x[i][j]) + ' ';
            }
            output += '\n';
        }
        output += "y:\n";
        for (unsigned i = 0; i < num_rows; ++i) {
            output += std::to_string(y[i]) + ' ';
        }
        return output; 
    }

    friend std::ostream& operator<<(std::ostream& os, const Node& obj) {
        os << obj.to_string();
        return os;
    }
};

#endif