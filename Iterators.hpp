/**
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
#define COMMA ,
#define CONST_SIGNATURE const
#define NON_CONST_SIGNATURE

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

#define BINARY_TUPLE_FOR_EACH(SIGNATURE, OPERATION, NAME) \
        template<typename Tuple1, typename Tuple2, std::size_t ...Idx> \
        static constexpr auto NAME##Impl(SIGNATURE Tuple1 &tuple1, SIGNATURE Tuple2 &tuple2, std::index_sequence<Idx...>) \
        noexcept(noexcept((OPERATION))) { \
            return (OPERATION); \
        } \
        template<typename Tuple1, typename Tuple2> \
        static constexpr auto NAME(SIGNATURE Tuple1 &tuple1, SIGNATURE Tuple2 &tuple2) \
        noexcept(noexcept(NAME##Impl(tuple1, tuple2, std::make_index_sequence<std::tuple_size_v<Tuple1>>{}))) { \
            static_assert(std::tuple_size_v<Tuple1> == std::tuple_size_v<Tuple2>); \
            return NAME##Impl(tuple1, tuple2, std::make_index_sequence<std::tuple_size_v<Tuple1>>{}); \
        }

#define BINARY_TUPLE_FOR_EACH_FOLD(SIGNATURE, OPERATION, COMBINATOR, NAME) BINARY_TUPLE_FOR_EACH(SIGNATURE, ( (OPERATION) COMBINATOR ...), NAME)

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

namespace iterators {
    namespace impl {
        template<typename ...Ts>
        struct ReferenceTuple;

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
            struct references{};

            template<typename ...Ts>
            struct references<std::tuple<Ts...>> {
                using type = ReferenceTuple<dereference_t<Ts>...>;
            };

            template<typename T>
            using references_t = typename references<T>::type;

            template<typename T, typename = std::void_t<>>
            struct has_iterator_traits {
                static constexpr bool value = false;
            };

            template<typename T>
            struct has_iterator_traits<T, std::void_t<typename std::iterator_traits<T>::value_type,
                    typename std::iterator_traits<T>::reference,
                    typename std::iterator_traits<T>::iterator_category,
                    typename std::iterator_traits<T>::difference_type>> {
                static constexpr bool value = true;
            };

            template<typename T>
            constexpr inline bool has_iterator_traits_v = has_iterator_traits<T>::value;

            template<typename T, bool B>
            struct iterator_value {
                using type = std::remove_reference_t<dereference_t<T>>;
            };

            template<typename T>
            struct iterator_value<T, true> {
                using type = typename std::iterator_traits<T>::value_type;
            };

            template<typename T>
            using iterator_value_t = typename iterator_value<T, has_iterator_traits_v<T>>::type;

            template<typename T>
            struct values {};

            template<typename ...Ts>
            struct values<std::tuple<Ts...>> {
                using type = ReferenceTuple<iterator_value_t<Ts>...>;
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

        template<typename ...Ts>
        struct ReferenceTuple : public std::tuple<Ts...> {
            template<typename ...Us>
            ReferenceTuple(ReferenceTuple<Us...> &&tuple) : std::tuple<Ts...>(moveFrom(std::move(tuple))) {}

            template<typename ...Us>
            explicit ReferenceTuple(Us &&... args) : std::tuple<Ts...>(std::forward<Us>(args)...) {}

            ReferenceTuple(const ReferenceTuple &) = default;
            ReferenceTuple(ReferenceTuple &&) = default;

            template<typename ...Us>
            ReferenceTuple &
            operator=(const ReferenceTuple<Us...> &other) noexcept(noexcept(this->copyAssign(*this, other))) {
                copyAssign(*this, other);
                return *this;
            }

            template<typename ...Us>
            ReferenceTuple &
            operator=(ReferenceTuple<Us...> &&other) noexcept(noexcept(this->moveAssign(*this, other))) {
                moveAssign(*this, other);
                return *this;
            }

            ~ReferenceTuple() = default;

        private:
            template<typename Tuple, std::size_t ...Idx>
            static constexpr auto moveFromImpl(Tuple && t, std::index_sequence<Idx...>) {
                return std::make_tuple(std::move(std::get<Idx>(t))...);
            }

            template<typename ...Us>
            static constexpr auto moveFrom(ReferenceTuple<Us...> &&t) {
                static_assert(sizeof...(Ts) == sizeof...(Us));
                return moveFromImpl(t, std::make_index_sequence<sizeof...(Us)>{});
            }

            BINARY_TUPLE_FOR_EACH_FOLD(NON_CONST_SIGNATURE, ELEMENT1 = ELEMENT2, COMMA, copyAssign)
            BINARY_TUPLE_FOR_EACH_FOLD(NON_CONST_SIGNATURE, ELEMENT1 = std::move(ELEMENT2), COMMA, moveAssign)
        };

        template<typename Iterators>
        class ZipIterator : public traits::iterator_category_from_value<traits::minimum_category_v<Iterators>> {
            using ValueTuple = traits::references_t<Iterators>;
        public:
            using reference = traits::references_t<Iterators>;
            using value_type = traits::values_t<Iterators>;
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
                std::apply([](auto &&...it) { (++it, ...); }, iterators);
                return tmp;
            }

            /**
             * Decrements all underlying iterators by one. Only available if all iterators support at least
             * bidirectional access
             * @tparam B
             * @return
             */
            template<bool B = traits::is_bidirectional_v<Iterators>>
            constexpr auto operator--() noexcept(traits::is_nothrow_decrementible_v<Iterators>)
                -> std::enable_if_t<B, ZipIterator &> {
                std::apply([](auto &&...it) { (--it, ...); }, iterators);
                return *this;
            }

            /**
             * Post decrement. Decrements all underlying iterators by one. Only available if all iterators support at
             * least bidirectional access
             * @tparam B
             * @return
             */
            template<bool B = traits::is_bidirectional_v<Iterators>>
            constexpr auto operator--(int) noexcept(traits::is_nothrow_decrementible_v<Iterators>)
                -> std::enable_if_t<B, ZipIterator> {
                ZipIterator tmp = *this;
                std::apply([](auto &&...it) { (--it, ...); }, iterators);
                return tmp;
            }

            /**
             * Compound assignment increment. Increments all underlying iterators by n. Only available if all underlying
             * iterators support at least random access
             * @tparam B
             * @param n
             * @return
             */
            template<bool B = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator+=(difference_type n) noexcept(traits::is_nothrow_compound_assignable_plus_v<Iterators>)
                    -> std::enable_if_t<B, ZipIterator &> {
                std::apply([n](auto &&...it) {((it += n), ...);}, iterators);
                return *this;
            }

            /**
             * Compound assignment decrement. Decrements all underlying iterators by n. Only available if all underlying
             * iterators support at least random access
             * @tparam B
             * @param n
             * @return
             */
            template<bool B = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator-=(difference_type n) noexcept(traits::is_nothrow_compound_assignable_minus_v<Iterators>)
                    -> std::enable_if_t<B, ZipIterator &> {
                std::apply([n](auto &&...it) {((it -= n), ...);}, iterators);
                return *this;
            }

            /**
             * Returns a ZipIterator where all underlying iterators are incremented by n. Only available if all
             * underlying iterators support at least random access
             * @tparam B
             * @param n
             * @return
             */
            template<bool B = traits::is_random_accessible_v<Iterators>>
            friend constexpr auto operator+(ZipIterator it, difference_type n)
                noexcept(traits::is_nothrow_compound_assignable_plus_v<Iterators>) -> std::enable_if_t<B, ZipIterator> {
                it += n;
                return it;
            }

            /**
             * Returns a ZipIterator where all underlying iterators are incremented by n. Only available if all
             * underlying iterators support at least random access
             * @tparam B
             * @param n
             * @return
             */
            template<bool B = traits::is_random_accessible_v<Iterators>>
            friend constexpr auto operator+(difference_type n, ZipIterator it)
                noexcept(traits::is_nothrow_compound_assignable_plus_v<Iterators>) -> std::enable_if_t<B, ZipIterator> {
                it += n;
                return it;
            }

            /**
             * Returns a ZipIterator where all underlying iterators are decremented by n. Only available if all
             * underlying iterators support at least random access
             * @tparam B
             * @param n
             * @return
             */
            template<bool B = traits::is_random_accessible_v<Iterators>>
            friend constexpr auto operator-(ZipIterator it, difference_type n)
            noexcept(traits::is_nothrow_compound_assignable_minus_v<Iterators>) -> std::enable_if_t<B, ZipIterator> {
                it -= n;
                return it;
            }

            /**
             * Returns the minimum pairwise difference n between all underlying iterators of *this and other, such that
             * other + n == *this
             * Only available if all underlying iterators support at least random access
             * @tparam B
             * @param other
             * @return
             */
            template<bool B = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator-(const ZipIterator &other) const
                noexcept(noexcept(ZipIterator::minDifference(std::declval<Iterators>(), other.iterators)))
                -> std::enable_if_t<B, difference_type> {
                return minDifference(iterators, other.iterators);
            }

            /**
             * Given a ZipIterator it, returns *(it + n)
             * Only available if all underlying iterators support at least random access
             * @tparam B
             * @param n
             * @return
             */
            template<bool B = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator[](difference_type n) const noexcept(noexcept(*(std::declval<ZipIterator>() + n)))
                -> std::enable_if_t<B, reference> {
                return *(*this + n);
            }

            /**
             * Returns true if all underlying iterators compare less to the corresponding iterators from other
             * Only available if all underlying iterators support at least random access
             * @tparam B
             * @param other
             * @return
             */
            template<bool B = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator<(const ZipIterator &other) const
            noexcept(noexcept(ZipIterator::allLess(std::declval<Iterators>(),
                                                   other.iterators))) -> std::enable_if_t<B, bool> {
                return allLess(iterators, other.iterators);
            }

            /**
             * Returns true if all underlying iterators compare greater to the corresponding iterators from other
             * Only available if all underlying iterators support at least random access
             * @tparam B
             * @param other
             * @return
             */
            template<bool B = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator>(const ZipIterator &other) const
            noexcept(noexcept(ZipIterator::allGreater(std::declval<Iterators>(),
                                                      other.iterators))) -> std::enable_if_t<B, bool> {
                return allGreater(iterators, other.iterators);
            }

            /**
             * Returns true if all underlying iterators compare less or equal to the corresponding iterators from
             * other. Only available if all underlying iterators support at least random access
             * @tparam B
             * @param other
             * @return
             */
            template<bool B = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator<=(const ZipIterator &other) const
            noexcept(noexcept(std::declval<ZipIterator>() > other)) -> std::enable_if_t<B, bool> {
                return !(*this > other);
            }

            /**
             * Returns true if all underlying iterators compare greater or equal to the corresponding iterators from
             * other. Only available if all underlying iterators support at least random access
             * @tparam B
             * @param other
             * @return
             */
            template<bool B = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator>=(const ZipIterator &other) const
            noexcept(noexcept(std::declval<ZipIterator>() < other)) -> std::enable_if_t<B, bool> {
                return !(*this < other);
            }

            /**
             * Returns true if at least one underlying iterator compares equal to the corresponding iterator from
             * other.
             * @tparam B
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
             * @tparam B
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
            reference operator*() const noexcept(traits::is_nothrow_dereferencible_v<Iterators>) {
                return std::apply([](auto &&...it) { return reference(*it...); }, iterators);
            }

            /**
             * Const reference to underlying iterators
             * @return
             */
            constexpr auto getIterators() const noexcept -> const Iterators& {
                return iterators;
            }

        private:
            BINARY_TUPLE_FOR_EACH_FOLD(CONST_SIGNATURE, ELEMENT1 == ELEMENT2, ||, oneEqual)

            BINARY_TUPLE_FOR_EACH_FOLD(CONST_SIGNATURE, ELEMENT1 < ELEMENT2, &&, allLess)

            BINARY_TUPLE_FOR_EACH_FOLD(CONST_SIGNATURE, ELEMENT1 > ELEMENT2, &&, allGreater)

            BINARY_TUPLE_FOR_EACH(CONST_SIGNATURE, std::min<difference_type>({ELEMENT1 - ELEMENT2 ...}), minDifference)

            Iterators iterators;
        };

        template<typename ...Iterable>
        struct ZipContainer {
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
            template<typename ...Container>
            explicit ZipContainer(Container &&...containers) : containers(std::forward<Container>(containers)...) {}


            auto begin() {
                return ZipIterator<IteratorTuple>(
                        std::apply([](auto &&...c) { return IteratorTuple(std::begin(c)...); }, containers));
            }

            auto end() {
                return ZipIterator<SentinelTuple>(
                        std::apply([](auto &&...c) { return SentinelTuple(std::end(c)...); }, containers));
            }

            auto begin() const {
                return ZipIterator<CIteratorTuple>(
                        std::apply([](auto &&...c) { return CIteratorTuple(std::begin(c)...); }, containers));
            }

            auto end() const {
                return ZipIterator<CSentinelTuple>(
                        std::apply([](auto &&...c) { return CSentinelTuple(std::end(c)...); }, containers));
            }

        private:
            ContainerTuple containers;
        };

        struct Unreachable {};

        template<typename T>
        struct CounterIterator {
            using value_type = T;
            using reference = T;
            using pointer = void;
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = T;
            static_assert(std::is_integral_v<T> && !std::is_floating_point_v<T>);

            explicit constexpr CounterIterator(T begin, T increment = T(1)) noexcept:
                    counter(begin), increment(increment) {}

            constexpr CounterIterator &operator++() noexcept {
                counter += increment;
                return *this;
            }

            constexpr CounterIterator operator++(int) noexcept {
                CounterIterator tmp = *this;
                ++*this;
                return tmp;
            }

            constexpr CounterIterator &operator--() noexcept {
                counter -= increment;
                return *this;
            }

            constexpr CounterIterator operator--(int) noexcept {
                CounterIterator tmp = *this;
                --*this;
                return tmp;
            }

            constexpr CounterIterator &operator+=(difference_type n) noexcept {
                counter += n * increment;
                return *this;
            }

            friend constexpr CounterIterator operator+(CounterIterator it, difference_type n) noexcept {
                it += n;
                return it;
            }

            friend constexpr CounterIterator operator+(difference_type n, CounterIterator it) noexcept {
                it += n;
                return it;
            }

            constexpr CounterIterator &operator-=(difference_type n) noexcept {
                counter -= n * increment;
                return *this;
            }

            friend constexpr CounterIterator operator-(CounterIterator it, difference_type n) noexcept {
                it -= n;
                return it;
            }

            constexpr difference_type operator-(const CounterIterator &other) const noexcept {
                return (counter - other.counter) / increment;
            }

            constexpr reference operator[](difference_type n) const noexcept {
                return counter + n * increment;
            }

            constexpr bool operator==(const CounterIterator &other) const noexcept {
                return counter == other.counter;
            }

            constexpr bool operator==(Unreachable) const noexcept {
                return false;
            }

            constexpr bool operator!=(const CounterIterator &other) const noexcept {
                return !(*this == other);
            }

            constexpr bool operator!=(Unreachable) const noexcept {
                return true;
            }

            constexpr bool operator<(const CounterIterator &other) const noexcept {
                return counter < other.counter;
            }

            constexpr bool operator<=(const CounterIterator &other) const noexcept {
                return counter <= other.counter;
            }

            constexpr bool operator>(const CounterIterator &other) const noexcept {
                return counter > other.counter;
            }

            constexpr bool operator>=(const CounterIterator &other) const noexcept {
                return counter >= other.counter;
            }

            constexpr bool operator<(Unreachable) const noexcept {
                return true;
            }

            constexpr bool operator<=(Unreachable) const noexcept {
                return true;
            }

            constexpr bool operator>(Unreachable) const noexcept {
                return false;
            }

            constexpr bool operator>=(Unreachable) const noexcept {
                return false;
            }

            constexpr T operator*() const noexcept {
                return counter;
            }

        private:
            T counter;
            T increment;
        };

        template<typename T = std::size_t>
        struct CounterContainer {
            explicit constexpr CounterContainer(T start, T increment) noexcept: start(start), increment(increment) {}

            [[nodiscard]] CounterIterator<T> begin() const noexcept {
                return CounterIterator<T>(start, increment);
            }

            [[nodiscard]] static constexpr Unreachable end() noexcept {
                return Unreachable{};
            }

        private:
            T start;
            T increment;
        };
    }

    /**
     * Function that is used to create a ZipIterator from an arbitrary number of iterators
     * @tparam Iterators type of iterators
     * @param iterators arbitrary number of iterators
     * @return ZipIterator
     * @note ZipIterators have the same iterator category as the least powerful underlying operator. This means that
     * for example, zipping a random access iterator and a bidirectional iterator only yields a bidirectional
     * ZipIterator
     */
    template<typename ...Iterators, std::enable_if_t<(impl::traits::is_dereferencible_v<Iterators> && ...), int> = 0>
    constexpr auto zip(Iterators ...iterators) -> impl::ZipIterator<std::tuple<Iterators...>> {
        return impl::ZipIterator<std::tuple<Iterators...>>(std::move(iterators)...);
    }

    /**
     * Function that can be used in range based loops to emulate the zip iterator from python.
     * As in python: if the passed containers have different lengths, the container with the least items decides
     * the overall range
     * @tparam Iterable Container types that support iteration
     * @param iterable Arbitrary number of containers
     * @return zip-container class that provides begin and end members to be used in range based for-loops
     */
    template<typename ...Iterable, std::enable_if_t<(impl::traits::is_container_v<Iterable> && ...), int> = 0>
    constexpr auto zip(Iterable &&...iterable) {
        return impl::ZipContainer<Iterable...>(std::forward<Iterable>(iterable)...);
    }

    /**
     * Zip variant that does not allow manipulation of the container elements
     * @tparam Iterable Container types that support iteration
     * @param iterable Arbitrary number of containers
     * @return zip-container class that provides begin and end members to be used in range based for-loops.
     */
    template<typename ...Iterable>
    constexpr auto const_zip(Iterable &&...iterable) {
        return impl::ZipContainer<impl::traits::reference_if_t<std::is_lvalue_reference_v<Iterable>,
                std::add_const_t<std::remove_reference_t<Iterable>>>...>(std::forward<Iterable>(iterable)...);
    }

    /**
     * Function that can be used in range based loops to emulate the enumerate iterator from python.
     * @tparam Container Container type that supports iteration
     * @tparam T type of enumerate counter (default std::size_t)
     * @param container Source container
     * @param start Optional index offset (default 0)
     * @param increment Optional index increment (default 1)
     * @return zip-container class that provides begin and end members to be used in range based for-loops.
     */
    template<typename Container, typename T = std::size_t>
    constexpr auto enumerate(Container &&container, T start = T(0), T increment = T(1)) {
        return zip(impl::CounterContainer(start, increment), std::forward<Container>(container));
    }

    /**
     * enumerate variant that does not allow manipulation of the container elements
     * @tparam Container Container type that supports iteration
     * @tparam T type of enumerate counter (default std::size_t)
     * @param container Source container
     * @param start Optional index offset (default 0)
     * @param increment Optional index increment (default 1)
     * @return zip-container class that provides begin and end members to be used in range based for-loops.
     */
    template<typename Container, typename T = std::size_t>
    constexpr auto const_enumerate(Container &&container, T start = T(0), T increment = T(1)) {
        return const_zip(impl::CounterContainer(start, increment), std::forward<Container>(container));
    }

}

namespace std {
    template<typename ...Ts>
    struct tuple_size<iterators::impl::ReferenceTuple<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

    template<std::size_t Index, typename ...Ts>
    struct tuple_element<Index, iterators::impl::ReferenceTuple<Ts...>> : tuple_element<Index, tuple < Ts...>> {};

    template<typename ...Ts>
    void swap(iterators::impl::ReferenceTuple<Ts...> a, iterators::impl::ReferenceTuple<Ts...> b) {
        iterators::impl::ReferenceTuple<std::remove_reference_t<Ts>...> tmp(std::move(a));
        a = std::move(b);
        b = std::move(tmp);
    }
}

#endif //ITERATORTOOLS_ITERATORS_HPP
