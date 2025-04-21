#!/bin/bash
cd xgboost
yes | conda create --name ranker_xgb
conda activate ranker_xgb
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX
cmake --build build --target install
cmake -B build -S . -DCMAKE_PREFIX_PATH=$CONDA_PREFIX
cmake --build build
cd ..
cd cmph
./configure
rm src/main.c
cd ..
mkdir build
cmake -S . -B build