#include "static/tree.h"

void quicksort_indices(unsigned int* indices, double** x, 
                       unsigned int col_idx, int left, int right) {
    if (left >= right)
        return;

    double pivot = x[indices[(left + right) / 2]][col_idx];
    int i = left;
    int j = right;
    while (i <= j) {
        while (x[indices[i]][col_idx] < pivot)
            i++;
        while (x[indices[j]][col_idx] > pivot)
            j--;
        if (i <= j) {
            unsigned int temp = indices[i];
            indices[i] = indices[j];
            indices[j] = temp;
            i++;
            j--;
        }
    }
    if (left < j)
        quicksort_indices(indices, x, col_idx, left, j);
    if (i < right)
        quicksort_indices(indices, x, col_idx, i, right);
}

Tree::Tree() : root(nullptr), max_depth(-1), min_samples_split(2) {}

Tree::Tree(unsigned int max_depth_in, unsigned int min_samples_split_in)
  : root(nullptr), max_depth(max_depth_in), 
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

double Tree::loss(unsigned int n, double sum, double sum_sq) {
    if (n == 0)
        return 0;

    // Return SSE
    return sum_sq - (sum * sum) / n;
}

SplitInfo Tree::best_split(unsigned int num_rows, unsigned int num_cols, 
                           double** x, double* y) {
    double lowest_loss = 999999999; // TODO: figure out if I can use <limits>
    unsigned int count = 0;
    SplitInfo split;

    // Loop through all possible splits
    unsigned int* indices = new unsigned int[num_rows];
    for (unsigned int col_idx = 0; col_idx < num_cols; ++col_idx) {
        // Find indices that sort the attribute col
        for (unsigned int i = 0; i < num_rows; ++i)
            indices[i] = i;

        quicksort_indices(indices, x, col_idx, 0, num_rows - 1);

        // Create list of cumulative sums and sqaured sums for SSE
        // optimization
        double cum_sum = 0;
        double cum_sum_sq = 0;
        double sums[num_rows];
        double sum_sqs[num_rows];

        
        double** row_ptr = x;
        for (unsigned int i = 0; i < num_rows - 1; ++i) {
            double val = y[indices[i]];
            cum_sum += val;
            cum_sum_sq += val * val;

            sums[i] = cum_sum;
            sum_sqs[i] = cum_sum_sq;
        }

        // Using cumulative sums, iterate through each possible split
        // calculating SSE
        for (unsigned int i = 0; i < num_rows - 1; ++i) {
            double current = x[indices[i]][col_idx];
            double next = x[indices[i + 1]][col_idx];

            if (current == next)
                continue;

            // Calculate SSE
            unsigned int left_n = i + 1;
            double left_sum = sums[i];
            double left_sum_sq = sum_sqs[i];
            double left_loss = loss(left_n, left_sum, left_sum_sq);

            unsigned int right_n = num_rows - i - 1;
            double right_sum = cum_sum - left_sum;
            double right_sum_sq = cum_sum_sq - left_sum_sq;
            double right_loss = loss(right_n, right_sum, right_sum_sq);

            double total_loss = left_loss + right_loss;
            
            // Save the best split
            if (total_loss < lowest_loss) {
                lowest_loss = total_loss;
                split.attribute = col_idx;
                split.threshold = (current + next) / 2;
                count = 1;
            }
            // Break ties randomly
            else if (total_loss == lowest_loss) {
                count++;
                if (rand() % count == 0) {
                    lowest_loss = total_loss;
                    split.attribute = col_idx;
                    split.threshold = (current + next) / 2;
                }
            }
        }
    }
    delete[] indices;

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

void Tree::fit(unsigned int num_rows, unsigned int num_cols, 
               double** x, double* y) {
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

double* load_1D(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << filename << std::endl;
    }

    std::vector<double> data;
    std::string val;
    while (std::getline(file, val, ',')) {
        data.push_back(std::stod(val));
    }
        
    file.close();
    
    unsigned int size = data.size();

    double* array = new double[size];
    for (unsigned int i = 0; i < size; ++i)
        array[i] = data[i];

    return array;
}

double** load_2D(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << filename << std::endl;
    }

    std::vector<std::vector<double>> data;
    std::string line, val;
    while (std::getline(file, line)) {
        std::vector<double> vect;
        std::stringstream s(line);
        while (std::getline(s, val, ','))
            vect.push_back(std::stod(val));
        data.push_back(vect);
    }
        
    file.close();
    
    unsigned int num_rows = data.size();
    unsigned int num_cols = data[0].size();

    double** array = new double*[num_rows];
    for (unsigned int i = 0; i < num_rows; ++i) {
        array[i] = new double[num_cols];
        for (unsigned int j = 0; j < num_cols; ++j) 
            array[i][j] = data[i][j];
    }

    return array;
}

void print_1D_arr(unsigned int size, double* arr) {
    double* ptr = arr;
    for (double* end = arr + size; ptr != end; ++ptr)
        std::cout << *ptr << ' ';
    std::cout << std::endl;
}

void print_2D_arr(unsigned int num_rows, unsigned int num_cols, double** arr) {
    double** ptr = arr;
    for (double** end = arr + num_rows; ptr != end; ++ptr) {
        for (unsigned int i = 0; i < num_cols; ++i) {
            std::cout << (*ptr)[i] << ' ';
        }
        std::cout << std::endl;
    }
}

unsigned int Tree::num_leaves(Node* node) {
    if (node->is_leaf())
        return 1;

    return 1 + num_leaves(node->left) + num_leaves(node->right);
}

unsigned int Tree::num_leaves() {
    return num_leaves(root);
}

unsigned int Tree::height(Node* node) {
    if (node->is_leaf())
        return 1;

    unsigned int left_height = height(node->left);
    unsigned int right_height = height(node->right);

    if (left_height > right_height)
        return 1 + left_height;

    return 1 + right_height;
}

unsigned int Tree::height() {
    return height(root);
}