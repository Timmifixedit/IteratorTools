cmake_minimum_required(VERSION 3.18)
project(IteratorTools)

option(BUILD_ITERATOR_TESTS "Build unit tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -fsanitize=address")

if (${BUILD_ITERATOR_TESTS})
    add_subdirectory(Test)
endif ()
