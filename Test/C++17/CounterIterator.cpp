#include <gtest/gtest.h>
#include "Iterators.hpp"

TEST(CounterIterator, basic) {
    using namespace iterators::impl;
    CounterIterator test(0);
    EXPECT_EQ(*test, 0);
    EXPECT_EQ(*++test, 1);
    EXPECT_EQ(*test++, 1);
    EXPECT_EQ(*test, 2);
    EXPECT_EQ(test, CounterIterator(2));
    EXPECT_NE(test, CounterIterator(3));
    EXPECT_EQ(--test, CounterIterator(1));
    EXPECT_EQ(test--, CounterIterator(1));
    EXPECT_EQ(*test, 0);
    EXPECT_EQ(test + 3, CounterIterator(3));
    EXPECT_EQ(3 + test, CounterIterator(3));
    test += 5;
    EXPECT_EQ(*test, 5);
    EXPECT_EQ(test - 2, CounterIterator(3));
    test -= 3;
    EXPECT_EQ(*test, 2);
    EXPECT_EQ(test[4], 6);
}

TEST(CounterIterator, default_ctor) {
    using namespace iterators::impl;
    CounterIterator ct;
    EXPECT_EQ(ct, CounterIterator<std::size_t>(0));
}

TEST(CounterIterator, sentinel) {
    using namespace iterators::impl;
    CounterIterator test(3);
    EXPECT_TRUE(test != Unreachable{});
    EXPECT_TRUE(Unreachable{} != test);
    test += 1000;
    EXPECT_TRUE(test != Unreachable{});
}

TEST(CounterIterator, less_greater_difference_basic) {
    using namespace iterators::impl;
    EXPECT_LT(CounterIterator(17), CounterIterator(23));
    EXPECT_LE(CounterIterator(17), CounterIterator(23));
    EXPECT_LE(CounterIterator(17), CounterIterator(17));
    EXPECT_GT(CounterIterator(17), CounterIterator(12));
    EXPECT_GE(CounterIterator(17), CounterIterator(17));
    EXPECT_EQ(CounterIterator(17) - CounterIterator(12), 5);
    CounterIterator test(14, 2);
    EXPECT_EQ(test - CounterIterator(2), 12);
    EXPECT_EQ(CounterIterator(18) - test, 2);
    EXPECT_EQ(test - CounterIterator(18), -4);
    EXPECT_EQ(CounterIterator(10) - test, -2);
}

TEST(CounterIterator, negative_increment) {
    using namespace iterators::impl;
    CounterIterator reverse(0, -1);
    EXPECT_EQ(*++reverse, -1);
    EXPECT_EQ(*--reverse, 0);
    EXPECT_EQ(reverse + 4, CounterIterator(-4));
    EXPECT_EQ(reverse - 3, CounterIterator(3));
    reverse += 3;
    EXPECT_EQ(*reverse, -3);
    EXPECT_EQ(reverse[7], -10);
    reverse -= 2;
    EXPECT_EQ(*reverse, -1);
    EXPECT_EQ(reverse[-2], 1);

}

TEST(CounterIterator, less_greater_difference_reverse) {
    using namespace iterators::impl;
    CounterIterator reverseOne(4, -1);
    EXPECT_EQ(reverseOne, CounterIterator(4, 3));
    EXPECT_LT(reverseOne, CounterIterator(-1));
    EXPECT_LE(reverseOne, CounterIterator(0));
    EXPECT_GT(reverseOne, CounterIterator(6));
    EXPECT_GE(reverseOne, CounterIterator(5));
    CounterIterator reverseThree(7, -3);
    EXPECT_EQ(reverseOne - reverseThree, 1);
    EXPECT_EQ(reverseThree - reverseOne, -3);
    reverseOne += 12;
    EXPECT_EQ(reverseOne - reverseThree, 5);
}
