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
#include <memory>
#include <iterator>

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
        noexcept(noexcept((OPERATION))) -> decltype(OPERATION) { \
            return (OPERATION); \
        } \
        template<typename Tuple1, typename Tuple2> \
        static constexpr auto NAME(const Tuple1 &tuple1, const Tuple2 &tuple2) \
        noexcept(noexcept(NAME##Impl(tuple1, tuple2, std::make_index_sequence<std::tuple_size_v<Tuple1>>{}))) \
        -> decltype(NAME##Impl(tuple1, tuple2, std::make_index_sequence<std::tuple_size_v<Tuple1>>{})) { \
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

#define INSTANCE_OF(TYPENAME) std::declval<TYPENAME>()

#define INSTANCE_OF_IMPL INSTANCE_OF(Implementation)

#define REQUIRES_IMPL(TYPENAME, EXPRESSION) typename Implementation = TYPENAME, typename = std::void_t<decltype(EXPRESSION)>

#define REQUIRES(EXPRESSION) typename = std::void_t<decltype(EXPRESSION)>

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

            template<typename ...Ts>
            struct is_dereferencible<std::tuple<Ts...>, void> {
                static constexpr bool value = (is_dereferencible<Ts>::value && ...);
            };

            template<typename T>
            constexpr inline bool is_dereferencible_v = is_dereferencible<T>::value;

            template<typename T, typename = std::void_t<>>
            struct is_incrementable : std::false_type {};

            template<typename T>
            struct is_incrementable<T, std::void_t<decltype(++REFERENCE(T))>> : std::true_type {};

            template<typename ...Ts>
            struct is_incrementable<std::tuple<Ts...>, void> {
                static constexpr bool value = (is_incrementable<Ts>::value && ...);
            };

            template<typename T>
            constexpr inline bool is_incrementable_v = is_incrementable<T>::value;

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

        template<typename T, typename = std::void_t<>>
        struct get_difference_type {
            using type = std::ptrdiff_t;
        };

        template<typename T>
        struct get_difference_type<T, std::void_t<typename std::iterator_traits<T>::difference_type>> {
            using type = typename std::iterator_traits<T>::difference_type;
        };

        /**
         * @brief CRTP-class that provides additional pointer arithmetic operators synthesized from basic operators
         * @details @copybrief
         * Adds the following operators
         * - postfix increment and decrement (requires the respective prefix operators)
         * - array subscript operator[] (requires operator+ and dereference operator)
         * - binary arithmetic operators (requires compound assignment operators)
         * - inequality comparison (requires operator==)
         * - less than or equal comparison (requires operator>)
         * - greater than or equal comparison (requires operator<)
         * @tparam Impl Base class
         */
        template<typename Impl>
        struct SynthesizedOperators {

            /**
             * Array subscript operator
             * @tparam Implementation SFINAE helper, do not specify explicitly
             * @param n index
             * @return *(*this + n)
             */
            template<REQUIRES_IMPL(Impl, *(INSTANCE_OF_IMPL + INSTANCE_OF(typename Implementation::difference_type)))>
            constexpr auto operator[](typename Implementation::difference_type n) const
            noexcept(noexcept(*(std::declval<Impl>() + n))) {
                return *(this->getImpl() + n);
            }

            /**
             * Postfix increment. Synthesized from prefix increment
             * @tparam Implementation SFINAE helper, do not specify explicitly
             * @return Instance of Impl
             */
            template<REQUIRES_IMPL(Impl, ++INSTANCE_OF_IMPL)>
            constexpr Impl operator++(int)
            noexcept(noexcept(++std::declval<Impl>()) && std::is_nothrow_copy_constructible_v<Impl>) {
                auto tmp = this->getImpl();
                this->getImpl().operator++();
                return tmp;
            }

            /**
             * Postfix decrement. Synthesized from prefix decrement
             * @tparam Implementation SFINAE helper, do not specify explicitly
             * @return Instance of Impl
             */
            template<REQUIRES_IMPL(Impl, --INSTANCE_OF_IMPL)>
            constexpr Impl operator--(int)
            noexcept(noexcept(--std::declval<Impl>()) && std::is_nothrow_copy_constructible_v<Impl>) {
                auto tmp = this->getImpl();
                this->getImpl().operator--();
                return tmp;
            }

            /**
             * Binary +plus operator. Synthesized from compound assignment operator+=
             * @tparam Implementation SFINAE helper, do not specify explicitly
             * @param it left hand side
             * @param n right hand side
             * @return Instance of Impl
             */
            template<REQUIRES_IMPL(Impl, INSTANCE_OF_IMPL += INSTANCE_OF(typename Implementation::difference_type)) >
            friend constexpr auto operator+(Impl it, typename Implementation::difference_type n)
            noexcept(noexcept(std::declval<Impl>() += n)) {
                it += n;
                return it;
            }

            /**
             * Binary +plus operator. Synthesized from compound assignment operator+=
             * @tparam Implementation SFINAE helper, do not specify explicitly
             * @param n left hand side
             * @param it right hand side
             * @return Instance of Impl
             */
            template<REQUIRES_IMPL(Impl, INSTANCE_OF_IMPL += INSTANCE_OF(typename Implementation::difference_type))>
            friend constexpr auto operator+(typename Implementation::difference_type n, Impl it)
            noexcept(noexcept(std::declval<Impl>() += n)) {
                it += n;
                return it;
            }

            /**
             * Binary minus operator. Synthesized from compound assignment operator-=
             * @tparam Implementation SFINAE helper, do not specify explicitly
             * @param it left hand side
             * @param n right hand side
             * @return Instance of Impl
             */
            template<REQUIRES_IMPL(Impl, INSTANCE_OF_IMPL -= INSTANCE_OF(typename Implementation::difference_type)) >
            friend constexpr auto operator-(Impl it, typename Implementation::difference_type n)
            noexcept(noexcept(std::declval<Impl>() -= n)) {
                it -= n;
                return it;
            }

            /**
             * Inequality comparison
             * @tparam T type of right hand side
             * @tparam Implementation SFINAE helper, do not specify explicitly
             * @param other right hand side
             * @return true if this is not equal to other
             */
            template<typename T, REQUIRES_IMPL(Impl, INSTANCE_OF_IMPL == INSTANCE_OF(T))>
            constexpr bool operator!=(const T &other) const noexcept(noexcept(INSTANCE_OF_IMPL == other)) {
                return !(this->getImpl()== other);
            }

            /**
             * Less than or equal comparison
             * @tparam T type of right hand side
             * @tparam Implementation SFINAE helper, do not specify explicitly
             * @param other right hand side
             * @return true if this is not greater than other
             */
            template<typename T, REQUIRES_IMPL(Impl, INSTANCE_OF_IMPL > INSTANCE_OF(T))>
            constexpr bool operator<=(const T& rhs) const noexcept(noexcept(INSTANCE_OF_IMPL > rhs)) {
                return !(this->getImpl()> rhs);
            }

            /**
             * Greater than or equal comparison
             * @tparam T type of right hand side
             * @tparam Implementation SFINAE helper, do not specify explicitly
             * @param other right hand side
             * @return true if this is not less than other
             */
            template<typename T, REQUIRES_IMPL(Impl, INSTANCE_OF_IMPL < INSTANCE_OF(T))>
            constexpr bool operator>=(const T& rhs) const noexcept(noexcept(INSTANCE_OF_IMPL < rhs)) {
                return !(this->getImpl() < rhs);
            }

        private:
            constexpr Impl &getImpl() noexcept {
                return static_cast<Impl &>(*this);
            }

            constexpr const Impl &getImpl() const noexcept {
                return static_cast<const Impl &>(*this);
            }

            SynthesizedOperators() = default;
            friend Impl;
        };

        /**
         * @brief Class combining multiple iterators into one. Use it to iterate over multiple ranges at the same time.
         * @details @copybrief
         * ZipIterators only support the operators of the least powerful underling iterator. Zipping a random access
         * iterator (e.g. from std::vector) and a bidirectional iterator (e.g. from std::list) results in a
         * bidirectional iterator. All operators are SFINAE friendly.
         *
         * ZipIterators return a tuple of references to the range elements. When using
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
        class ZipIterator
                : public traits::iterator_category_from_value<traits::minimum_category_v<Iterators>>,
                  public SynthesizedOperators<ZipIterator<Iterators>> {

        public:
            using value_type = traits::values_t<Iterators>;
            using reference = value_type;
            using pointer = void;
            using difference_type = std::ptrdiff_t;

        private:
            BINARY_TUPLE_FOR_EACH_FOLD(ELEMENT1 == ELEMENT2, ||, oneEqual)
            BINARY_TUPLE_FOR_EACH_FOLD(ELEMENT1 < ELEMENT2, &&, allLess)
            BINARY_TUPLE_FOR_EACH_FOLD(ELEMENT1 > ELEMENT2, &&, allGreater)
            BINARY_TUPLE_FOR_EACH(std::min<difference_type>({ELEMENT1 - ELEMENT2 ...}), minDifference)
            Iterators iterators;

        public:
            using SynthesizedOperators<ZipIterator>::operator++;
            using SynthesizedOperators<ZipIterator>::operator--;

            explicit constexpr ZipIterator(
                    const Iterators &iterators) noexcept(std::is_nothrow_copy_constructible_v<Iterators>)
                    : iterators(iterators) {}

            template<typename ...Its>
            explicit constexpr ZipIterator(Its &&...its) : iterators(std::forward<Its>(its)...) {}

            /**
             * Increments all underlying iterators by one
             * @tparam Its SFINAE guard, do not specify
             * @return reference to this
             */
            template<typename Its = Iterators, typename = std::enable_if_t<traits::is_incrementable_v<Its>>>
            constexpr ZipIterator &operator++() noexcept(traits::is_nothrow_incrementible_v<Iterators>) {
                std::apply([](auto &&...it) { (++it, ...); }, iterators);
                return *this;
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
             * @tparam IsBidirectional SFINAE guard, do not specify
             * @return reference to this
             */
            template<bool IsBidirectional = traits::is_bidirectional_v<Iterators>>
            constexpr auto operator--() noexcept(traits::is_nothrow_decrementible_v<Iterators>)
                -> std::enable_if_t<IsBidirectional, ZipIterator &> {
                std::apply([](auto &&...it) { (--it, ...); }, iterators);
                return *this;
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
             * @tparam IsRandomAccessible SFINAE guard, do not specify
             * @param n increment
             * @return reference to this
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
             * @tparam IsRandomAccessible SFINAE guard, do not specify
             * @param n decrement
             * @return reference to this
             */
            template<bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>>
            constexpr auto operator-=(difference_type n) noexcept(traits::is_nothrow_compound_assignable_minus_v<Iterators>)
                    -> std::enable_if_t<IsRandomAccessible, ZipIterator &> {
                std::apply([n](auto &&...it) {((it -= n), ...);}, iterators);
                return *this;
            }

            /**
             * Returns the minimum pairwise difference n between all underlying iterators of *this and other, such that
             * (other + n) == *this
             * Only available if all underlying iterators support at least random access
             * @tparam Its Iterator types of right hand side
             * @tparam IsRandomAccessible SFINAE guard, do not specify
             * @param other right hand side
             * @return integer n such that (other + n) == *this
             */
            template<typename Its, bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>, REQUIRES(
                    ZipIterator::minDifference(INSTANCE_OF(Iterators), INSTANCE_OF(Its)))>
            constexpr auto operator-(const ZipIterator<Its> &other) const
            -> std::enable_if_t<IsRandomAccessible, difference_type> {
                return minDifference(iterators, other.getIterators());
            }

            /**
             * Pairwise less comparison of underlying iterators
             * Only available if all underlying iterators support at least random access
             * @tparam Its Iterator types of right hand side
             * @tparam IsRandomAccessible SFINAE guard, do not specify
             * @param other right hand side
             * @return true if all underlying iterators compare less to the corresponding iterators from other
             */
            template<typename Its, bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>, REQUIRES(
                    ZipIterator::allLess(INSTANCE_OF(Iterators), INSTANCE_OF(Its)))>
            constexpr auto operator<(const ZipIterator<Its> &other) const
            noexcept(noexcept(ZipIterator::allLess(INSTANCE_OF(Iterators), INSTANCE_OF(Its))))
            -> std::enable_if_t<IsRandomAccessible, bool> {
                return allLess(iterators, other.getIterators());
            }

            /**
             * Pairwise grater comparison of underlying iterators
             * Only available if all underlying iterators support at least random access
             * @tparam Its Iterator types of right hand side
             * @tparam IsRandomAccessible SFINAE guard, do not specify
             * @param other right hand side
             * @return true if all underlying iterators compare greater to the corresponding iterators from other
             */
            template<typename Its, bool IsRandomAccessible = traits::is_random_accessible_v<Iterators>, REQUIRES(
                    ZipIterator::allGreater(INSTANCE_OF(Iterators), INSTANCE_OF(Its))) >
            constexpr auto operator>(const ZipIterator<Its> &other) const noexcept(noexcept(ZipIterator::allGreater(
                    INSTANCE_OF(Iterators), INSTANCE_OF(Its)))) -> std::enable_if_t<IsRandomAccessible, bool> {
                return allGreater(iterators, other.getIterators());
            }

            ///@}

            /**
             * Pairwise equality comparison of underlying iterators
             * @tparam Its Iterator types of right hand side
             * @param other right hand side
             * @return true if at least one underlying iterator compares equal to the corresponding iterator from other
             */
            template<typename Its, REQUIRES(ZipIterator::oneEqual(INSTANCE_OF(Iterators), INSTANCE_OF(Its)))>
            constexpr bool operator==(const ZipIterator<Its> &other) const
            noexcept(noexcept(ZipIterator::oneEqual(std::declval<Iterators>(), other.getIterators()))) {
                return oneEqual(iterators, other.getIterators());
            }

            /**
             * Dereferences all underlying iterators and returns a tuple of the resulting range reference types
             * @tparam Its SFINAE guard, do not specify
             * @return tuple of references to range elements
             */
            template<typename Its = Iterators, typename = std::enable_if_t<traits::is_dereferencible_v<Its>>>
            constexpr auto operator*() const noexcept(traits::is_nothrow_dereferencible_v<Iterators>) {
                return std::apply([](auto &&...it) { return value_type(*it...); }, iterators);
            }

            /**
             * Getter for underlying iterators
             * @return Const reference to underlying iterators
             */
            constexpr auto getIterators() const noexcept -> const Iterators& {
                return iterators;
            }
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

        /**
         * Signum function
         * @tparam T arbitrary scalar type
         * @param val function argument
         * @return +1 if val >= 0, -1 else
         */
        template<typename T>
        constexpr T sgn(T val) noexcept {
            return val < 0 ? T(-1) : T(1);
        }

        /**
         * @brief Iterator of an infinite sequence of numbers. Simply increments an internal counter
         * @tparam Type of the counter (most of the time this is ```std::size_t```)
         */
        template<typename T>
        struct CounterIterator : public SynthesizedOperators<CounterIterator<T>> {
            using value_type = T;
            using reference = T;
            using pointer = void;
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            static_assert(std::is_integral_v<T> && !std::is_floating_point_v<T>);

            using SynthesizedOperators<CounterIterator<T>>::operator++;
            using SynthesizedOperators<CounterIterator<T>>::operator--;

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
             * @return reference to this
             */
            constexpr CounterIterator &operator++() noexcept {
                counter += increment;
                return *this;
            }

            /**
             * Decrements value by increment
             * @return reference to this
             */
            constexpr CounterIterator &operator--() noexcept {
                counter -= increment;
                return *this;
            }

            /**
             * Compound assignment increment. Increments value by n times increment
             * @param n number of steps
             * @return reference to this
             */
            constexpr CounterIterator &operator+=(difference_type n) noexcept {
                counter += n * increment;
                return *this;
            }

            /**
             * Compound assignment decrement. Increments value by n times increment
             * @param n number of steps
             * @return reference to this
             */
            constexpr CounterIterator &operator-=(difference_type n) noexcept {
                counter -= n * increment;
                return *this;
            }

            /**
             * Difference between two CounterIterators
             * @param other right hand side
             * @return integer ```n``` with the smallest possible absolute value such that ```other + n <= *this```
             * @note When other has the same increment as ```*this```, then the returned value is guaranteed to
             * fulfil ```other + n == *this```. In the following example, this is not the case:
             * ```
             * CounterIterator a(8, 1);
             * CounterIterator b(4, 3);
             * auto diff = a - b; // yields 1 since b + 1 <= a
             * ```
             */
            constexpr difference_type operator-(const CounterIterator &other) const noexcept {
                return static_cast<difference_type>((counter - other.counter) / other.increment);
            }

            /**
             * Equality comparison.
             * @param other right hand side
             * @return true if counter of left and right hand side are equal
             */
            constexpr bool operator==(const CounterIterator &other) const noexcept {
                return counter == other.counter;
            }

            /**
             * Equality comparison with Unreachable sentinel
             * @return false
             */
            friend constexpr bool operator==(const CounterIterator &, Unreachable) noexcept {
                return false;
            }

            /**
             * @copydoc operator==(const CounterIterator &, Unreachable)
             */
            friend constexpr bool operator==(Unreachable, const CounterIterator &) noexcept {
                return false;
            }

            friend constexpr bool operator!=(Unreachable, const CounterIterator &) noexcept {
                return true;
            }

            /**
             * Less comparison of internal counters with respect to increment of this instance
             * @param other right hand side
             * @return true if
             * ```
             * sgn(increment) **this < *other sgn(increment)
             * ```
             * where ```sgn``` is the signum
             * function
             * @note If increment is negative then both sides of the inequality are multiplied with -1.
             * For example: let ```it1 = 5``` and ```it2 = -2``` be two CounterIterators where ```it1``` has negative
             * increment. Then ```it1 < it2``` is true.
             */
            constexpr bool operator<(const CounterIterator &other) const noexcept {
                return sgn(increment) *  counter < sgn(increment) * other.counter;
            }

            /**
             * Greater comparison of internal counters with respect to increment of this instance
             * @param other right hand side
             * @return true if
             * ```
             * sgn(increment) **this > *other sgn(increment)
             * ```
             * where ```sgn``` is the signum
             * @note If increment is negative then both sides of the inequality are multiplied with -1.
             * For example: let ```it1 = 5``` and ```it2 = -2``` be two CounterIterators where ```it1``` has negative
             * increment. Then ```it1 > it2``` is false.
             */
            constexpr bool operator>(const CounterIterator &other) const noexcept {
                return sgn(increment) * counter > sgn(increment) * other.counter;
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

        template<typename Iterator, typename Function>
        class TransformIterator
                : public traits::iterator_category_from_value<traits::iterator_category_value<Iterator>::value> {
            static_assert(std::is_copy_constructible_v<Function>, "Function object must be copyable");
            using Element = traits::dereference_t<Iterator>;
            static_assert(std::is_invocable_v<Function, Element>,
                          "Function object is not callable with container element type");
            using FPtr = std::shared_ptr<Function>;
        public:
            using reference = std::invoke_result_t<Function, Element>;
            using value_type = std::remove_cv_t<std::remove_reference_t<reference>>;
            using pointer = std::conditional_t<std::is_lvalue_reference_v<reference>,
                    std::add_pointer_t<std::remove_reference_t<reference>>, void>;
            using difference_type = typename get_difference_type<Iterator>::type;

            template<typename F>
            TransformIterator(const Iterator &iterator, F &&func) : it(iterator),
                                                                    f(std::make_shared<Function>(
                                                                            std::forward<F>(func))) {}

            TransformIterator(const Iterator &iterator,
                              FPtr func) noexcept(std::is_nothrow_copy_constructible_v<Iterator>)
                    : it(iterator), f(std::move(func)) {}

            TransformIterator &operator++() noexcept(noexcept(++this->it)) {
                ++it;
                return *this;
            }

            TransformIterator operator++(int) noexcept(noexcept(this->it++)) {
                TransformIterator tmp = *this;
                ++(*this);
                return tmp;
            }

            template<bool B = traits::is_bidirectional_v<Iterator>>
            auto operator--() noexcept(noexcept(--this->it)) -> std::enable_if_t<B, TransformIterator &> {
                --it;
                return *this;
            }

            template<bool B = traits::is_bidirectional_v<Iterator>>
            auto operator--(int) noexcept(noexcept(this->it--)) -> std::enable_if_t<B, TransformIterator> {
                TransformIterator tmp = *this;
                --(*this);
                return tmp;
            }

            template<bool RA = traits::is_random_accessible_v<Iterator>>
            constexpr auto operator+=(
                    difference_type n) noexcept(noexcept(this->it += n)) -> std::enable_if_t<RA, TransformIterator &> {
                it += n;
                return *this;
            }

            template<bool RA = traits::is_random_accessible_v<Iterator>>
            constexpr auto operator-=(
                    difference_type n) noexcept(noexcept(this->it -= n)) -> std::enable_if_t<RA, TransformIterator &> {
                it -= n;
                return *this;
            }

            template<bool RA = traits::is_random_accessible_v<Iterator>>
            friend constexpr auto operator+(TransformIterator iter, difference_type n) noexcept(noexcept(iter += n))
                -> std::enable_if_t<RA, TransformIterator> {
                iter += n;
                return iter;
            }

            template<bool RA = traits::is_random_accessible_v<Iterator>>
            friend constexpr auto operator-(TransformIterator iter, difference_type n) noexcept(noexcept(iter -= n))
                -> std::enable_if_t<RA, TransformIterator> {
                iter -= n;
                return iter;
            }

            template<bool RA = traits::is_random_accessible_v<Iterator>>
            friend constexpr auto operator+(difference_type n, TransformIterator iter) noexcept(noexcept(iter += n))
                -> std::enable_if_t<RA, TransformIterator> {
                iter += n;
                return iter;
            }

            template<bool RA = traits::is_random_accessible_v<Iterator>>
            constexpr auto operator-(const TransformIterator &other) const noexcept(noexcept(this->it - other.it))
                -> std::enable_if_t<RA, difference_type> {
                return it - other.it;
            }

            template<bool RA = traits::is_random_accessible_v<Iterator>>
            constexpr auto operator[](difference_type n) const noexcept(noexcept(*(*this + n)))
                -> std::enable_if_t<RA, reference> {
                return *(*this + n);
            }

            template<bool RA = traits::is_random_accessible_v<Iterator>>
            constexpr auto operator<(const TransformIterator &other) const noexcept(noexcept(this->it < other.it))
                -> std::enable_if_t<RA, bool> {
                return it < other.it;
            }

            template<bool RA = traits::is_random_accessible_v<Iterator>>
            constexpr auto operator>(const TransformIterator &other) const noexcept(noexcept(this->it > other.it))
                -> std::enable_if_t<RA, bool> {
                return it > other.it;
            }

            template<bool RA = traits::is_random_accessible_v<Iterator>>
            constexpr auto operator<=(const TransformIterator &other) const noexcept(noexcept(this->it <= other.it))
                -> std::enable_if_t<RA, bool> {
                return it <= other.it;
            }

            template<bool RA = traits::is_random_accessible_v<Iterator>>
            constexpr auto operator>=(const TransformIterator &other) const noexcept(noexcept(this->it >= other.it))
                -> std::enable_if_t<RA, bool> {
                return it >= other.it;
            }

            bool operator==(const TransformIterator &other) const noexcept(noexcept(this->it == this->it)) {
                return it == other.it;
            }

            bool operator!=(const TransformIterator &other) const noexcept(noexcept(*this == *this)) {
                return !(*this == other);
            }

            auto operator*() const noexcept(noexcept(*(this->it)) &&
                                            std::is_nothrow_invocable_v<Function, Element>) -> reference {
                return (*f)(*it);
            }

            template<bool ReturnsRef = std::is_lvalue_reference_v<reference>>
            auto operator->() const noexcept(noexcept(**this)) -> std::enable_if_t<ReturnsRef, pointer> {
                return &**this;
            }

        private:
            Iterator it;
            FPtr f;
        };
        template<typename Container, typename Function>
        struct TransformContainer {
            using ContainerType = std::remove_reference_t<Container>;
            using Ftype = std::remove_reference_t<Function>;
            using FPtr = std::shared_ptr<Ftype>;
            using It = decltype(std::begin(std::declval<std::add_lvalue_reference_t<ContainerType>>()));
            using ConstIt = decltype(std::begin(
                    std::declval<std::add_lvalue_reference_t<std::add_const_t<ContainerType>>>()));
            using Sentinel = decltype(std::end(std::declval<std::add_lvalue_reference_t<ContainerType>>()));
            using ConstSentinel = decltype(std::end(
                    std::declval<std::add_lvalue_reference_t<std::add_const_t<ContainerType>>>()));

            template<typename T, typename F>
            TransformContainer(T &&init, F &&func) : container(std::forward<T>(init)),
                                                     f(std::make_shared<Ftype>(std::forward<F>(func))) {}

            auto begin() const noexcept(std::is_nothrow_constructible_v<TransformIterator<ConstIt, Ftype>,
                    ConstIt, FPtr>) {
                return TransformIterator<ConstIt, Ftype>(std::begin(container), f);
            }

            auto end() const noexcept(std::is_nothrow_constructible_v<TransformIterator<ConstSentinel, Ftype>,
                    ConstSentinel, FPtr>) {
                return TransformIterator<ConstSentinel, Ftype>(std::end(container), f);
            }

            auto begin() noexcept(std::is_nothrow_constructible_v<TransformIterator<It, Ftype>, It, FPtr>) {
                return TransformIterator<It, Ftype>(std::begin(container), f);
            }

            auto end() noexcept(std::is_nothrow_constructible_v<TransformIterator<Sentinel, Ftype>, Sentinel, FPtr>) {
                return TransformIterator<Sentinel, Ftype>(std::end(container), f);
            }
        private:
            Container container;
            FPtr f;
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
     * @return impl::ZipView class that provides begin and end members to be used in range based for-loops
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

    /**
     * Creates a TransformIterator from an arbitrary iterator and a function object
     * @tparam Iterator type of iterator
     * @tparam Function  type of function
     * @param iterator base iterator
     * @param function transformation function
     * @return TransformIterator
     */
    template<typename Iterator, typename Function, std::enable_if_t<impl::traits::is_dereferencible_v<Iterator>, int> = 0>
    auto transform(const Iterator &iterator,
                   Function &&function) -> impl::TransformIterator<Iterator, std::remove_reference_t<Function>> {
        return impl::TransformIterator<Iterator, std::remove_reference_t<Function>>(iterator,
                                                                                    std::forward<Function>(function));
    }

    /**
     * Transform view similar to std::ranges::transform_view
     * @tparam Container Container type that supports iteration
     * @tparam Function Function object that is callable with container elements
     * @param container source container
     * @param function function object that is applied to each element
     * @return TransformContainer class that provides begin and end members to be used in range based for loops
     */
    template<typename Container, typename Function, std::enable_if_t<impl::traits::is_container_v<Container>, int> = 0>
    auto transform(Container &&container, Function &&function) {
        return impl::TransformContainer<Container, Function>(std::forward<Container>(container),
                                                             std::forward<Function>(function));
    }
}

#endif //ITERATORTOOLS_ITERATORS_HPP
