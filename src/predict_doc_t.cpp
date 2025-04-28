#include <iostream>
#include <chrono>
#include <cassert>
#include <cstdlib>
#include <deque>
#include <vector>
#include <sstream>
#include <fstream>
#include <xgboost/c_api.h>

#define safe_xgboost(call) {  \
    int err = (call); \
    if (err != 0) { \
      throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + \
                          ": error in " + #call + ":" + XGBGetLastError());  \
    } \
}

struct doc_t {
    std::string url;
    uint32_t numWords;
    uint32_t numTitleWords;
    uint32_t numOutLinks;
    uint32_t numTitleMatch;
    uint32_t numBodyMatch;
    float pageRank;
    float cheiRank;
    float rankingScore;
    std::string snippet;
    std::string title;
};

int main() {
    std::vector<doc_t> doc_ts;
    std::vector<float> data;
    unsigned num_rows = 0;
    unsigned num_cols = 8;

    for (const doc_t& page: doc_ts) {
        data.push_back(page.numWords);
        data.push_back(page.numTitleWords);
        data.push_back(page.numOutLinks);
        data.push_back(page.numTitleMatch);
        data.push_back(page.numBodyMatch);
        data.push_back(page.pageRank);
        data.push_back(page.cheiRank);
        data.push_back(page.rankingScore);
        num_rows++;
    }

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
    std::vector<unsigned> indices;
    for (unsigned i = 0; i < num_rows; ++i) {
        indices.push_back(i);
    }
    indices.shrink_to_fit();
    auto comp = [predictions](size_t first, size_t second) {
        return predictions[first] > predictions[second];
    };
    std::partial_sort(indices.begin(), indices.begin() + 10,
                      indices.end(), comp);
    std::sort(indices.begin(), indices.begin() + 10, comp);
    indices.resize(10);
    std::vector<doc_t> top10;
    for (unsigned i = 0; i < 10; ++i) {
        top10.push_back(doc_ts[indices[i]]);
    }

    safe_xgboost(XGDMatrixFree(dmat));
    safe_xgboost(XGBoosterFree(booster));
}