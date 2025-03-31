cmake_minimum_required(VERSION 3.22.1)

project(Ranker)

add_library(Ranker STATIC src/louvain.cpp src/network.cpp src/node.cpp 
src/pagerank.cpp src/tree.cpp src/xgboost.cpp include/static/vector.h)

target_include_directories(Ranker PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(Runner src/main.cpp)

target_link_libraries(Runner PUBLIC Ranker)