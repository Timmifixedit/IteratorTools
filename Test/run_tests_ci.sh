./build/Test/C++17/Tests_C++17 --gtest_color=yes
if [[ $CXX == g++1* ]]; then
  ./build/Test/C++17/Tests_C++20 --gtest_color=yes
fi
