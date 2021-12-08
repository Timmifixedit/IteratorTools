#include <gtest/gtest.h>
#include <array>
#include <vector>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <map>
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

    for (auto [squared, expected] : const_zip(transform(numbers, square), results)) {
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
    for (auto &string : transform(map, values)) {
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
    auto zBegin = iterators::zip(tBegin, expected.begin());
    auto zEnd = iterators::zip(tEnd, expected.end());
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
    for (auto &item : transform(items, getString)) {
        item += std::to_string(1);
    }

    for (auto &item : transform(std::move(items), getString)) {
        item += std::to_string(2);
    }
}
TEST(TransformIterators, container_no_copy) {
    using iterators::transform;
    using iterators::const_zip;
    MustNotCopyContainer<std::pair<std::string, int>> strings{{"a", 1}, {"b", 2}, {"c", 3}};
    auto getString = [](auto &a) -> auto & {
        return a.first;
    };

    for (auto &string : transform(strings, getString)) {
        string += "x";
    }

    std::vector<std::string> results{"ax", "bx", "cx"};
    for (auto [string, result] : const_zip(transform(std::move(strings), getString), results)) {
        EXPECT_EQ(string, result);
    }
}

TEST(TransformIterators, temporary_lifetime) {
    using iterators::transform;
    using iterators::const_zip;
    std::array expected_values{1, 2, 3};
    bool allowToDie = false;
    auto identity = [](auto a) {return a;};
    for (auto[expected, actual]: const_zip(expected_values,
                                           transform(LifeTimeChecker<int>({1, 2, 3}, allowToDie), identity))) {
        EXPECT_EQ(expected, actual);
        if (actual == 3) { // after the last iteration, the temporary container may be destroyed
            allowToDie = true;
        }
    }
}
