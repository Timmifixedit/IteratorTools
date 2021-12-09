#include <gtest/gtest.h>
#include <vector>
#include <list>
#include <string>
#include <type_traits>
#include "Iterators.hpp"
#include "utils.hpp"

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

TEST(Iterators, zip_constness) {
    using namespace iterators;
    std::list<std::string> strings;
    std::vector<int> numbers;
    const auto &constStrings = strings;
    auto zipView = zip(strings, numbers);
    const auto &constZipView = zipView;
    EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(std::get<0>(*const_zip(strings, numbers).begin()))>>);
    EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(std::get<1>(*const_zip(strings, numbers).begin()))>>);
    EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(std::get<0>(*zip(constStrings, numbers).begin()))>>);
    EXPECT_FALSE(std::is_const_v<std::remove_reference_t<decltype(std::get<1>(*zip(constStrings, numbers).begin()))>>);
    EXPECT_TRUE(
            std::is_const_v<std::remove_reference_t<decltype(std::get<0>(*zip(strings.cbegin(), numbers.begin())))>>);
    EXPECT_FALSE(
            std::is_const_v<std::remove_reference_t<decltype(std::get<1>(*zip(strings.cbegin(), numbers.begin())))>>);
    EXPECT_FALSE(std::is_const_v<std::remove_reference_t<decltype(std::get<0>(*zipView.begin()))>>);
    EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(std::get<0>(*constZipView.begin()))>>);
}

TEST(Iterators, zip_iterator_types) {
    using namespace iterators;
    std::list<int> forward {1, 2, 3};
    std::vector<int> backward{3, 2, 1};
    auto zipView = const_zip(forward, backward);
    constexpr bool res = std::is_same_v<decltype(zipView.begin()), decltype(zipView.end())>;
    EXPECT_TRUE(res);
    constexpr bool res1 = std::is_same_v<decltype(zip(forward.begin(), backward.begin())), decltype(zip(forward.end(), backward.end()))>;
    EXPECT_TRUE(res1);
}

TEST(Iterators, zip_iterator_manual) {
    using namespace iterators;
    std::list<int> forward {1, 2, 3};
    std::vector<int> backward{3, 2, 1};
    auto start = zip(forward.cbegin(), backward.cbegin());
    auto end = zip(forward.cend(), backward.cend());
    std::size_t counter = 0;
    while (start != end) {
        EXPECT_EQ(std::get<0>(*start) + std::get<1>(*start), 4);
        ++counter;
        ++start;
    }

    EXPECT_EQ(counter, 3);
}

TEST(Iterators, elements_no_copy) {
    using namespace iterators;
    std::vector<MustNotCopy> items;
    items.emplace_back("a");
    items.emplace_back("b");
    items.emplace_back("c");
    for (auto [item] : zip(items)) {
        item.s = "";
    }

    for (auto [item] : zip(std::move(items))) {
        item.s = "";
    }
}

TEST(Iterators, container_no_copy) {
    using namespace iterators;
    MustNotCopyContainer<std::string> strings{"a", "b", "c"};
    MustNotCopyContainer<int> numbers{1, 2, 3};
    for (auto [string, number] : zip(strings, numbers)) {
        number *= 2;
        string += std::to_string(number);
    }
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

TEST(Iterators, enumerate_offset) {
    using namespace iterators;
    std::list<std::string> strings{"a", "b", "c"};
    std::size_t i = 4;
    auto sIt = strings.begin();
    for (auto [index, string] : const_enumerate(strings, i)) {
        EXPECT_EQ(string, *sIt);
        EXPECT_EQ(index, i++);
        ++sIt;
    }
}

TEST(Iterators, enumerate_negative_offset) {
    using namespace iterators;
    std::list<std::string> strings{"a", "b", "c"};
    int i = -3;
    auto sIt = strings.begin();

    for (auto [index, string] : const_enumerate(strings, i)) {
        EXPECT_EQ(string, *sIt);
        EXPECT_EQ(index, i++);
        ++sIt;
    }
}

TEST(Iterators, enumerate_increment) {
    using namespace iterators;
    std::list<std::string> strings{"a", "b", "c"};
    int i = 4;
    auto sIt = strings.begin();
    for (auto [index, string] : const_enumerate(strings, 4, -2)) {
        EXPECT_EQ(string, *sIt);
        EXPECT_EQ(index, i);
        i -= 2;
        ++sIt;
    }
}

TEST(Iterators, enumerate_mutuate) {
    using namespace iterators;
    std::list<std::string> strings{"a", "b", "c"};
    for (auto [index, string] : enumerate(strings)) {
        string += std::to_string(index);
        index += 2; // should have no effect
    }

    std::list<std::string> stringsResults{"a0", "b1", "c2"};
    EXPECT_TRUE(std::equal(strings.begin(), strings.end(), stringsResults.begin()));
}

TEST(Iterators, enumerate_constness) {
    using namespace iterators;
    std::list<std::string> strings;
    const auto &constStrings = strings;
    EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(std::get<1>(*const_enumerate(strings).begin()))>>);
    EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(std::get<1>(*enumerate(constStrings).begin()))>>);
}

TEST(Iterators, enumerate_elements_no_copy) {
    using namespace iterators;
    std::vector<MustNotCopy> items;
    items.emplace_back("a");
    items.emplace_back("b");
    items.emplace_back("c");
    for (auto [index, item] : enumerate(items)) {
        item.s += std::to_string(index);
    }

    for (auto [index, item] : enumerate(std::move(items))) {
        item.s += std::to_string(index);
    }
}

TEST(Iterators, enumerate_container_no_copy) {
    using namespace iterators;
    std::list<std::string> strings{"a", "b", "c"};
    for (auto [index, string] : enumerate(strings)) {
        string += std::to_string(index);
    }
}

TEST(Iterators, temporary_container) {
    using namespace iterators;
    std::array expected_values{4, 5, 6};
    for (auto [expected, actual] : zip(expected_values, std::array{4, 5, 6})) {
        EXPECT_EQ(expected, actual);
    }
}

TEST(Iterators, temporary_no_copy) {
    using namespace iterators;
    std::array expected_values{1, 2, 3};
    for (auto [expected, actual] : zip(expected_values, MustNotCopyContainer<int>({1, 2, 3}))) {
        EXPECT_EQ(expected, actual);
    }
}

TEST(Iterators, temporary_lifetime) {
    using namespace iterators;
    std::array expected_values{1, 2, 3};
    bool allowToDie = false;
    for (auto [expected, actual] : zip(expected_values, LifeTimeChecker<int>({1, 2, 3}, allowToDie))) {
        EXPECT_EQ(expected, actual);
        if (actual == 3) { // after the last iteration, the temporary container may be destroyed
            allowToDie = true;
        }
    }
}

TEST(Iterators, bool_vector) {
    using namespace iterators;
    std::vector<bool> booleans{true, true, false, false};
    for (auto [_, boolean] : enumerate(booleans)) {
        boolean = !boolean;
    }

    std::vector<bool> expected{false, false, true, true};
    EXPECT_TRUE(std::equal(booleans.begin(), booleans.end(), expected.begin()));
}

TEST(Iterators, type_traits) {
    using namespace iterators;
    std::array<int, 1> array;
    std::list<int> list;
    auto zipIt = zip(array.begin(), list.begin());
    constexpr bool correctCategory = std::is_same_v<decltype(zipIt)::iterator_category, std::bidirectional_iterator_tag>;
    EXPECT_TRUE(correctCategory);
}