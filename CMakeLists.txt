cmake_minimum_required(VERSION 3.0)

project(monadic_experiments)

set(CMAKE_CXX_COMPILER g++-7)
set(CMAKE_CXX_STANDARD 17)
add_executable(experiments monadic-clutter/monadicclutter.cpp)
