#include <gtest/gtest.h>
#include <vector>
#include <list>
#include <string>
#include "Iterators.hpp"
#include "MustNotCopy.hpp"

TEST(Iterators, zip_elements) {
    using namespace iterators;
    std::list<std::string> strings{"a", "b", "c"};
    std::vector<int> numbers{1, 2, 3};
    auto sIt = strings.begin();
    auto nIt = numbers.begin();
    for (auto [string, number] : const_zip(strings, numbers)) {
        EXPECT_EQ(string, *sIt);
        EXPECT_EQ(number, *nIt);
        ++sIt; ++nIt;
    }

    EXPECT_EQ(sIt, strings.end());
    EXPECT_EQ(nIt, numbers.end());
}

TEST(Iterators, zip_c_arrays) {
    using namespace iterators;
    int numbers[] = {1, 2, 3};
    const char * strings[] = {"a", "b", "c"};
    auto sIt = std::begin(strings);
    auto nIt = std::begin(numbers);
    for (auto [string, number] : const_zip(strings, numbers)) {
        EXPECT_EQ(string, *sIt);
        EXPECT_EQ(number, *nIt);
        ++sIt; ++nIt;
    }

    EXPECT_EQ(sIt, std::end(strings));
    EXPECT_EQ(nIt, std::end(numbers));
}

TEST(Iterators, zip_inequal_length) {
    using namespace iterators;
    std::list<std::string> strings{"a", "b", "c"};
    std::vector<int> numbers{1, 2, 3, 4, 5};
    auto sIt = std::begin(strings);
    auto nIt = std::begin(numbers);
    for (auto [string, number] : const_zip(strings, numbers)) {
        EXPECT_EQ(string, *sIt);
        EXPECT_EQ(number, *nIt);
        ++sIt; ++nIt;
    }

    EXPECT_EQ(sIt, std::end(strings));
    EXPECT_EQ(*nIt, 4);
}

TEST(Iterators, zip_mutuate) {
    using namespace iterators;
    std::list<std::string> strings{"a", "b", "c"};
    std::vector<int> numbers{1, 2, 3};
    for (auto [string, number] : zip(strings, numbers)) {
        string += std::to_string(number);
        number *= 2;
    }

    std::vector<int> numbersResults{2, 4, 6};
    std::list<std::string> stringsResults{"a1", "b2", "c3"};
    EXPECT_TRUE(std::equal(numbers.begin(), numbers.end(), numbersResults.begin()));
    EXPECT_TRUE(std::equal(strings.begin(), strings.end(), stringsResults.begin()));
}

TEST(Iterators, enumerate_elements) {
    using namespace iterators;
    std::list<std::string> strings{"a", "b", "c"};
    std::size_t i = 0;
    auto sIt = strings.begin();
    for (auto [index, string] : const_enumerate(strings)) {
        EXPECT_EQ(string, *sIt);
        EXPECT_EQ(index, i++);
        ++sIt;
    }
}

TEST(Iterators, no_copy) {
    using namespace iterators;
    std::vector<MustNotCopy> items;
    items.emplace_back("a");
    items.emplace_back("b");
    items.emplace_back("c");
    for (auto [index, item] : enumerate(items)) {
        item.s = "";
    }
}