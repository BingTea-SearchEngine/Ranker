#include <iostream>
#include <chrono>
#include <cassert>
#include <cstdlib>
#include <deque>
#include <vector>
#include <sstream>
#include <fstream>
#include <xgboost/c_api.h>
#include <algorithm>

#define safe_xgboost(call) {  \
    int err = (call); \
    if (err != 0) { \
      throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + \
                          ": error in " + #call + ":" + XGBGetLastError());  \
    } \
}

int main() {
    std::ifstream ifs("../data/train.csv");
    std::vector<float> data; // NEEDS TO BE A FLOAT NOT A DOUBLE
    std::string line;
    unsigned num_rows = 0;
    unsigned num_cols = 0;

    std::getline(ifs, line);
    while (std::getline(ifs, line)) {
        std::istringstream iss(line);
        std::string temp;

        // Ignore first two, they are assumed to be URL and ID
        std::getline(iss, temp, ',');
        std::getline(iss, temp, ',');
        while (std::getline(iss, temp, ',')) {
            data.push_back(stod(temp));
        }

        // TODO: GET RID OF THESE LINEs IN THE FINAL, IT'S ONLY HERE
        // BECAUSE I'M USING THE SAME DATASET TO TEST AS TRAIN, AND IT
        // HAS A QID AND RELEVANCE COLUMN
        data.pop_back();
        data.pop_back();

        if (num_cols == 0)
            num_cols = data.size();

        num_rows++;
    }
    std::cout << num_rows << ' ' << num_cols << std::endl;
    DMatrixHandle dmat;
    // Valgrind says there a memory leak here. IDFK how, I free at the end.
    safe_xgboost(XGDMatrixCreateFromMat(data.data(), num_rows, num_cols, -1, &dmat));

    BoosterHandle booster;
    const char *model_path = "../data/xgb.ubj";

    // create booster handle first
    safe_xgboost(XGBoosterCreate(NULL, 0, &booster));

    // set the model parameters here

    // load model
    // Valgrind says there a memory leak here. IDFK how, I free at the end.
    safe_xgboost(XGBoosterLoadModel(booster, model_path));

    char const config[] =
        "{\"training\": false, \"type\": 0, "
        "\"iteration_begin\": 0, \"iteration_end\": 0, \"strict_shape\": false}";
    /* Shape of output prediction */
    uint64_t const* out_shape;
    /* Dimension of output prediction */
    uint64_t out_dim;
    /* Pointer to a thread local contiguous array, assigned in prediction function. */
    float const* predictions = NULL;
    safe_xgboost(XGBoosterPredictFromDMatrix(booster, dmat, config, &out_shape, &out_dim, &predictions));

    // DO SOMETHING WITH predicitions
    const unsigned N = 10;
    std::vector<unsigned> indices;
    for (unsigned i = 0; i < num_rows; ++i) {
        indices.push_back(i);
    }
    indices.shrink_to_fit();
    auto comp = [predictions](size_t first, size_t second) {
        return predictions[first] > predictions[second];
    };
    std::partial_sort(indices.begin(), indices.begin() + N,
                      indices.end(), comp);
    std::sort(indices.begin(), indices.begin() + N, comp);
    indices.resize(N);
    std::vector<float> topN;
    for (unsigned i = 0; i < N; ++i) {
        topN.push_back(predictions[indices[i]]);
        std::cout << topN.back() << std::endl;
    }

    safe_xgboost(XGDMatrixFree(dmat));
    safe_xgboost(XGBoosterFree(booster));
}