cmake_minimum_required(VERSION 3.0)

project(monadic_experiments)

set(CMAKE_CXX_COMPILER clang-5.0)
set(CMAKE_CXX_STANDARD 17)
add_executable(experiments monadic-clutter/monadicclutter.cpp)
target_link_libraries(experiments stdc++)
add_executable(variant monadic-clutter/variantstuff.cpp)
