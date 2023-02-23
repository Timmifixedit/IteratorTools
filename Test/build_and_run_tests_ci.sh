echo The current compiler is "$CXX"
make Tests_C++17 -j"$(nproc)"
./Test/C++17/Tests_C++17 --gtest_color=yes
if [[ $CXX == g++-1* ]]; then
  echo running c++20 tests
  make Tests_C++20 -j"$(nproc)"
  ./Test/C++17/Tests_C++20 --gtest_color=yes
fi
