#include <iostream>

#include <chrono>

#include "static/tree.h"
#include "static/vector.h"
#include "static/network.h"
#include "static/algorithm.h"
#include "static/deque.h"
#include "static/louvain.h"
#include <cassert>
#include <cstdlib>
#include <deque>
#include <xgboost/c_api.h>
#include "static/google.h"
#include "cmph.h"
#include "static/hash.h"

#define safe_xgboost(call) {  \
    int err = (call); \
    if (err != 0) { \
      throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + \
                          ": error in " + #call + ":" + XGBGetLastError());  \
    } \
}

int main() {
    BoosterHandle booster;
    const char *model_path = "../src/xgb_model.bin";

    // create booster handle first
    safe_xgboost(XGBoosterCreate(NULL, 0, &booster));

    // set the model parameters here

    // load model
    safe_xgboost(XGBoosterLoadModel(booster, model_path));
    XGBoosterFree(booster);
}