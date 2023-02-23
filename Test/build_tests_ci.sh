make Tests_C++17 -j$(nproc)
if [[ $CXX == g++1* ]]; then
  make Tests_C++20 -j$(nproc)
fi
