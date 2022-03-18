#!/bin/sh
CC=gcc CXX=g++ cmake -B build/gcc -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build/gcc -- --verbose
CC=clang CXX=clang++ cmake -B build/clang -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build/clang -- --verbose
