#include <gtest/gtest.h>
#include <array>
#include <vector>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <map>
#include <list>
#include "Iterators.hpp"
#include "utils.hpp"

TEST(TransformIterators, transform_results) {
    std::array numbers{1, 2, 3, 4};
    std::array results{1, 4, 9, 16};
    using iterators::const_zip;
    using iterators::transform;

    auto square = [](const auto &num) {
        return num * num;
    };

    for (auto[squared, expected]: const_zip(transform(numbers, square), results)) {
        EXPECT_EQ(squared, expected);
    }
}

TEST(TransformIterators, reference_result) {
    using iterators::transform;
    std::unordered_map<int, std::string> map{{1, "1"}, {2, "2"}, {3, "3"}};
    std::unordered_map<int, std::string> results{{1, "1a"}, {2, "2a"}, {3, "3a"}};
    auto values = [](auto &pair) -> auto & {
        return pair.second;
    };

    EXPECT_TRUE(std::is_lvalue_reference_v<decltype(*transform(map, values).begin())>);
    for (auto &string: transform(map, values)) {
        string += "a";
    }

    EXPECT_EQ(map, results);
}

TEST(TransformIterators, constness) {
    using iterators::transform;
    std::unordered_map<int, std::string> map{{1, "1"}, {2, "2"}, {3, "3"}};
    const auto &constMap = map;
    auto values = [](auto &pair) -> auto & {
        return pair.second;
    };

    auto t = transform(map, values);
    auto constT = transform(constMap, values);
    const auto &constTRef = t;
    EXPECT_FALSE(std::is_const_v<std::remove_reference_t<decltype(*t.begin())>>);
    EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(*constT.begin())>>);
    EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(*constTRef.begin())>>);
}

TEST(TransformIterators, deref_member_access) {
    using iterators::transform;
    std::map<int, std::string> map{{1, "1"}, {2, "2"}, {3, "3"}};
    auto identity = [](auto &a) -> auto & {return a;};
    auto it = transform(map, identity).begin();
    EXPECT_EQ(it->first, 1);
    EXPECT_EQ(it->second, "1");
}

TEST(TransformIterators, manual) {
    std::map<int, std::string> map{{1, "1"}, {2, "2"}, {3, "3"}};
    auto value = [](auto &pair) -> auto & { return pair.second; };
    auto tBegin = iterators::transform(map.begin(), value);
    auto tEnd = iterators::transform(map.end(), value);
    std::array expected{"1", "2", "3"};
    auto zBegin = iterators::zip_i(tBegin, expected.begin());
    auto zEnd = iterators::zip_i(tEnd, expected.end());
    while (zBegin != zEnd) {
        EXPECT_EQ(std::get<0>(*zBegin), std::get<1>(*zBegin));
        ++zBegin;
    }
}

TEST(TransformIterators, elements_no_copy) {
    using iterators::transform;
    std::vector<MustNotCopy> items;
    items.emplace_back("a");
    items.emplace_back("b");
    items.emplace_back("c");
    auto getString = [](auto &a) -> auto & {
        return a.s;
    };
    for (auto &item: transform(items, getString)) {
        item += std::to_string(1);
    }

    auto getStringWithMember = [i = MustNotCopy("a")](auto &a) -> auto & {
        return a.s;
    };

    for (auto &item : transform(items, std::move(getStringWithMember))) {
        item += std::to_string(2);
    }

    for (auto &item: transform(std::move(items), getString)) {
        item += std::to_string(3);
    }


}

TEST(TransformIterators, container_no_copy) {
    using iterators::transform;
    using iterators::const_zip;
    MustNotCopyContainer<std::pair<std::string, int>> strings{{"a", 1}, {"b", 2}, {"c", 3}};
    auto getString = [](auto &a) -> auto & {
        return a.first;
    };

    for (auto &string: transform(strings, getString)) {
        string += "x";
    }

    std::vector<std::string> results{"ax", "bx", "cx"};
    for (auto[string, result]: const_zip(transform(std::move(strings), getString), results)) {
        EXPECT_EQ(string, result);
    }
}

TEST(TransformIterators, temporary_lifetime) {
    using iterators::transform;
    using iterators::const_zip;
    std::array expected_values{1, 2, 3};
    bool allowToDie = false;
    auto identity = [](auto a) { return a; };
    for (auto[expected, actual]: const_zip(expected_values,
                                           transform(LifeTimeChecker<int>({1, 2, 3}, allowToDie), identity))) {
        EXPECT_EQ(expected, actual);
        if (actual == 3) { // after the last iteration, the temporary container may be destroyed
            allowToDie = true;
        }
    }
}

TEST(TransformIterators, iterator_traits_and_operators) {
    using iterators::transform;
    std::array numbers{1, 2, 3};
    auto identity = [](auto &x) -> auto & { return x; };
    const auto iterator = transform(numbers.begin(), identity);
    const auto end = transform(numbers.end(), identity);
    constexpr bool correctType = std::is_same_v<decltype(iterator)::iterator_category, std::random_access_iterator_tag>;
    constexpr bool correctValue = std::is_same_v<decltype(iterator)::value_type, int>;
    constexpr bool correctRef = std::is_same_v<decltype(iterator)::reference, int &>;
    constexpr bool correctPointer = std::is_same_v<decltype(iterator)::pointer, int *>;
    constexpr bool correctDiffType = std::is_same_v<decltype(iterator)::difference_type,
        std::iterator_traits<decltype(numbers.begin())>::difference_type>;
    EXPECT_TRUE(correctType);
    EXPECT_TRUE(correctValue);
    EXPECT_TRUE(correctRef);
    EXPECT_TRUE(correctPointer);
    EXPECT_TRUE(correctDiffType);
    EXPECT_EQ(iterator + 3, end);
    EXPECT_EQ(3 + iterator, end);
    EXPECT_EQ(end - iterator, numbers.size());
    EXPECT_TRUE(iterator < end);
    EXPECT_TRUE(end > iterator);
    EXPECT_TRUE(iterator <= end);
    EXPECT_TRUE(end >= iterator);
    iterator[1] *= 2;
    EXPECT_EQ(iterator[1], 4);
    auto incremented = iterator + 1;
    EXPECT_NE(iterator, incremented--);
    ++incremented;
    EXPECT_EQ(iterator, --incremented);
}

TEST(TransformIterators, iterator_traits_and_operators_1) {
    using iterators::transform;
    std::list numbers{4, 2, 3};
    std::unordered_map<int, std::string> map{{1, "1"}, {2, "2"}, {3, "3"}};
    auto square = [](auto x) { return x * x; };
    auto value = [](const auto &pair) { return pair.second; };
    auto mapIterator = transform(map.begin(), value);
    auto listIterator = transform(std::next(numbers.begin()), square);
    constexpr bool correctPointerMap = std::is_void_v<decltype(mapIterator)::pointer>;
    constexpr bool correctTypeMap = std::is_same_v<decltype(mapIterator)::iterator_category, std::forward_iterator_tag>;
    EXPECT_TRUE(correctPointerMap);
    EXPECT_TRUE(correctTypeMap);
    constexpr bool correctPointerList = std::is_void_v<decltype(listIterator)::pointer>;
    constexpr bool correctTypeList = std::is_same_v<decltype(listIterator)::iterator_category,
        std::bidirectional_iterator_tag>;
    EXPECT_TRUE(correctPointerList);
    EXPECT_TRUE(correctTypeList);
    --listIterator;
    EXPECT_EQ(*listIterator, 16);
}

TEST(TransformIterators, stl_algos) {
    using iterators::transform;
    std::vector<std::pair<int, std::string>> unordered{{1, "3"}, {2, "1"}, {3, "7"}};
    std::vector<std::pair<int, std::string>> ordered{{1, "1"}, {2, "3"}, {3, "7"}};
    std::vector<std::string> dest;
    std::vector<std::string> expectedValues = {"3", "1", "7"};
    auto value = [](auto &pair) -> auto & { return pair.second; };
    auto valueView = transform(unordered, value);
    std::copy(valueView.begin(), valueView.end(), std::back_inserter(dest));
    EXPECT_EQ(dest, expectedValues);
    std::sort(valueView.begin(), valueView.end());
    EXPECT_EQ(unordered, ordered);
}

TEST(TransformIterators, throwing_operators) {
    std::array numbers{1, 2, 3};
    auto throwingTIterator = iterators::transform(numbers.begin(), [](int) { throw std::runtime_error(""); });
    EXPECT_THROW(*throwingTIterator, std::runtime_error);
    BadIterator badIt;
    auto badTransform = iterators::transform(badIt, [](int) noexcept { return 0; });
    EXPECT_THROW(++badTransform, std::runtime_error);
    EXPECT_THROW(badTransform++, std::runtime_error);
    EXPECT_THROW(--badTransform, std::runtime_error);
    EXPECT_THROW(badTransform--, std::runtime_error);
    EXPECT_THROW(badTransform == badTransform, std::runtime_error);
    EXPECT_THROW(badTransform != badTransform, std::runtime_error);
    EXPECT_THROW(badTransform < badTransform, std::runtime_error);
    EXPECT_THROW(badTransform > badTransform, std::runtime_error);
    EXPECT_THROW(badTransform <= badTransform, std::runtime_error);
    EXPECT_THROW(badTransform >= badTransform, std::runtime_error);
    EXPECT_THROW(badTransform + 1, std::runtime_error);
    EXPECT_THROW(1 + badTransform, std::runtime_error);
    EXPECT_THROW(badTransform - 1, std::runtime_error);
    EXPECT_THROW(badTransform[1], std::runtime_error);
    EXPECT_THROW(*badTransform, std::runtime_error);
    EXPECT_THROW(badTransform - badTransform, std::runtime_error);
}
