#include <gtest/gtest.h>
#include <ranges>
#include <vector>
#include <string>
#include <list>
#include "Iterators.hpp"

TEST(cpp20_compat, view_concat) {
    using namespace iterators;
    using namespace std::views;
    std::vector numbers{1, 2, 3};
    std::list<std::string> strings{"a", "b", "c"};
    auto combined = {"a1", "b2", "c3"};
    auto combineView = transform(zip(strings, numbers),
                                 [](auto tuple) { return std::get<0>(tuple) + std::to_string(std::get<1>(tuple)); });
    EXPECT_TRUE(std::ranges::equal(combineView, combined));
}

TEST(cpp20_compat, view_concat_pipe) {
    using namespace iterators;
    using namespace std::views;
    std::vector vecA{1, 2, 3};
    std::list vecB{1, 2, 3};
    auto sqRange = zip(vecA, vecB) | transform([](auto tuple) {return std::get<0>(tuple) * std::get<1>(tuple); });
    for (auto [i, val] : enumerate(sqRange, 1)) {
        EXPECT_EQ(val, i * i);
    }

    auto indices = {0, 1, 2};
    auto indexRange = enumerate(vecA) | elements<0>;
    EXPECT_TRUE(std::ranges::equal(indices, indexRange));
}

TEST(cpp20_compat, multiple_concatenations) {
    using namespace iterators;
    using namespace std::views;
    std::vector numbers{1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto gt = {2, 5, 8};
    auto div3IndexRange = enumerate(numbers) |
            filter([](auto tuple) { return std::get<1>(tuple) % 3 == 0; }) | elements<0>;
    EXPECT_TRUE(std::ranges::equal(div3IndexRange, gt));
}

TEST(cpp20_compat, range_algo) {
    using namespace iterators;
    using namespace std::views;
    std::vector numbers{1, 2, 3, 4, 5};
    auto backForthView = zip(numbers, reverse(numbers));
    auto res = std::ranges::find_if(backForthView, [](auto tuple) { return std::get<0>(tuple) == std::get<1>(tuple); });
    ASSERT_NE(res, backForthView.end());
    EXPECT_EQ(std::get<0>(*res), 3);
}

TEST(cpp20_compat, range_algo_copy) {
    using namespace iterators;
    std::array numbers{4, 2, 3, 1, 0};
    std::vector numbers1{0, 1, 2, 3, 4};
    auto zView = zip(numbers, numbers1);
    std::ranges::copy(zView | std::views::drop(1), zView.begin());
    EXPECT_EQ(numbers, (std::array{2, 3, 1, 0, 0}));
    EXPECT_EQ(numbers1, (std::vector{1, 2, 3, 4, 4}));
}

TEST(cpp20_compat, range_algo_sort) {
    using namespace iterators;
    std::array<std::string, 4> strings{"a", "b", "c", "d"};
    std::array priorities{7, 1, 2, 0};
    auto zView = zip(strings, priorities);
    std::ranges::sort(zView, {}, [](const auto &tpl) { return std::get<1>(tpl); });
    EXPECT_EQ(strings, (std::array<std::string, 4>{"d", "b", "c", "a"}));
    EXPECT_EQ(priorities, (std::array{0, 1, 2, 7}));
}

TEST(cpp20_compat, range_members) {
    using namespace iterators;
    using namespace std::views;
    std::vector numbers{1, 2, 3, 4, 5};
    auto zipView = zip(numbers, reverse(numbers));
    EXPECT_EQ(zipView.front(), std::tuple(1, 5));
    EXPECT_EQ(zipView.back(), std::tuple(5, 1));
    EXPECT_EQ(zipView[2], std::tuple(3, 3));
    EXPECT_EQ(zipView.size(), 5);
    EXPECT_TRUE(zipView);
    EXPECT_FALSE(zipView.empty());
}

TEST(cpp20_compat, sentinel) {
    using namespace iterators;
    using namespace std::views;
    std::vector<int> numbers{1, 2, 3};
    EXPECT_TRUE((std::sentinel_for<impl::Unreachable, impl::CounterIterator<std::size_t>>));
    auto enumVew = enumerate(numbers);
    EXPECT_TRUE((std::sentinel_for<decltype(enumVew.end()), decltype(enumVew.begin())>));
    auto commonView = enumVew | common;
    std::vector<std::ranges::range_value_t<decltype(enumVew)>> target(commonView.begin(), commonView.end());
    std::vector<std::tuple<std::size_t, int>> gt{{0, 1}, {1, 2}, {2, 3}};
    EXPECT_TRUE(std::ranges::equal(target, gt));
}

TEST(cpp20_compat, subrange) {
    using namespace iterators;
    std::vector nums{1, 2, 3};
    auto range = std::ranges::subrange(zip_i(nums.begin()), zip_i(nums.end())) | std::views::elements<0>;
    EXPECT_EQ(range[2], 3);
    EXPECT_EQ(range.size(), 3);
    range[1] = 17;
    EXPECT_EQ(nums[1], 17);
}