set(CMAKE_CXX_STANDARD 17)
set(EXEC_NAME ${PROJECT_NAME}_C++17)
add_executable(${EXEC_NAME} main.cpp Iterators.cpp utils.cpp CounterIterator.cpp)
target_include_directories(${EXEC_NAME} PRIVATE ${GTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(${EXEC_NAME} ${LIBS} gmock gtest pthread)
add_test(NAME ${EXEC_NAME} COMMAND ${PROJECT_NAME})