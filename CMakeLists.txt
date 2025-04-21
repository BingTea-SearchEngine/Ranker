cmake_minimum_required(VERSION 3.22.1)

project(Ranker)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB cmph_src cmph/src/*)
add_library(CMPH STATIC ${cmph_src})
target_include_directories(CMPH PUBLIC ${CMAKE_SOURCE_DIR}/cmph/src)

add_library(Hash STATIC src/hash.cpp)
target_link_libraries(Hash PUBLIC CMPH)
target_include_directories(Hash
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/cmph/src
)

add_executable(RankInserter src/insertrank.cpp)
target_link_libraries(RankInserter PUBLIC Hash)
