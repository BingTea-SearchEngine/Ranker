#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#include "static/tree.h"

using namespace std;

std::vector<vector<double>> asdf(string filename) {
    //string filename;
    //cout << "Enter the filename: ";
    //cin >> filename;

    ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
    }

    vector<vector<double>> data;
    string line;

    while (getline(inputFile, line)) {
        vector<double> row;
        stringstream ss(line);
        double value;

        while (ss >> value) {
            row.push_back(value);
            if (ss.peek() == ',')
                ss.ignore();
        }
        data.push_back(row);
    }

    inputFile.close();

    cout << "Data read from file:" << endl;
    for (const auto& row : data) {
        std::cout << row.size() << std::endl;
        for (double value : row) {
            cout << value << " ";
        }
        cout << endl;
    }
    return data;
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    auto train_x = asdf("train_x.txt");
    auto train_y = asdf("train_y.txt");
    auto test_x = asdf("test_x.txt");
    auto test_y = asdf("test_y.txt");

    double** x1 = new double*[14];
    double* y1 = new double[14];
    double** x2 = new double*[14];
    double* y2 = new double[14];

    for (int i = 0; i < 14; ++i) {
        x1[i] = new double[6];
        x2[i] = new double[6];
        for (int j = 0; j < 6; ++j) {
            x1[i][j] = train_x[i][j];
            x2[i][j] = test_x[i][j];
        }
        y1[i] = train_y[0][i];
        y2[i] = test_y[0][i];
    }

    Tree tree;
    tree.fit(14, 6, x1, y1);
    auto qwer = tree.predict(14, x2);
    for (int i = 0; i < 14; ++i) {
        std::cout << qwer[i] << std::endl;
    }
    
    return 0;
}