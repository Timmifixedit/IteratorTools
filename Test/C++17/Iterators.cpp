#include <gtest/gtest.h>
#include <vector>
#include <list>
#include <string>
#include <type_traits>
#include <deque>
#include <unordered_map>
#include "Iterators.hpp"
#include "utils.hpp"

#define UNUSED(x) ((void) (x))

DETECT(pre_inc, ++INSTANCE_OF(T))
DETECT(pre_dec, --INSTANCE_OF(T))
DETECT(post_inc, INSTANCE_OF(T)++)
DETECT(post_dec, INSTANCE_OF(T)--)
DETECT(plus_comp, INSTANCE_OF(T) += INSTANCE_OF(std::size_t))
DETECT(minus_comp, INSTANCE_OF(T) -= INSTANCE_OF(std::size_t))
DETECT(plus_left, INSTANCE_OF(T) + INSTANCE_OF(std::size_t))
DETECT(plus_right, INSTANCE_OF(std::size_t) + INSTANCE_OF(T))
DETECT(minus_left, INSTANCE_OF(T) - INSTANCE_OF(std::size_t))
DETECT(difference, INSTANCE_OF(T) - INSTANCE_OF(T))
DETECT(less, INSTANCE_OF(T) < INSTANCE_OF(T))
DETECT(greater, INSTANCE_OF(T) > INSTANCE_OF(T))
DETECT(leq, INSTANCE_OF(T) <= INSTANCE_OF(T))
DETECT(geq, INSTANCE_OF(T) >= INSTANCE_OF(T))
DETECT(eq, INSTANCE_OF(T) == INSTANCE_OF(T))
DETECT(ineq, INSTANCE_OF(T) != INSTANCE_OF(T))
DETECT(subsc, INSTANCE_OF(T)[INSTANCE_OF(std::size_t)])
DETECT(deref, *INSTANCE_OF(T))
DETECT(size, INSTANCE_OF(T).size())

DETECT_BINARY(eq_with, INSTANCE_OF(T) == INSTANCE_OF(U))
DETECT_BINARY(neq_with, INSTANCE_OF(T) != INSTANCE_OF(U))
DETECT_BINARY(less_with, INSTANCE_OF(T) < INSTANCE_OF(U))
DETECT_BINARY(greater_with, INSTANCE_OF(T) > INSTANCE_OF(U))
DETECT_BINARY(le_with, INSTANCE_OF(T) <= INSTANCE_OF(U))
DETECT_BINARY(ge_with, INSTANCE_OF(T) >= INSTANCE_OF(U))

template<typename Vec1, typename Vec2>
constexpr auto dot(const Vec1 &x, const Vec2 &y) {
    using T = decltype(*std::begin(x) * *std::begin(y));
    T res = 0;
    for (auto [v1, v2] : iterators::zip(x, y)) {
        res += v1 * v2;
    }

    return res;
}

template<std::size_t N, typename T>
constexpr T initArrayAndSum(T start, T increment) {
    T numbers[N] = {0};
    for (auto [number, i] : iterators::enumerate(numbers, start, increment)) {
        i = number;
    }

    T sum = 0;
    for (auto i : numbers) {
        sum += i;
    }

    return sum;
}

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
            std::is_const_v<std::remove_reference_t<decltype(std::get<0>(*zip_i(strings.cbegin(), numbers.begin())))>>);
    EXPECT_FALSE(
            std::is_const_v<std::remove_reference_t<decltype(std::get<1>(*zip_i(strings.cbegin(), numbers.begin())))>>);
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
    constexpr bool res1 = std::is_same_v<decltype(zip_i(forward.begin(), backward.begin())),
            decltype(zip_i(forward.end(), backward.end()))>;
    EXPECT_TRUE(res1);
}

TEST(Iterators, zip_iterator_manual) {
    using namespace iterators;
    std::list<int> forward {1, 2, 3};
    std::vector<int> backward{3, 2, 1};
    auto start = zip_i(forward.cbegin(), backward.cbegin());
    auto end = zip_i(forward.cend(), backward.cend());
    std::size_t counter = 0;
    while (start != end) {
        EXPECT_EQ(std::get<0>(*start) + std::get<1>(*start), 4);
        ++counter;
        ++start;
    }

    EXPECT_EQ(counter, 3);
}

TEST(Iterators, zip_c_arrays_manual) {
    using namespace iterators;
    int numbers[] = {1, 2, 3};
    const char * strings[] = {"a", "b", "c"};
    strings[1] = "4";
    auto zBegin = zip_i(numbers, strings);
    zBegin[1] = std::tuple{10, "f"};
    EXPECT_EQ(strings[1], "f");
    EXPECT_EQ(numbers[1], 10);
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

TEST(Iterators, zip_enumerate) {
    using namespace iterators;
    auto numbers = {1, 2, 3};
    auto strings = std::array{"a", "b", "c"};
    std::vector<std::string> results;
    for (auto [index, n, s] : zip_enumerate(numbers, strings)) {
        results.emplace_back(s + std::to_string(n) + std::to_string(index));
    }

    EXPECT_EQ(results, (std::vector<std::string>{"a10", "b21", "c32"}));
}

TEST(Iterators, zip_enumerate_offset) {
    using namespace iterators;
    auto numbers = {1, 2, 3};
    auto strings = std::array{"a", "b", "c"};
    std::vector<std::string> results;
    for (auto [index, n, s] : zip_enumerate(numbers, strings, 4, 2)) {
        results.emplace_back(s + std::to_string(n) + std::to_string(index));
    }

    EXPECT_EQ(results, (std::vector<std::string>{"a14", "b26", "c38"}));
    results.clear();
    for (auto [index, n, s] : zip_enumerate(numbers, strings, 4)) {
        results.emplace_back(s + std::to_string(n) + std::to_string(index));
    }

    EXPECT_EQ(results, (std::vector<std::string>{"a14", "b25", "c36"}));
}

TEST(Iterators, zip_enumerate_offset_no_copy) {
    using namespace iterators;
    auto numbers = {1, 2, 3};
    std::vector<MustNotCopy> strings;
    strings.emplace_back("a");
    strings.emplace_back("b");
    strings.emplace_back("c");
    bool allowedToDie = false;
    for (auto [index, n, s, _] : zip_enumerate(LifeTimeChecker<int>({1, 2, 3}, allowedToDie), strings, numbers, 4)) {
        s.s += std::to_string(n) + std::to_string(index);
        if (index == 6) {
            allowedToDie = true;
        }
    }
}

TEST(Iterators, const_zip_enumerate) {
    using namespace iterators;
    std::vector<int> numbers;
    EXPECT_TRUE((std::is_const_v<std::remove_reference_t<decltype(std::get<1>(
            *const_zip_enumerate(numbers, numbers).begin()))>>));
    EXPECT_TRUE((std::is_const_v<std::remove_reference_t<decltype(std::get<1>(
            *const_zip_enumerate(numbers, numbers, 2, 4).begin()))>>));

}

TEST(Iterators, type_traits) {
    using namespace iterators;
    std::array<int, 1> array;
    std::list<int> list;
    std::unordered_map<int, int> map;
    auto zipIt = zip_i(array.begin(), list.begin());
    constexpr bool correctCategory = std::is_same_v<decltype(zipIt)::iterator_category, std::bidirectional_iterator_tag>;
    EXPECT_TRUE(correctCategory);
    auto zipIt1 = zip_i(array.begin(), map.begin());
    constexpr bool correctCategory1 = std::is_same_v<decltype(zipIt1)::iterator_category, std::forward_iterator_tag>;
    EXPECT_TRUE(correctCategory1);
}

TEST(Iterators, bidirectional_operators) {
    using namespace iterators;
    std::array numbers{1, 2, 3, 4};
    std::list<std::string> strings{"a", "b", "c"};
    auto zIt = zip_i(numbers.begin(), strings.begin());
    zIt++;
    auto [n, s] =*zIt;
    EXPECT_EQ(n, 2);
    EXPECT_EQ(s, "b");
    ++zIt;
    auto [n1, s1] = *--zIt;
    EXPECT_EQ(n1, n);
    EXPECT_EQ(s1, s);
}

TEST(Iterators, post_increment_and_decrement) {
    using namespace iterators;
    std::array numbers{1, 2, 3, 4};
    std::list<std::string> strings{"a", "b", "c"};
    auto zIt = zip_i(numbers.begin(), strings.begin());
    auto [n, s] = *++zIt;
    EXPECT_EQ(n, 2);
    EXPECT_EQ(s, "b");
    auto [n1, s1] = *zIt++;
    EXPECT_EQ(n, n1);
    EXPECT_EQ(s, s1);
    auto [n2, s2] = *zIt;
    EXPECT_EQ(n2, 3);
    EXPECT_EQ(s2, "c");
    auto [dn, ds] = *--zIt;
    EXPECT_EQ(dn, 2);
    EXPECT_EQ(ds, "b");
    auto [dn1, ds1] = *zIt--;
    EXPECT_EQ(dn1, dn);
    EXPECT_EQ(ds1, ds);
    auto [dn2, ds2] = *zIt;
    EXPECT_EQ(dn2, 1);
    EXPECT_EQ(ds2, "a");
}

TEST(Iterators, random_access_operators) {
    using namespace iterators;
    std::array numbers{1, 2, 3, 4};
    std::vector<std::string> strings{"a", "b", "c"};
    const auto zBegin = zip_i(numbers.begin(), strings.begin());
    const auto zEnd = zip_i(numbers.end(), strings.end());
    EXPECT_EQ(*(zBegin + 2), zBegin[2]);
    EXPECT_LT(zBegin, zEnd);
    auto tmp = zBegin;
    tmp += 2;
    EXPECT_EQ(zBegin, zip_i(numbers.begin(), strings.begin()));
    EXPECT_EQ(*tmp, zBegin[2]);
    EXPECT_EQ(zBegin + 3, zEnd);
    EXPECT_LE(zBegin + 3, zEnd);
    EXPECT_GE(zBegin + 3, zEnd);
    EXPECT_FALSE(zBegin + 3 < zEnd);
    EXPECT_FALSE(zBegin + 3 > zEnd);
    EXPECT_EQ(zEnd - zBegin, 3);
}

TEST(Iterators, comparisons_with_different_types) {
    using namespace iterators;
    int numbers[] = {7, 8, 9};
    auto curr = enumerate(numbers).begin();
    auto cCurr = const_enumerate(numbers).begin();
    auto end = enumerate(numbers).end();
    EXPECT_TRUE(curr <= cCurr);
    EXPECT_FALSE(cCurr < cCurr);
    ++curr;
    EXPECT_EQ(curr - cCurr, 1);
    EXPECT_EQ(cCurr - curr, -1);
    EXPECT_TRUE(cCurr <= curr);
    EXPECT_TRUE(curr > cCurr);
    EXPECT_TRUE(curr >= cCurr);
    EXPECT_TRUE(curr != cCurr);
    EXPECT_TRUE(cCurr < curr);
    EXPECT_TRUE(curr != end);
    EXPECT_TRUE(end != cCurr);
}

TEST(Iterators, SFINAE_forward_operators) {
    using namespace iterators;
    using ForwardZipIt = decltype(zip(std::declval<std::unordered_map<int, std::string>>(),
                                      std::declval<std::add_lvalue_reference_t<int[3]>>()).begin());
    EXPECT_TRUE(has_pre_inc_v<ForwardZipIt>);
    EXPECT_TRUE(has_post_inc_v<ForwardZipIt>);
    EXPECT_TRUE(has_eq_v<ForwardZipIt>);
    EXPECT_TRUE(has_ineq_v<ForwardZipIt>);
    EXPECT_TRUE(has_deref_v<ForwardZipIt>);

    EXPECT_FALSE(has_pre_dec_v<ForwardZipIt>);
    EXPECT_FALSE(has_post_dec_v<ForwardZipIt>);
    EXPECT_FALSE(has_plus_comp_v<ForwardZipIt>);
    EXPECT_FALSE(has_minus_comp_v<ForwardZipIt>);
    EXPECT_FALSE(has_plus_left_v<ForwardZipIt>);
    EXPECT_FALSE(has_plus_right_v<ForwardZipIt>);
    EXPECT_FALSE(has_minus_left_v<ForwardZipIt>);
    EXPECT_FALSE(has_difference_v<ForwardZipIt>);
    EXPECT_FALSE(has_less_v<ForwardZipIt>);
    EXPECT_FALSE(has_greater_v<ForwardZipIt>);
    EXPECT_FALSE(has_leq_v<ForwardZipIt>);
    EXPECT_FALSE(has_geq_v<ForwardZipIt>);
    EXPECT_FALSE(has_subsc_v<ForwardZipIt>);
}

TEST(Iterators, SFINAE_bidirectional_operators) {
    using namespace iterators;
    using BidirZipIt = decltype(zip(std::declval<std::list<int>>(), std::declval<std::vector<std::string>>()).begin());
    EXPECT_TRUE(has_pre_inc_v<BidirZipIt>);
    EXPECT_TRUE(has_post_inc_v<BidirZipIt>);
    EXPECT_TRUE(has_eq_v<BidirZipIt>);
    EXPECT_TRUE(has_ineq_v<BidirZipIt>);
    EXPECT_TRUE(has_deref_v<BidirZipIt>);
    EXPECT_TRUE(has_pre_dec_v<BidirZipIt>);
    EXPECT_TRUE(has_post_dec_v<BidirZipIt>);

    EXPECT_FALSE(has_plus_comp_v<BidirZipIt>);
    EXPECT_FALSE(has_minus_comp_v<BidirZipIt>);
    EXPECT_FALSE(has_plus_left_v<BidirZipIt>);
    EXPECT_FALSE(has_plus_right_v<BidirZipIt>);
    EXPECT_FALSE(has_minus_left_v<BidirZipIt>);
    EXPECT_FALSE(has_difference_v<BidirZipIt>);
    EXPECT_FALSE(has_less_v<BidirZipIt>);
    EXPECT_FALSE(has_greater_v<BidirZipIt>);
    EXPECT_FALSE(has_leq_v<BidirZipIt>);
    EXPECT_FALSE(has_geq_v<BidirZipIt>);
    EXPECT_FALSE(has_subsc_v<BidirZipIt>);
}

TEST(Iterators, SFINAE_random_access_operators) {
    using namespace iterators;
    using RandomAccessIterator = decltype(zip(std::declval<std::array<int, 4>>(),
                                              std::declval<std::vector<std::string>>()).begin());
    EXPECT_TRUE(has_pre_inc_v<RandomAccessIterator>);
    EXPECT_TRUE(has_post_inc_v<RandomAccessIterator>);
    EXPECT_TRUE(has_eq_v<RandomAccessIterator>);
    EXPECT_TRUE(has_ineq_v<RandomAccessIterator>);
    EXPECT_TRUE(has_deref_v<RandomAccessIterator>);
    EXPECT_TRUE(has_pre_dec_v<RandomAccessIterator>);
    EXPECT_TRUE(has_post_dec_v<RandomAccessIterator>);
    EXPECT_TRUE(has_plus_comp_v<RandomAccessIterator>);
    EXPECT_TRUE(has_minus_comp_v<RandomAccessIterator>);
    EXPECT_TRUE(has_plus_left_v<RandomAccessIterator>);
    EXPECT_TRUE(has_plus_right_v<RandomAccessIterator>);
    EXPECT_TRUE(has_minus_left_v<RandomAccessIterator>);
    EXPECT_TRUE(has_difference_v<RandomAccessIterator>);
    EXPECT_TRUE(has_less_v<RandomAccessIterator>);
    EXPECT_TRUE(has_greater_v<RandomAccessIterator>);
    EXPECT_TRUE(has_leq_v<RandomAccessIterator>);
    EXPECT_TRUE(has_geq_v<RandomAccessIterator>);
    EXPECT_TRUE(has_subsc_v<RandomAccessIterator>);
}

TEST(Iterators, SFINAE_invalid_iterators) {
    using namespace iterators::impl;
    using BrokenIt = ZipIterator<std::tuple<int *, int>>;
    EXPECT_TRUE(has_pre_inc_v<BrokenIt>);
    EXPECT_TRUE(has_post_inc_v<BrokenIt>);
    EXPECT_TRUE(has_eq_v<BrokenIt>);
    EXPECT_TRUE(has_ineq_v<BrokenIt>);

    EXPECT_FALSE(has_deref_v<BrokenIt>);
    EXPECT_FALSE(has_pre_dec_v<BrokenIt>);
    EXPECT_FALSE(has_post_dec_v<BrokenIt>);
    EXPECT_FALSE(has_plus_comp_v<BrokenIt>);
    EXPECT_FALSE(has_minus_comp_v<BrokenIt>);
    EXPECT_FALSE(has_plus_left_v<BrokenIt>);
    EXPECT_FALSE(has_plus_right_v<BrokenIt>);
    EXPECT_FALSE(has_minus_left_v<BrokenIt>);
    EXPECT_FALSE(has_difference_v<BrokenIt>);
    EXPECT_FALSE(has_less_v<BrokenIt>);
    EXPECT_FALSE(has_greater_v<BrokenIt>);
    EXPECT_FALSE(has_leq_v<BrokenIt>);
    EXPECT_FALSE(has_geq_v<BrokenIt>);
    EXPECT_FALSE(has_subsc_v<BrokenIt>);
}

TEST(Iterators, SFINAE_differnt_zip_types) {
    using namespace iterators::impl;
    using NormaleIt = ZipIterator<std::tuple<int*>>;
    using ConstIt = ZipIterator<std::tuple<const int*>>;
    using DequeIt = ZipIterator<std::tuple<decltype(std::declval<std::deque<double>>().begin())>>;
    auto res = has_eq_with_v<NormaleIt, ConstIt>;
    EXPECT_TRUE(res);
    res = has_neq_with_v<NormaleIt, ConstIt>;
    EXPECT_TRUE(res);
    res = has_less_with_v<NormaleIt, ConstIt>;
    EXPECT_TRUE(res);
    res = has_greater_with_v<NormaleIt, ConstIt>;
    EXPECT_TRUE(res);
    res = has_le_with_v<NormaleIt, ConstIt>;
    EXPECT_TRUE(res);
    res = has_ge_with_v<NormaleIt, ConstIt>;
    EXPECT_TRUE(res);

    res = has_eq_with_v<NormaleIt, DequeIt>;
    EXPECT_FALSE(res);
    res = has_neq_with_v<NormaleIt, DequeIt>;
    EXPECT_FALSE(res);
    res = has_less_with_v<NormaleIt, DequeIt>;
    EXPECT_FALSE(res);
    res = has_greater_with_v<NormaleIt, DequeIt>;
    EXPECT_FALSE(res);
    res = has_le_with_v<NormaleIt, DequeIt>;
    EXPECT_FALSE(res);
    res = has_ge_with_v<NormaleIt, DequeIt>;
    EXPECT_FALSE(res);
}

TEST(Iterators, stl_algos) {
    using namespace iterators;
    std::array numbers{4, 2, 3, 1, 0};
    auto zBegin = zip_i(numbers.begin(), numbers.rbegin());
    auto zEnd = zip_i(numbers.end(), numbers.rend());
    auto res = std::find_if(zBegin, zEnd, [](const auto &tuple) { return std::get<0>(tuple) == std::get<1>(tuple); });
    ASSERT_NE(res, zEnd);
    EXPECT_EQ(std::get<0>(*res), 3);
}

TEST(Iterators, noexcept_stl_containers) {
    using namespace iterators;
    std::array numbers {1, 2, 3};
    std::list<std::string> strings{"1", "2" , "3"};
    std::unordered_map<int, std::string> map{{1, "1"}, {2, "2"}, {3, "3"}};
    auto zipper = zip(numbers, strings, map);
    auto zipIt = zipper.begin();
    auto nIt = numbers.begin();
    auto sIt = strings.begin();
    auto mapIt = map.begin();
    EXPECT_EQ(noexcept(++zipIt), noexcept(++nIt) && noexcept(++sIt) && noexcept(++mapIt));
    EXPECT_EQ(noexcept(zipIt++), noexcept(nIt++) && noexcept(sIt++) && noexcept(mapIt++));
    EXPECT_EQ(noexcept(zipIt == zipIt), noexcept(nIt == nIt) && noexcept(sIt == sIt) && noexcept(mapIt == mapIt));
    EXPECT_EQ(noexcept(zipIt != zipIt), noexcept(nIt != nIt) && noexcept(sIt != sIt) && noexcept(mapIt != mapIt));
    EXPECT_EQ(noexcept(*zipIt), noexcept(*nIt) && noexcept(*sIt) && noexcept(*mapIt));
}

TEST(Iterators, noexcept_true) {
    using namespace iterators;
    int numbers[] = {1, 2, 3};
    auto begin = enumerate(numbers).begin();
    auto end = begin + 2;
    EXPECT_TRUE(noexcept(begin == end));
    EXPECT_TRUE(noexcept(begin != end));
    EXPECT_TRUE(noexcept(begin < end));
    EXPECT_TRUE(noexcept(begin > end));
    EXPECT_TRUE(noexcept(begin <= end));
    EXPECT_TRUE(noexcept(begin >= end));
    EXPECT_TRUE(noexcept(begin[1]));
    EXPECT_TRUE(noexcept(*begin));
    EXPECT_TRUE(noexcept(1 + begin + 1));
    EXPECT_TRUE(noexcept(begin - 1));
    EXPECT_TRUE(noexcept(begin += 1));
    EXPECT_TRUE(noexcept(begin -= 1));
    EXPECT_TRUE(noexcept(++begin++));
    EXPECT_TRUE(noexcept(--begin--));
}

TEST(Iterators, noexcept_throwing_iterator) {
    using namespace iterators;
    std::array numbers{1, 2, 3};
    BadIterator badIt;
    // zip order is necessary so the comparison operators of BadIterator actually get called
    auto zipIt = zip_i(badIt, numbers.begin());
    EXPECT_THROW(++zipIt, std::runtime_error);
    EXPECT_THROW(zipIt++, std::runtime_error);
    EXPECT_THROW(--zipIt, std::runtime_error);
    EXPECT_THROW(zipIt--, std::runtime_error);
    EXPECT_THROW(UNUSED(zipIt == zipIt), std::runtime_error);
    EXPECT_THROW(UNUSED(zipIt != zipIt), std::runtime_error);
    EXPECT_THROW(UNUSED(zipIt < zipIt), std::runtime_error);
    EXPECT_THROW(UNUSED(zipIt > zipIt), std::runtime_error);
    EXPECT_THROW(UNUSED(zipIt <= zipIt), std::runtime_error);
    EXPECT_THROW(UNUSED(zipIt >= zipIt), std::runtime_error);
    EXPECT_THROW(zipIt + 1, std::runtime_error);
    EXPECT_THROW(1 + zipIt, std::runtime_error);
    EXPECT_THROW(zipIt - 1, std::runtime_error);
    EXPECT_THROW(zipIt[1], std::runtime_error);
    EXPECT_THROW(*zipIt, std::runtime_error);
    EXPECT_THROW(zipIt - zipIt, std::runtime_error);
}

TEST(Iterators, compiletime_foreach) {
    using namespace iterators;
    constexpr int x[4] = {3, 2, 1, 5};
    constexpr double y[4] = {10.21, 100.0014, 221.3009, 177.04};
    constexpr auto res = dot(x, y);
    EXPECT_DOUBLE_EQ(res, 1337.1337);
}

TEST(Iterators, compiletime_enumerate) {
    constexpr auto sum = initArrayAndSum<4>(-1, -1);
    EXPECT_EQ(sum, -10);
}

TEST(Iterators, compiletime_iterator) {
    using namespace iterators;
    static constexpr int x[] = {1, 2, 3, 4};
    static constexpr double y[] = {3, 2, 1, 4};
    constexpr auto res = zip(x, y).begin()[2];
    EXPECT_EQ(std::get<0>(res), 3);
    EXPECT_EQ(std::get<1>(res), 1);
    constexpr bool less = zip_i(x + 2, y + 1) < zip_i(x + 3, y + 3);
    EXPECT_TRUE(less);
    constexpr bool greater = zip_i(x + 2, y + 1) > zip_i(x + 1, y);
    EXPECT_TRUE(greater);
    constexpr auto diff = zip_i(x + 2, y + 1) - zip_i(x + 1, y + 1);
    EXPECT_EQ(diff, 0);
    constexpr auto minus = zip_i(x + 2, y + 3) - 1;
    constexpr auto plus = zip_i(x, y + 1) + 1;
    constexpr bool equal = plus == minus;
    EXPECT_TRUE(equal);
    constexpr auto plusComp = zip_i(x + 1, y) += 1;
    constexpr auto minusComp = zip_i(x + 3, y + 2) -= 1;
    EXPECT_EQ(plusComp, minusComp);
    constexpr bool decrementEq = --zip_i(x + 1, y + 3) == zip_i(x, y + 2);
    EXPECT_TRUE(decrementEq);
}

TEST(Iterators, size) {
    using namespace iterators;
    static constexpr int numbers[] = {1, 2, 3 , 4};
    static constexpr auto strings = {"a", "b", "c"};
    static constexpr std::array numbers1{1, 4};
    std::list doubles{0.1, 0.4, 0.2, 1.9, 1.4};
    auto zipView = zip(numbers, strings, doubles);
    EXPECT_EQ(zipView.size(), 3);
    constexpr auto zipC = zip(numbers, numbers1, strings);
    constexpr auto size = zipC.size();
    EXPECT_EQ(size, 2);
    using namespace iterators;
    EXPECT_FALSE(has_size_v<decltype(enumerate(numbers))>);
}

TEST(Iterators, zip_subscript) {
    using namespace iterators;
    static constexpr int numbers[] = {1, 2, 3 , 4};
    static constexpr auto strings = {"a", "b", "c"};
    constexpr auto res = zip(numbers, strings)[2];
    EXPECT_EQ(std::get<0>(res), 3);
    EXPECT_EQ(std::get<1>(res), "c");
    std::list list{1, 2, 3};
    EXPECT_FALSE(has_subsc_v<decltype(zip(numbers, list))>);
}