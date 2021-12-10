/**
 * @author tim Luchterhand
 * @date 10.09.21
 * @brief This file contains the definitions of Python-like zip- and enumerate-functions. They can be used in range
 * based for-loops to loop over multiple ranges at the same time, or to index a range while looping respectively.
 */

#ifndef ITERATORTOOLS_ITERATORS_HPP
#define ITERATORTOOLS_ITERATORS_HPP

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

#define ELEM std::get<Idx>(tuple)

#define TUPLE_FOR_EACH(OPERATION, COMBINATOR, NAME) \
        template<typename Tuple, std::size_t ...Idx> \
        static constexpr auto NAME##Impl(Tuple &tuple, std::index_sequence<Idx...>) \
        noexcept() { \
            return (OPERATIRON COMBINATOR ...); \
        } \
        template<typename Tuple> \
        static constexpr auto NAME(Tuple &tuple) \
        noexcept(noexcept(tupleForEachImpl(tuple, std::make_index_sequence<std::tuple_size_v<Tuple>>{}))) { \
            return NAME##Impl(tuple, std::make_index_sequence<std::tuple_size_v<Tuple>>{}); \
        }


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
        template<bool Cond, typename T>
        using reference_if_t = std::conditional_t<Cond, std::add_lvalue_reference_t<T>, T>;

        template<bool Cond, typename T>
        using const_if_t = std::conditional_t<Cond, std::add_const_t<T>, T>;

        template<typename T>
        std::true_type
        container_test(decltype(std::begin(std::declval<T>()), std::end(std::declval<T>()), std::declval<T>()));

        template<typename T>
        std::false_type container_test(...);

        template<typename T>
        struct is_container : decltype(container_test<T>(std::declval<T>())) {};

        template<typename T>
        constexpr inline bool is_container_v = is_container<T>::value;

        template<typename T>
        std::true_type deref_test(decltype(*std::declval<T>(), std::declval<T>()));

        template<typename T>
        std::false_type deref_test(...);

        template<typename T>
        struct dereferencible : decltype(deref_test<T>(std::declval<T>())) {};

        template<typename T>
        constexpr inline bool dereferencible_v = dereferencible<T>::value;

        struct empty {};

        template<typename T, bool B>
        struct dereference {
            using type = empty;
        };

        template<typename T>
        struct dereference<T, true> {
            using type = decltype(*std::declval<T>());
        };

        template<typename T>
        using dereference_t = typename dereference<T, dereferencible_v<T>>::type;

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

        template<typename Iterators>
        class ZipIterator : public iterator_category_from_value<minimum_category_v<Iterators>> {
            using ValueTuple = impl::values_t<Iterators>;
        public:
            using value_type = ValueTuple;
            using reference = value_type;
            using pointer = void;
            using difference_type = std::ptrdiff_t;

            explicit constexpr ZipIterator(
                    const Iterators &iterators) noexcept(std::is_nothrow_copy_constructible_v<Iterators>)
                    : iterators(iterators) {}

            template<typename ...Its>
            explicit constexpr ZipIterator(Its &&...its) : iterators(std::tuple(std::forward<Its>(its)...)) {}

            constexpr ZipIterator &operator++() noexcept(is_nothrow_incrementible_v<Iterators>) {
                std::apply([](auto &&...it) { (++it, ...); }, iterators);
                return *this;
            }

            constexpr ZipIterator operator++(int) noexcept(is_nothrow_incrementible_v<Iterators>) {
                ZipIterator tmp = *this;
                std::apply([](auto &&...it) { (++it, ...); }, iterators);
                return tmp;
            }

            template<bool B = is_bidirectional_v<Iterators>>
            constexpr auto operator--() noexcept(is_nothrow_decrementible_v<Iterators>)
                -> std::enable_if_t<B, ZipIterator &> {
                std::apply([](auto &&...it) { (--it, ...); }, iterators);
                return *this;
            }

            template<bool B = is_bidirectional_v<Iterators>>
            constexpr auto operator--(int) noexcept(is_nothrow_decrementible_v<Iterators>)
                -> std::enable_if_t<B, ZipIterator> {
                ZipIterator tmp = *this;
                std::apply([](auto &&...it) { (--it, ...); }, iterators);
                return tmp;
            }

            template<bool B = is_random_accessible_v<Iterators>>
            constexpr auto operator+=(difference_type n) noexcept(is_nothrow_compound_assignable_plus_v<Iterators>)
                    -> std::enable_if_t<B, ZipIterator &> {
                tupleForEach(iterators, [n](auto &it) constexpr noexcept(noexcept(it += n)) {it += n;});
                return *this;
            }

            template<bool B = is_random_accessible_v<Iterators>>
            constexpr auto operator-=(difference_type n) noexcept(is_nothrow_compound_assignable_minus_v<Iterators>)
                    -> std::enable_if_t<B, ZipIterator &> {
                tupleForEach(iterators, [n](auto &it) constexpr noexcept(noexcept(it -= n)) {it -= n;});
                return *this;
            }

            template<bool B = is_random_accessible_v<Iterators>>
            friend constexpr auto operator+(ZipIterator it, difference_type n)
                noexcept(is_nothrow_compound_assignable_plus_v<Iterators>) -> std::enable_if_t<B, ZipIterator> {
                it += n;
                return it;
            }

            template<bool B = is_random_accessible_v<Iterators>>
            friend constexpr auto operator+(difference_type n, ZipIterator it)
                noexcept(is_nothrow_compound_assignable_plus_v<Iterators>) -> std::enable_if_t<B, ZipIterator> {
                it += n;
                return it;
            }

            template<bool B = is_random_accessible_v<Iterators>>
            friend constexpr auto operator-(ZipIterator it, difference_type n)
            noexcept(is_nothrow_compound_assignable_minus_v<Iterators>) -> std::enable_if_t<B, ZipIterator> {
                it -= n;
                return it;
            }

            template<bool B = is_random_accessible_v<Iterators>>
            constexpr auto operator-(const ZipIterator &other) const
                noexcept(noexcept(std::get<0>(this->iterators) - std::get<0>(other.iterators))) -> std::enable_if_t<B, difference_type> {
                return std::get<0>(iterators) - std::get<0>(other.iterators);
            }

            template<bool B = is_random_accessible_v<Iterators>>
            constexpr auto operator[](difference_type n) const noexcept(noexcept(*(*this + n)))
                -> std::enable_if_t<B, reference> {
                return *(*this + n);
            }

            /*template<bool B = is_random_accessible_v<Iterators>>
            constexpr auto operator<(const ZipIterator &other) const noexcept() -> std::enable_if_t<B, bool> {
            }*/

            template<typename Its>
            constexpr bool operator==(const ZipIterator<Its> &other) const
            noexcept(noexcept(this->oneEqual(this->iterators, other.getIterators()))) {
                return oneEqual(iterators, other.getIterators());
            }

            template<typename Its>
            constexpr bool operator!=(const ZipIterator<Its> &other) const noexcept(noexcept(*this == other)) {
                return !(*this == other);
            }

            auto operator*() const noexcept(impl::is_nothrow_dereferencible_v<Iterators>) {
                return std::apply([](auto &&...it) { return ValueTuple(*it...); }, iterators);
            }

            constexpr auto getIterators() const noexcept -> const Iterators& {
                return iterators;
            }

        private:
            template<typename Tuple1, typename Tuple2, std::size_t ...Idx>
            static constexpr bool
            oneEqualImpl(const Tuple1 &t1, const Tuple2 &t2, std::index_sequence<Idx...>) noexcept((noexcept(
                    std::get<Idx>(t1) == std::get<Idx>(t2)) && ...)) {
                return (... || (std::get<Idx>(t1) == std::get<Idx>(t2)));
            }

            template<typename Tuple1, typename Tuple2>
            static constexpr bool oneEqual(const Tuple1 &t1, const Tuple2 &t2)
            noexcept(noexcept(oneEqualImpl(t1, t2, std::make_index_sequence<std::tuple_size_v<Tuple1>>{}))) {
                static_assert(std::tuple_size_v<Tuple1> == std::tuple_size_v<Tuple2>);
                return oneEqualImpl(t1, t2, std::make_index_sequence<std::tuple_size_v<Tuple1>>{});
            }

            template<typename Tuple, typename Fun, std::size_t ...Idx>
            static constexpr auto tupleForEachImpl(Tuple &tuple, const Fun &f, std::index_sequence<Idx...>)
                noexcept((std::is_nothrow_invocable_v<Fun, decltype(std::get<Idx>(tuple))> && ...)) {
                return (f(std::get<Idx>(tuple)), ...);
            }

            template<typename Fun, typename Tuple>
            static constexpr auto tupleForEach(Tuple &tuple, const Fun &f)
                noexcept(noexcept(tupleForEachImpl(tuple, f, std::make_index_sequence<std::tuple_size_v<Tuple>>{}))) {
                return tupleForEachImpl(tuple, f, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
            }

            Iterators iterators;
        };

        template<typename ...Iterable>
        struct ZipContainer {
        private:
            using ContainerTuple = std::tuple<Iterable...>;
            template<bool Const>
            using Iterators = std::tuple<decltype(std::begin(
                    std::declval<std::add_lvalue_reference_t<const_if_t<Const, std::remove_reference_t<Iterable>>>>()))...>;
            template<bool Const>
            using Sentinels = std::tuple<decltype(std::end(
                    std::declval<std::add_lvalue_reference_t<const_if_t<Const, std::remove_reference_t<Iterable>>>>()))...>;
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

            constexpr bool operator<(const CounterIterator &other) noexcept {
                return counter < other.counter;
            }

            constexpr bool operator<=(const CounterIterator &other) noexcept {
                return counter <= other.counter;
            }

            constexpr bool operator>(const CounterIterator &other) noexcept {
                return counter > other.counter;
            }

            constexpr bool operator>=(const CounterIterator &other) noexcept {
                return counter >= other.counter;
            }

            constexpr bool operator<(Unreachable) noexcept {
                return true;
            }

            constexpr bool operator<=(Unreachable) noexcept {
                return true;
            }

            constexpr bool operator>(Unreachable) noexcept {
                return false;
            }

            constexpr bool operator>=(Unreachable) noexcept {
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
     */
    template<typename ...Iterators, std::enable_if_t<(impl::dereferencible_v<Iterators> && ...), int> = 0>
    auto zip(Iterators ...iterators) -> impl::ZipIterator<std::tuple<Iterators...>> {
        return impl::ZipIterator<std::tuple<Iterators...>>(iterators...);
    }

    /**
     * Function that can be used in range based loops to emulate the zip iterator from python.
     * As in python: if the passed containers have different lengths, the container with the least items decides
     * the overall range
     * @tparam Iterable Container types that support iteration
     * @param iterable Arbitrary number of containers
     * @return zip-container class that provides begin and end members to be used in range based for-loops
     */
    template<typename ...Iterable, std::enable_if_t<(impl::is_container_v<Iterable> && ...), int> = 0>
    auto zip(Iterable &&...iterable) {
        return impl::ZipContainer<Iterable...>(std::forward<Iterable>(iterable)...);
    }

    /**
     * Zip variant that does not allow manipulation of the container elements
     * @tparam Iterable Container types that support iteration
     * @param iterable Arbitrary number of containers
     * @return zip-container class that provides begin and end members to be used in range based for-loops.
     */
    template<typename ...Iterable>
    auto const_zip(Iterable &&...iterable) {
        return impl::ZipContainer<impl::reference_if_t<std::is_lvalue_reference_v<Iterable>,
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
    auto enumerate(Container &&container, T start = T(0), T increment = T(1)) {
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
    auto const_enumerate(Container &&container, T start = T(0), T increment = T(1)) {
        return const_zip(impl::CounterContainer(start, increment), std::forward<Container>(container));
    }

}

#endif //ITERATORTOOLS_ITERATORS_HPP
