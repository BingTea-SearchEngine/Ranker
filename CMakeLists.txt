cmake_minimum_required(VERSION 3.22.1)

project(Ranker)

add_library(Ranker STATIC src/louvain.cpp src/network.cpp src/node.cpp 
src/pagerank.cpp src/tree.cpp src/xgboost.cpp src/google.cpp src/hash.cpp)

file(GLOB cmph_src cmph/src/*)
add_library(CMPH STATIC ${cmph_src})

target_include_directories(Ranker PUBLIC ${PROJECT_SOURCE_DIR}/include)

find_package(xgboost REQUIRED)
add_executable(Runner src/main.cpp)

target_link_libraries(Runner PUBLIC Ranker xgboost::xgboost CMPH)