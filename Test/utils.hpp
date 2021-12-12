#ifndef BIDIRECTIONALMAP_MUSTNOTCOPY_HPP
#define BIDIRECTIONALMAP_MUSTNOTCOPY_HPP

#include <string>
#include <gtest/gtest.h>

class MustNotCopy {
public:
    MustNotCopy(std::string string);
    MustNotCopy(const char *chars);
    MustNotCopy() = default;
    MustNotCopy(const MustNotCopy &other);

    MustNotCopy(MustNotCopy &&) = default;
    MustNotCopy &operator=(const MustNotCopy &) = default;
    MustNotCopy &operator=(MustNotCopy &&) = default;
    ~MustNotCopy() = default;

    operator std::string () {
        return s;
    }

    bool operator==(const MustNotCopy &other) const;

    bool operator!=(const MustNotCopy &other) const;

    std::string s;
};


template<typename T>
struct MustNotCopyContainer {
    MustNotCopyContainer(std::initializer_list<T> init) : values(init.begin(), init.end()) {}

    MustNotCopyContainer(const MustNotCopyContainer &) {
        EXPECT_TRUE(false) << "Container was copied";
    }

    MustNotCopyContainer(MustNotCopyContainer &&other) = default;
    MustNotCopyContainer &operator=(const MustNotCopyContainer &) = default;
    MustNotCopyContainer &operator=(MustNotCopyContainer &&) = default;
    ~MustNotCopyContainer() = default;

    auto begin() {
        return values.begin();
    }

    auto begin() const {
        return values.begin();
    }

    auto end() {
        return values.end();
    }

    auto end() const {
        return values.end();
    }

private:
    std::vector<T> values;
};


template<typename T>
struct LifeTimeChecker {
    explicit LifeTimeChecker(std::initializer_list<T> init, const bool &deathFlag) : values(init.begin(), init.end()),
                                                                                     allowedToDie(deathFlag) {};

    LifeTimeChecker(const LifeTimeChecker &other) = default;

    LifeTimeChecker(LifeTimeChecker &&other) noexcept: values(std::move(other.values)),
                                                       allowedToDie(other.allowedToDie) {
        other.moved = true;
    }

    ~LifeTimeChecker() {
        EXPECT_TRUE(allowedToDie || moved) << "DTor called although not allowed yet";
    }

    auto begin() {
        return values.begin();
    }

    auto begin() const {
        return values.begin();
    }

    auto end() {
        return values.end();
    }

    auto end() const {
        return values.end();
    }

private:
    std::vector<T> values;
    const bool &allowedToDie;
    bool moved = false;
};

struct BadIterator {
    using value_type = int;
    using reference = int;
    using pointer = int*;
    using difference_type = int;
    using iterator_category = std::random_access_iterator_tag;

    BadIterator &operator++();

    BadIterator operator++(int);

    BadIterator &operator--();

    BadIterator operator--(int);

    BadIterator &operator+=(int);

    BadIterator &operator-=(int);

    friend BadIterator operator+(BadIterator, int) {
        throw std::runtime_error("");
        return BadIterator{};
    }

    friend BadIterator operator+(int, BadIterator) {
        throw std::runtime_error("");
        return BadIterator{};
    }

    friend BadIterator operator-(BadIterator, int) {
        throw std::runtime_error("");
        return BadIterator{};
    }

    int operator-(const BadIterator &) const;

    int operator[](int) const;

    bool operator<(const BadIterator &) const;

    bool operator>(const BadIterator &) const;

    bool operator<=(const BadIterator &) const;

    bool operator>=(const BadIterator &) const;

    bool operator==(const BadIterator &) const;

    bool operator!=(const BadIterator &) const;

    int operator*() const;

    int* operator->() const;
};

#endif //BIDIRECTIONALMAP_MUSTNOTCOPY_HPP
