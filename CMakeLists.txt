cmake_minimum_required(VERSION 3.19)
project(LL1Parser)

set(CMAKE_CXX_STANDARD 14)
# LL1Parser2.0/LL1Parser.cpp LL1Parser2.0/LL1Parser.h
add_executable(LL1Parser LL1Parser2.0/utils.cpp LL1Parser2.0/utils.h LL1Parser2.0/LL1Paraser3_0.cpp)