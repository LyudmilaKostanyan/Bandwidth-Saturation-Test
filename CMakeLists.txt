cmake_minimum_required(VERSION 3.10)

project(vector-vs-list-insert)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -O2 -march=native")

add_executable(main main.cpp)
