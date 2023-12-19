cmake_minimum_required(VERSION 3.10)
project(IteratorTools)

option(BUILD_TESTS "Build unit tests" OFF)
option(DISABLE_RANGES_COMPAT "Disable c++20 ranges compatibility to prevent problems with clang versions < 16" OFF)

if (${DISABLE_RANGES_COMPAT})
    add_compile_definitions(__STD_RANGES_DISABLED__=1)
endif ()

add_subdirectory(Test)
