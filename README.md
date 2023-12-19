[![unit tests](https://github.com/Timmifixedit/IteratorTools/actions/workflows/unit_tests.yml/badge.svg)](https://github.com/Timmifixedit/IteratorTools/actions/workflows/unit_tests.yml)
# Python-like Zip and Enumerate Iterators
C++-implementation of Python-like zip- and enumerate-iterators which can be used in range-based
for loops along with structured bindings to iterate over multiple containers at the same
time. Requires C++17.

The library has been tested on the following compilers:
* g++:
  * C++17 support: g++-9 to g++-12
  * C++20 ranges compatibility: g++-10 to g++-12
* clang:
  * C++17 support: clang-9 to clang-17
  * C++20 ranges compatibility: clang-16 and clang-17

## Properties
The `zip`-class is a container-wrapper for arbitrary iterable containers. It provides the
member functions `begin()` and `end()` enabling it to be used in range-based for loops to
iterate over multiple containers at the same time. The `enuerate`-function is a special
case of `zip` and uses a "counting container" (similar to `std::ranges::iota`) to provide
an index. Additionally, const-versions exist which do not allow the manipulation of the
container elements.

## Code Examples
The syntax is mostly similar to Python:
```c++
#include <vector>
#include <list>
#include "Iterators.hpp"

using namespace iterators;
std::list<std::string> strings{"a", "b", "c"};
std::vector<int> numbers{1, 2, 3};
for (auto [string, number] : zip(strings, numbers)) {
    // 'string' and 'number' are references to the container element
    string += std::to_string(number); 
}

// now 'strings' contains {"a1", "b2", "c3"}
```
The for loop uses so called `ZipIterator`s which point to tuples which in turn contain
references to the container elements. Therefore, no copying occurs and manipulation of the
container elements is possible. Observe that the structured binding captures by value
(since the values are themselves references).

If you want to prohibit manipulation, you can use `const_zip`
```c++
using namespace iterators;
std::list<std::string> strings{"a", "b", "c"};
std::vector<int> numbers{1, 2, 3};
for (auto [string, number] : const_zip(strings, numbers)) {
    // string += std::to_string(number);  error, string is readonly!
    std::cout << string << " " << number << std::endl;
}
```
Additionally, you can use `zip_i` to manually zip iterators or pointers:
```c++
using namespace iterators;
std::list<std::string> strings{"a", "b", "c"};
std::vector<int> numbers{1, 2, 3};
auto zipBegin = zip_i(strings.begin(), numbers.begin());
auto zipEnd = zip_i(strings.end(), numbers.end());
while (zipBegin != zipEnd) {
    auto [s, num] = *zipBegin;
    // ...
    ++zipBegin;
}
```
`ZipIterators` support the same operations as the least powerful underlying iterator.
For example, if you zip a random access iterator (e.g. from `std::vector`) and a bidirectional
iterator (e.g. from `std::list`), then the resulting `ZipIterator` will only support
bidirectional iteration but no random access.

As in Python, the shortest range decides the overall range:
```c++
using namespace iterators;
std::list<std::string> strings{"a", "b", "c"};
std::vector<int> numbers{1, 2, 3, 4, 5, 6};
for (auto [string, number] : zip(strings, numbers)) {
    std::cout << string << " " << number << " | "
}
// prints a 1 | b 2 | c 3 |
```

The `enumerate`-function works similarly.
```c++
using namespace iterators;
std::list<std::string> strings{"a", "b", "c"};
for (auto [index, string] : enumerate(strings)) {
    string += std::to_string(index); 
}
// now 'strings' contains {"a0", "b1", "c2"}
```
Also, an optional offset can be specified:
```c++
for (auto [index, string] : enumerate(strings, 4)) { // index starts from 4
    ...
}
```
And as with `zip`, a const version (`const_enumerate`) exists.

In case temporary containers are used, `zip` and `enumerate` will take ownership of the
containers to guarantee well-defined memory access.
```c++
for (auto [index, number] : enumerate(std::array{53, 21, 17})) {
    // enumerate takes ownership of the array. The elements
    // can safely be accessed and manipulated
}
```

## Doxygen Documentation
* [HTML](https://timmifixedit.github.io/IteratorTools/html/index.html)
* [PDF](https://timmifixedit.github.io/IteratorTools/ZipEnumerateCppDocs.pdf)

