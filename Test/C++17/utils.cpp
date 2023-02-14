#include "utils.hpp"

MustNotCopy::MustNotCopy(std::string string) : s(std::move(string)) {}

MustNotCopy::MustNotCopy(const char *chars) : MustNotCopy(std::string(chars)) {}

MustNotCopy::MustNotCopy(const MustNotCopy &other) : s(other.s) {
    // Hack because FAIL() macro is a non-void return statement
    EXPECT_TRUE(false) << "Copy occurred";
}

bool MustNotCopy::operator==(const MustNotCopy &other) const {
    return s == other.s;
}

bool MustNotCopy::operator!=(const MustNotCopy &other) const {
    return !(*this == other);
}

BadIterator &BadIterator::operator++() {
    throw std::runtime_error("");
    return *this;
}

BadIterator BadIterator::operator++(int) {
    throw std::runtime_error("");
    return *this;
}

BadIterator &BadIterator::operator--(){
    throw std::runtime_error("");
    return *this;
}

BadIterator BadIterator::operator--(int) {
    throw std::runtime_error("");
    return *this;
}

BadIterator &BadIterator::operator+=(int){
    throw std::runtime_error("");
    return *this;
}

BadIterator &BadIterator::operator-=(int){
    throw std::runtime_error("");
    return *this;
}

int BadIterator::operator-(const BadIterator &) const {
    throw std::runtime_error("");
    return 0;
}

int BadIterator::operator[](int) const{
    throw std::runtime_error("");
    return 0;
}

bool BadIterator::operator<(const BadIterator &) const {
    throw std::runtime_error("");
    return false;
}

bool BadIterator::operator>(const BadIterator &) const {
    throw std::runtime_error("");
    return false;
}

bool BadIterator::operator<=(const BadIterator &) const {
    throw std::runtime_error("");
    return false;
}

bool BadIterator::operator>=(const BadIterator &) const {
    throw std::runtime_error("");
    return false;
}

bool BadIterator::operator==(const BadIterator &) const {
    throw std::runtime_error("");
    return false;
}

bool BadIterator::operator!=(const BadIterator &) const {
    throw std::runtime_error("");
    return false;
}

int BadIterator::operator*() const {
    throw std::runtime_error("");
    return 0;
}

int* BadIterator::operator->() const {
    throw std::runtime_error("");
    return nullptr;
}
