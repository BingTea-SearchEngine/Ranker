//#include "static/node.h"
#include "static/tree.h"

int main() {
    Tree tree;
    double base_x[5][2] = {{1, 5}, {2, 4}, {3, 3}, {4, 2}, {5, 1}};
    double base_y[5] = {2, 3, 3.5, 5, 4.5};

    double** x = new double*[5];
    double* y = new double[5];

    for (int i = 0; i < 5; ++i) {
        x[i] = new double[2];
    }

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 2; ++j) {
            x[i][j] = base_x[i][j];
        }
        y[i] = base_y[i];
    }
    tree.fit(5, 2, x, y);

    double** predict = new double*[1];
    predict[0] = new double[2];
    predict[0][0] = 3.1;
    predict[0][1] = 3.1;
    double* prediction = tree.predict(1, predict); 
    std::cout << *prediction << std::endl;

    for (int i = 0; i < 5; ++i) {
        delete[] x[i];
    }
    delete[] x;
    delete y;

    delete[] predict[0];
    delete[] predict;

    delete[] prediction;

    return 0;
}