cmake_minimum_required(VERSION 3.0)

project(monadic_experiments)

set(CMAKE_CXX_COMPILER clang++-6.0)
set(CMAKE_CXX_STANDARD 17)
add_executable(experiments monadic-clutter/monadicclutter.cpp)
