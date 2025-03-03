cmake_minimum_required(VERSION 3.10)

project(vector-vs-list-insert)

set(CMAKE_CXX_STANDARD 17)

# Set optimization flags based on compiler
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -O2 -march=native")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Enable AVX2 for MSVC (if desired)
    add_compile_options(/arch:AVX2 /O2)
endif()

add_executable(main main.cpp)
