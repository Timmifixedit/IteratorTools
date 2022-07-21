/**
 * @file Iterators.hpp
 * @author tim Luchterhand
 * @date 10.09.21
 * @brief This file contains the definitions of Python-like zip- and enumerate-functions. They can be used in range
 * based for-loops to loop over multiple ranges at the same time, or to index a range while looping respectively.
 */

#ifndef ITERATORTOOLS_ITERATORS_HPP
#define ITERATORTOOLS_ITERATORS_HPP

#include <algorithm>
#include <tuple>

#define REFERENCE(TYPE) std::declval<std::add_lvalue_reference_t<TYPE>>()

#define ALL_NOEXCEPT(OP, NAME) \
        template<typename T> \
        struct NAME { \
            static constexpr bool value = false; \
        }; \
        template<typename ...Ts> \
        struct NAME <std::tuple<Ts...>> { \
            static constexpr bool value = (... && noexcept(OP)); \
        };                 \
        template<typename T> \
        inline constexpr bool NAME##_v = NAME<T>::value;

#define ELEMENT1 std::get<Idx>(tuple1)
#define ELEMENT2 std::get<Idx>(tuple2)

#define BINARY_TUPLE_FOR_EACH(OPERATION, NAME) \
        template<typename Tuple1, typename Tuple2, std::size_t ...Idx> \
        static constexpr auto NAME##Impl(const Tuple1 &tuple1, const Tuple2 &tuple2, std::index_sequence<Idx...>) \
        noexcept(noexcept((OPERATION))) { \
            return (OPERATION); \
        } \
        template<typename Tuple1, typename Tuple2> \
        static constexpr auto NAME(const Tuple1 &tuple1, const Tuple2 &tuple2) \
        noexcept(noexcept(NAME##Impl(tuple1, tuple2, std::make_index_sequence<std::tuple_size_v<Tuple1>>{}))) { \
            static_assert(std::tuple_size_v<Tuple1> == std::tuple_size_v<Tuple2>); \
            return NAME##Impl(tuple1, tuple2, std::make_index_sequence<std::tuple_size_v<Tuple1>>{}); \
        }

#define BINARY_TUPLE_FOR_EACH_FOLD(OPERATION, COMBINATOR, NAME) BINARY_TUPLE_FOR_EACH( ( (OPERATION) COMBINATOR ...), NAME)

#define TYPE_MAP_DEFAULT \
        template<typename> \
        struct type_to_value {}; \
        template<std::size_t>    \
        struct value_to_type {};

#define TYPE_MAP(TYPE, VALUE) \
        template<>            \
        struct type_to_value<TYPE> { \
            static constexpr std::size_t value = VALUE; \
        };                    \
        template<>            \
        struct value_to_type<VALUE>{ \
            static_assert(VALUE != 0, "0 is a reserved value"); \
            using type = TYPE;\
        };

#define TYPE_MAP_ALIAS \
        template<typename T> \
        constexpr inline std::size_t type_to_value_v = type_to_value<T>::value; \
        template<std::size_t V>                                                 \
        using value_to_type_t = typename value_to_type<V>::type;

/**
 * @brief namespace containing zip and enumerate functions
 */
namespace iterators {

    /**
     * @brief namespace containing structures and helpers used to implement zip and enumerate.
     * Normally there is no need to use any of its members directly
     */
    namespace impl {

        /**
         * @brief namespace containing type traits used in implementation of zip and enumerate
         */
        namespace traits {
            template<bool Cond, typename T>
            using reference_if_t = std::conditional_t<Cond, std::add_lvalue_reference_t<T>, T>;

            template<bool Cond, typename T>
            using const_if_t = std::conditional_t<Cond, std::add_const_t<T>, T>;

            template<typename T, typename = std::void_t<>>
            struct is_container : std::false_type {};

            template<typename T>
            struct is_container<T, std::void_t<decltype(std::begin(std::declval<T>()), std::end(std::declval<T>()))>>
                    : std::true_type {};

            template<typename T>
            constexpr inline bool is_container_v = is_container<T>::value;

            template<typename T, typename = std::void_t<>>
            struct is_dereferencible : std::false_type {};

            template<typename T>
            struct is_dereferencible<T, std::void_t<decltype(*std::declval<T>())>> : std::true_type {};

            template<typename T>
            constexpr inline bool is_dereferencible_v = is_dereferencible<T>::value;

            template<typename T, bool B>
            struct dereference {
                using type = void;
            };

            template<typename T>
            struct dereference<T, true> {
                using type = decltype(*std::declval<T>());
            };

            template<typename T>
            using dereference_t = typename dereference<T, is_dereferencible_v<T>>::type;

            template<typename T>
            struct values{};

            template<typename ...Ts>
            struct values<std::tuple<Ts...>> {
                using type = std::tuple<dereference_t<Ts>...>;
            };

            template<typename T>
            using values_t = typename values<T>::type;

            ALL_NOEXCEPT(++REFERENCE(Ts), is_nothrow_incrementible)
            ALL_NOEXCEPT(--REFERENCE(Ts), is_nothrow_decrementible)
            ALL_NOEXCEPT(*REFERENCE(Ts), is_nothrow_dereferencible)
            ALL_NOEXCEPT(REFERENCE(Ts) += 5, is_nothrow_compound_assignable_plus)
            ALL_NOEXCEPT(REFERENCE(Ts) -= 5, is_nothrow_compound_assignable_minus)

            TYPE_MAP_DEFAULT

            TYPE_MAP(std::input_iterator_tag, 1)
            TYPE_MAP(std::forward_iterator_tag, 2)
            TYPE_MAP(std::bidirectional_iterator_tag, 3)
            TYPE_MAP(std::random_access_iterator_tag, 4)
            #if __cplusplus > 201703L
            TYPE_MAP(std::contiguous_iterator_tag, 5)
            #endif

            TYPE_MAP_ALIAS

            template<typename T, typename = std::void_t<>>
            struct iterator_category_value {
                static constexpr std::size_t value = 0;
            };

            template<typename T>
            struct iterator_category_value<T, std::void_t<typename std::iterator_traits<T>::iterator_category>> {
                static constexpr std::size_t value = type_to_value_v<typename std::iterator_traits<T>::iterator_category>;
            };

            template<std::size_t Val>
            struct iterator_category_from_value {
                using iterator_category = value_to_type_t<Val>;
            };

            template<>
            struct iterator_category_from_value<0> {};

            template<typename T>
            struct minimum_category {};

            template<typename ...Ts>
            struct minimum_category<std::tuple<Ts...>> {
                static constexpr std::size_t value = std::min({iterator_category_value<Ts>::value...});
            };

            template<typename T>
            constexpr inline std::size_t minimum_category_v = minimum_category<T>::value;

            template<typename T, typename = std::void_t<>>
            struct is_random_accessible {
                static constexpr bool value = false;
            };

            template<typename T>
            struct is_random_accessible<T, std::void_t<typename std::iterator_traits<T>::iterator_category>> {
                static constexpr bool value = std::is_base_of_v<std::random_access_iterator_tag,
                        typename std::iterator_traits<T>::iterator_category>;
            };

            template<typename ...Ts>
            struct is_random_accessible<std::tuple<Ts...>, std::void_t<value_to_type_t<minimum_category_v<std::tuple<Ts...>>>>> {
                static constexpr bool value = std::is_base_of_v<std::random_access_iterator_tag,
                        value_to_type_t<minimum_category_v<std::tuple<Ts...>>>>;
            };

            template<typename T>
            constexpr inline bool is_random_accessible_v = is_random_accessible<T>::value;

            template<typename T, typename = std::void_t<>>
            struct is_bidirectional {
                static constexpr bool value = false;
            };

            template<typename T>
            struct is_bidirectional<T, std::void_t<typename std::iterator_traits<T>::iterator_category>> {
                static constexpr bool value = std::is_base_of_v<std::bidirectional_iterator_tag,
                        typename std::iterator_traits<T>::iterator_category>;
            };

            template<typename ...Ts>
            struct is_bidirectional<std::tuple<Ts...>, std::void_t<value_to_type_t<minimum_category_v<std::tuple<Ts...>>>>> {
                static constexpr bool value = std::is_base_of_v<std::bidirectional_iterator_tag,
                        value_to_type_t<minimum_category_v<std::tuple<Ts...>>>>;
            };

            template<typename T>
            constexpr inline bool is_bidirectional_v = is_bidirectional<T>::value;
        }

        /**
         * @brief Class combining multiple iterators into one. Use it to iterate over multiple ranges at the same time.
         * @details @copybrief
         * ZipIterators only support the operators of the least powerful underling iterator. Zipping a random access
         * iterator (e.g. from std::vector) and a bidirectional iterator (e.g. from std::list) results in a
         * bidirectional iterator. ZipIterators return a tuple of references to the range elements. When using
         * structured bindings, no additional reference binding is necessary.
         *
         * Let ```z``` be a ZipIterator composed from two ```std::vector<int>```
         * ```
         * auto [val1, val2] = *z; // val1 and val2 are references to the vector elements
         * val1 = 17; // this will change the respective value in the first vector
         * ```
         * @tparam Iterators Underlying iterator types
         */
        template<typename Iterators>
        class ZipIterator : public traits::iterator_category_from_value<traits::minimum_category_v<Iterators>> {
        public:
            using value_type = traits::values_t<Iterators>;
            using reference = value_type;
            using pointer = void;
            using difference_type = std::ptrdiff_t;

            explicit constexpr ZipIterator(
                    const Iterators &iterators) noexcept(std::is_nothrow_copy_constructible_v<Iterators>)
                    : iterators(iterators) {}

            template<typename ...Its>
            explicit constexpr ZipIterator(Its &&...its) : iterators(std::forward<Its>(its)...) {}

            /**
             * Increments all underlying iterators by one
             * @return
             */
            constexpr ZipIterator &operator++() noexcept(traits::is_nothrow_incrementible_v<Iterators>) {
                std::apply([](auto &&...it) { (++it, ...); }, iterators);
                return *this;
            }

            /**
             * Post increment. Increments all underlying iterators by one
             * @return
             */
            constexpr ZipIterator operator++(int) noexcept(traits::is_nothrow_incrementible_v<Iterators>) {
                ZipIterator tmp = *this;
                ++*this;
                return tmp;
            }

            /**
             * @name bidirectional iteration
             * @brief the following operators are only available if all underlying iterators support bidirectional
             * access
             */
            ///@{

            /**
             * Decrements all underlying iterators by one. Only available if all iterators support at least
             * bidirectional access
             * @tparam IsBidirectional
             * @return
             */
            template<bool IsBidirectional = traits::is_bidirectional_v<Iterators>>
            constexpr auto operator--() noexcept(traits::is_nothrow_decrementible_v<Iterators>)
                -> std::enable_if_t<IsBidirectional, ZipIterator &> {
                std::apply([](auto &&...it) { (--it, ...); }, iterators);
                return *this;
            }

            /**
             * Post decrement. Decrements all underlying iterators by one. Only available if all iterators support at
             * least bidirectional access
             * @tparam IsBidirectional
             * @return
             */
            template<bool IsBidirectional = traits::is_bidirectional_v<Iterators>>
            constexpr auto operator--(int) noexcept(traits::is_nothrow_decrementible_v<Iterators>)
                -> std::enable_if_t<IsBidirectional, ZipIterator> {
                ZipIterator tmp = *this;
                --*this;
                return tmp;
            }

            ///@}

            /**
             * @name random access operators
             * @brief the following operators are only available if all underlying iterators support random access
             *
             */
            ///@{

            /**
             * Compound assignment increment. Increments all underlying iterators by n. Only available if all underlying
             * iterators support at least random access
             * @tparam IsRandomAccessible
             * @param n
             * @return
             */
            template<bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator+=(difference_type n) noexcept(traits::is_nothrow_compound_assignable_plus_v<Iterators>)
                    -> std::enable_if_t<IsRandomAccessible, ZipIterator &> {
                std::apply([n](auto &&...it) {((it += n), ...);}, iterators);
                return *this;
            }

            /**
             * Compound assignment decrement. Decrements all underlying iterators by n. Only available if all underlying
             * iterators support at least random access
             * @tparam IsRandomAccessible
             * @param n
             * @return
             */
            template<bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator-=(difference_type n) noexcept(traits::is_nothrow_compound_assignable_minus_v<Iterators>)
                    -> std::enable_if_t<IsRandomAccessible, ZipIterator &> {
                std::apply([n](auto &&...it) {((it -= n), ...);}, iterators);
                return *this;
            }

            /**
             * Returns a ZipIterator where all underlying iterators are incremented by n. Only available if all
             * underlying iterators support at least random access
             * @tparam IsRandomAccessible
             * @param n
             * @return
             */
            template<bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>>
            friend constexpr auto operator+(ZipIterator it, difference_type n)
            noexcept(traits::is_nothrow_compound_assignable_plus_v<Iterators>)
            -> std::enable_if_t<IsRandomAccessible, ZipIterator> {
                it += n;
                return it;
            }

            /**
             * Returns a ZipIterator where all underlying iterators are incremented by n. Only available if all
             * underlying iterators support at least random access
             * @tparam IsRandomAccessible
             * @param n
             * @return
             */
            template<bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>>
            friend constexpr auto operator+(difference_type n, ZipIterator it)
            noexcept(traits::is_nothrow_compound_assignable_plus_v<Iterators>)
            -> std::enable_if_t<IsRandomAccessible, ZipIterator> {
                it += n;
                return it;
            }

            /**
             * Returns a ZipIterator where all underlying iterators are decremented by n. Only available if all
             * underlying iterators support at least random access
             * @tparam IsRandomAccessible
             * @param n
             * @return
             */
            template<bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>>
            friend constexpr auto operator-(ZipIterator it, difference_type n)
            noexcept(traits::is_nothrow_compound_assignable_minus_v<Iterators>)
            -> std::enable_if_t<IsRandomAccessible, ZipIterator> {
                it -= n;
                return it;
            }

            /**
             * Returns the minimum pairwise difference n between all underlying iterators of *this and other, such that
             * other + n == *this
             * Only available if all underlying iterators support at least random access
             * @tparam IsRandomAccessible
             * @param other
             * @return
             */
            template<bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator-(const ZipIterator &other) const
            noexcept(noexcept(ZipIterator::minDifference(std::declval<Iterators>(), other.iterators)))
            -> std::enable_if_t<IsRandomAccessible, difference_type> {
                return minDifference(iterators, other.iterators);
            }

            /**
             * Given a ZipIterator it, returns *(it + n)
             * Only available if all underlying iterators support at least random access
             * @tparam IsRandomAccessible
             * @param n
             * @return
             */
            template<bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator[](difference_type n) const noexcept(noexcept(*(std::declval<ZipIterator>() + n)))
            -> std::enable_if_t<IsRandomAccessible, reference> {
                return *(*this + n);
            }

            /**
             * Returns true if all underlying iterators compare less to the corresponding iterators from other
             * Only available if all underlying iterators support at least random access
             * @tparam IsRandomAccessible
             * @param other
             * @return
             */
            template<bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator<(const ZipIterator &other) const
            noexcept(noexcept(ZipIterator::allLess(std::declval<Iterators>(),
                                                   other.iterators))) -> std::enable_if_t<IsRandomAccessible, bool> {
                return allLess(iterators, other.iterators);
            }

            /**
             * Returns true if all underlying iterators compare greater to the corresponding iterators from other
             * Only available if all underlying iterators support at least random access
             * @tparam IsRandomAccessible
             * @param other
             * @return
             */
            template<bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator>(const ZipIterator &other) const noexcept(noexcept(other < other))
            -> std::enable_if_t<IsRandomAccessible, bool> {
                return other < *this;
            }

            /**
             * Returns true if all underlying iterators compare less or equal to the corresponding iterators from
             * other. Only available if all underlying iterators support at least random access
             * @tparam IsRandomAccessible
             * @param other
             * @return
             */
            template<bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator<=(const ZipIterator &other) const
            noexcept(noexcept(std::declval<ZipIterator>() > other)) -> std::enable_if_t<IsRandomAccessible, bool> {
                return !(*this > other);
            }

            /**
             * Returns true if all underlying iterators compare greater or equal to the corresponding iterators from
             * other. Only available if all underlying iterators support at least random access
             * @tparam IsRandomAccessible
             * @param other
             * @return
             */
            template<bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator>=(const ZipIterator &other) const
            noexcept(noexcept(std::declval<ZipIterator>() < other)) -> std::enable_if_t<IsRandomAccessible, bool> {
                return !(*this < other);
            }

            ///@}

            /**
             * Returns true if at least one underlying iterator compares equal to the corresponding iterator from
             * other.
             * @tparam Its
             * @param other
             * @return
             */
            template<typename Its>
            constexpr bool operator==(const ZipIterator<Its> &other) const
            noexcept(noexcept(ZipIterator::oneEqual(std::declval<Iterators>(), other.getIterators()))) {
                return oneEqual(iterators, other.getIterators());
            }

            /**
             * True if *this is not equal to other
             * @tparam Its
             * @param other
             * @return
             */
            template<typename Its>
            constexpr bool operator!=(const ZipIterator<Its> &other) const
            noexcept(noexcept(std::declval<ZipIterator>() == other)) {
                return !(*this == other);
            }

            /**
             * Dereferences all underlying iterators and returns a tuple of the resulting iterator reference types
             * @return
             */
            constexpr auto operator*() const noexcept(traits::is_nothrow_dereferencible_v<Iterators>) {
                return std::apply([](auto &&...it) { return value_type(*it...); }, iterators);
            }

            /**
             * Const reference to underlying iterators
             * @return
             */
            constexpr auto getIterators() const noexcept -> const Iterators& {
                return iterators;
            }

        private:
            BINARY_TUPLE_FOR_EACH_FOLD(ELEMENT1 == ELEMENT2, ||, oneEqual)

            BINARY_TUPLE_FOR_EACH_FOLD(ELEMENT1 < ELEMENT2, &&, allLess)

            BINARY_TUPLE_FOR_EACH_FOLD(ELEMENT1 > ELEMENT2, &&, allGreater)

            BINARY_TUPLE_FOR_EACH(std::min<difference_type>({ELEMENT1 - ELEMENT2 ...}), minDifference)

            Iterators iterators;
        };

        /**
         * @brief Zip-view that provides begin() and end() member functions. Use to loop over multiple ranges at the
         * same time using ranged based for-loops.
         * @details @copybrief
         * Ranges are captured by lvalue reference, no copying occurs. Temporaries are allowed as well in which case
         * storage is moved into the zip-view.
         * @tparam Iterable Underlying range types
         */
        template<typename ...Iterable>
        struct ZipView {
        private:
            using ContainerTuple = std::tuple<Iterable...>;
            template<bool Const>
            using Iterators = std::tuple<decltype(std::begin(
                    std::declval<std::add_lvalue_reference_t<traits::const_if_t<Const, std::remove_reference_t<Iterable>>>>()))...>;
            template<bool Const>
            using Sentinels = std::tuple<decltype(std::end(
                    std::declval<std::add_lvalue_reference_t<traits::const_if_t<Const, std::remove_reference_t<Iterable>>>>()))...>;
            using IteratorTuple = Iterators<false>;
            using CIteratorTuple = Iterators<true>;
            using SentinelTuple = Sentinels<false>;
            using CSentinelTuple = Sentinels<true>;
        public:
            /**
             * CTor. Binds reference to ranges or takes ownership in case of rvalue references
             * @tparam Container range types
             * @param containers arbitrary number of ranges
             */
            template<typename ...Container>
            constexpr explicit ZipView(Container &&...containers) :
                containers(std::forward<Container>(containers)...) {}


            /**
            * Returns a ZipIterator to the first elements of the underlying ranges
            * @return ZipIterator created by invoking std::begin on all underlying ranges
            */
            constexpr auto begin() {
                return ZipIterator<IteratorTuple>(
                        std::apply([](auto &&...c) { return IteratorTuple(std::begin(c)...); }, containers));
            }

            /**
            * Returns a ZipIterator to the elements following the last elements of the the underlying ranges
            * @return ZipIterator created by invoking std::end on all underlying ranges
            */
            constexpr auto end() {
                return ZipIterator<SentinelTuple>(
                        std::apply([](auto &&...c) { return SentinelTuple(std::end(c)...); }, containers));
            }

            /**
             * @copydoc ZipView::begin()
             * @note returns a ZipIterator that does not allow changing the ranges' elements
             */
            constexpr auto begin() const {
                return ZipIterator<CIteratorTuple>(
                        std::apply([](auto &&...c) { return CIteratorTuple(std::begin(c)...); }, containers));
            }

            /**
             * @copydoc ZipView::end()
             */
            constexpr auto end() const {
                return ZipIterator<CSentinelTuple>(
                        std::apply([](auto &&...c) { return CSentinelTuple(std::end(c)...); }, containers));
            }

        private:
            ContainerTuple containers;
        };

        /**
         * @brief represents the unreachable end of an infinite sequence
         */
        struct Unreachable {};

        template<typename T>
        constexpr T sgn(T val) noexcept {
            return val < 0 ? T(-1) : T(1);
        }

        /**
         * @brief Iterator of an infinite sequence of numbers. Simply increments an internal counter
         * @tparam Type of the counter (most of the time this is ```std::size_t```)
         */
        template<typename T>
        struct CounterIterator {
            using value_type = T;
            using reference = T;
            using pointer = void;
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            static_assert(std::is_integral_v<T> && !std::is_floating_point_v<T>);

            /**
             * CTor.
             * @param begin start of number sequence
             * @param increment step size (default is 1)
             * @note Depending on the template type T, increment can also be negative.
             */
            explicit constexpr CounterIterator(T begin, T increment = T(1)) noexcept:
                    counter(begin), increment(increment) {}

            /**
             * Increments value by increment
             * @return
             */
            constexpr CounterIterator &operator++() noexcept {
                counter += increment;
                return *this;
            }

            /**
             * Post increment
             *
             * @copydoc CounterIterator::operator++
             */
            constexpr CounterIterator operator++(int) noexcept {
                CounterIterator tmp = *this;
                ++*this;
                return tmp;
            }

            /**
             * Decrements value by increment
             * @return
             */
            constexpr CounterIterator &operator--() noexcept {
                counter -= increment;
                return *this;
            }

            /**
             * Post decrement
             *
             * @copydoc CounterIterator::operator--
             */
            constexpr CounterIterator operator--(int) noexcept {
                CounterIterator tmp = *this;
                --*this;
                return tmp;
            }

            /**
             * Compound assignment increment. Increments value by n times increment
             * @param n number of steps
             * @return
             */
            constexpr CounterIterator &operator+=(difference_type n) noexcept {
                counter += n * increment;
                return *this;
            }

            /**
             * Returns a CounterIterator where the counter is incremented by n times increment
             * @param it Instance of CounterIterator
             * @param n number of steps
             * @return
             */
            friend constexpr CounterIterator operator+(CounterIterator it, difference_type n) noexcept {
                it += n;
                return it;
            }

            /**
             * @copydoc operator+
             */
            friend constexpr CounterIterator operator+(difference_type n, CounterIterator it) noexcept {
                it += n;
                return it;
            }

            /**
             * Compound assignment increment. Increments value by n times increment
             * @param n number of steps
             * @return
             */
            constexpr CounterIterator &operator-=(difference_type n) noexcept {
                counter -= n * increment;
                return *this;
            }

            /**
             * Returns a CounterIterator where the counter is incremented by n times increment
             * @param it Instance of CounterIterator
             * @param n number of steps
             * @return
             */
            friend constexpr CounterIterator operator-(CounterIterator it, difference_type n) noexcept {
                it -= n;
                return it;
            }

            /**
             * Difference between two CounterIterators
             * @param other
             * @return integer ```n``` such that ```other + n <= *this```
             * @note When other has the same increment as ```*this```, then the returned value is guaranteed to
             * fulfil ```other + n == *this```. In the following example, this is not the case:
             * ```c++
             * CounterIterator a(8, 1);
             * CounterIterator b(4, 3);
             * auto diff = a - b; // yields 1 since b + 1 <= a
             * ```
             */
            constexpr difference_type operator-(const CounterIterator &other) const noexcept {
                return static_cast<difference_type>((counter - other.counter) / other.increment);
            }

            /**
             * Given a CounterIterator ```it``` returns ```*(it + n)```
             * @param n  number of steps
             * @return
             */
            constexpr reference operator[](difference_type n) const noexcept {
                return counter + n * increment;
            }

            /**
             * Equality comparison.
             * @param other
             * @return true if counter of left and right hand side are equal
             */
            constexpr bool operator==(const CounterIterator &other) const noexcept {
                return counter == other.counter;
            }

            /**
             * Equality comparison with unreachable sentinel
             * @return false
             */
            constexpr bool operator==(Unreachable) const noexcept {
                return false;
            }

            /**
             * Inequality comparison.
             * @tparam It Type of right hand side
             * @param other
             * @return true if counter of left and right hand side are not equal
             */
            template<typename It>
            constexpr bool operator!=(const It &other) const noexcept {
                return !(*this == other);
            }

            /**
             * Less than comparison with Unreachable sentinel
             * @return true
             */
            constexpr bool operator<(Unreachable) const noexcept {
                return true;
            }

            /**
             * Greater than comparison with Unreachable sentinel
             * @return false
             */
            constexpr bool operator>(Unreachable) const noexcept {
                return false;
            }

            /**
             * Less comparison
             * @param other
             * @return returns true if there exists an integer ```n``` such
             * that ```(*this) + n = other```
             * @note If increment is negative then equality comparison is done by multiplying both side with -1.
             * For example: let ```it1 = 5``` and ```it2 = -2``` be two CounterIterators where ```it1``` has negative
             * increment. Then ```it1 < it2``` is true.
             */
            constexpr bool operator<(const CounterIterator &other) const noexcept {
                return sgn(increment) *  counter < sgn(increment) * other.counter;
            }

            /**
             * Greater comparison
             * @param other
             * @return returns true if there exists an integer ```n``` such
             * that ```(*this) - n = other```
             * @note If increment is negative then equality comparison is done by multiplying both side with -1.
             * For example: let ```it1 = 5``` and ```it2 = -2``` be two CounterIterators where ```it1``` has negative
             * increment. Then ```it1 > it2``` is false.
             */
            constexpr bool operator>(const CounterIterator &other) const noexcept {
                return sgn(increment) * counter > sgn(increment) * other.counter;
            }

            /**
             * Less or equal comparison
             * @tparam type of right hand side
             * @param other
             * @return true if left hand side is not greater than right hand side
             */
            template<typename It>
            constexpr bool operator<=(const It &other) const noexcept {
                return !(*this > other);
            }

            /**
             * Greater or equal comparison
             * @tparam type of right hand side
             * @param other
             * @return true if left hand side is not less than right hand side
             */
            template<typename It>
            constexpr bool operator>=(const It &other) const noexcept {
                return !(*this < other);
            }

            /**
             * Produces the counter value
             * @return value of internal counter
             */
            constexpr T operator*() const noexcept {
                return counter;
            }

        private:
            T counter;
            T increment;
        };

        /**
         * @brief Represents an infinite range of numbers
         * @tparam T type of number range
         */
        template<typename T = std::size_t>
        struct CounterRange {
            /**
             * CTor
             * @param start start of the range
             * @param increment step size
             * @note Depending on the template type T, increment can also be negative.
             */
            explicit constexpr CounterRange(T start, T increment) noexcept: start(start), increment(increment) {}

            /**
             * @return CounterIterator representing the beginning of the sequence
             */
            [[nodiscard]] constexpr CounterIterator<T> begin() const noexcept {
                return CounterIterator<T>(start, increment);
            }

            /**
             * @return Sentinel object representing the unreachable end of the sequence
             */
            [[nodiscard]] static constexpr Unreachable end() noexcept {
                return Unreachable{};
            }

        private:
            T start;
            T increment;
        };
    }

    /**
     * Function that is used to create a impl::ZipIterator from an arbitrary number of iterators
     * @tparam Iterators type of iterators
     * @param iterators arbitrary number of iterators
     * @return impl::ZipIterator
     * @note ZipIterators have the same iterator category as the least powerful underlying operator. This means that
     * for example, zipping a random access iterator and a bidirectional iterator only yields a bidirectional
     * impl::ZipIterator
     * @relatesalso impl::ZipIterator
     */
    template<typename ...Iterators>
    constexpr auto zip_i(Iterators ...iterators) -> impl::ZipIterator<std::tuple<Iterators...>> {
        return impl::ZipIterator<std::tuple<Iterators...>>(std::move(iterators)...);
    }

    /**
     * Function that can be used in range based loops to emulate the zip iterator from python.
     * As in python: if the passed containers have different lengths, the container with the least items decides
     * the overall range
     * @tparam Iterable Container types that support iteration
     * @param iterable Arbitrary number of containers
     * @return zip-container class that provides begin and end members to be used in range based for-loops
     * @relatesalso impl::ZipView
     */
    template<typename ...Iterable>
    constexpr auto zip(Iterable &&...iterable) {
        return impl::ZipView<Iterable...>(std::forward<Iterable>(iterable)...);
    }

    /**
     * Zip variant that does not allow manipulation of the container elements
     *
     * @copydoc zip
     */
    template<typename ...Iterable>
    constexpr auto const_zip(Iterable &&...iterable) {
        return impl::ZipView<impl::traits::reference_if_t<std::is_lvalue_reference_v<Iterable>,
                std::add_const_t<std::remove_reference_t<Iterable>>>...>(std::forward<Iterable>(iterable)...);
    }

    /**
     * Function that can be used in range based loops to emulate the enumerate iterator from python.
     * @tparam Container Container type that supports iteration
     * @tparam T type of enumerate counter (default std::size_t)
     * @param container Source container
     * @param start Optional index offset (default 0)
     * @param increment Optional index increment (default 1)
     * @return impl::ZipView that provides begin and end members to be used in range based for-loops.
     * @relatesalso impl::ZipView
     */
    template<typename Container, typename T = std::size_t>
    constexpr auto enumerate(Container &&container, T start = T(0), T increment = T(1)) {
        return zip(impl::CounterRange(start, increment), std::forward<Container>(container));
    }

    /**
     * enumerate variant that does not allow manipulation of the container elements
     *
     * @copydoc enumerate
     */
    template<typename Container, typename T = std::size_t>
    constexpr auto const_enumerate(Container &&container, T start = T(0), T increment = T(1)) {
        return const_zip(impl::CounterRange(start, increment), std::forward<Container>(container));
    }

}

#endif //ITERATORTOOLS_ITERATORS_HPP
