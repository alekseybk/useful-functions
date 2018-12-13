/*
 * Copyright Aleksey Verkholat 2018 - ...
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt
*/

#pragma once
#include <vector>
#include <functional>
#include <atomic>
#include <thread>

namespace uf
{
    using std::vector;
    using std::string;
    using std::string_view;
    using std::pair;
    using std::function;
    using std::reference_wrapper;
    using std::atomic;
    using std::atomic_flag;

    using std::enable_if_t;
    using std::remove_const_t;
    using std::remove_reference_t;
    using std::remove_pointer_t;
    using std::is_lvalue_reference_v;
    using std::conditional_t;
    using std::is_pointer_v;
    using std::is_const_v;
    using std::is_same_v;
    using std::decay_t;
    using std::integral_constant;
    using std::tuple;
    using std::tuple_element_t;
    using std::tuple_size_v;
    using std::is_invocable_v;
    using std::is_invocable_r_v;

    using std::true_type;
    using std::false_type;
    using std::index_sequence;
    using std::make_index_sequence;

    using std::declval;

    namespace basic_types
    {
        using u8 = uint8_t;
        using i8 = int8_t;

        using u16 = uint16_t;
        using i16 = int16_t;

        using u32 = uint32_t;
        using i32 = int32_t;

        using u64 = uint64_t;
        using i64 = int64_t;

        namespace literals
        {
            inline constexpr u8 operator"" _u8(unsigned long long int value) noexcept { return value; }
            inline constexpr i8 operator"" _i8(unsigned long long int value) noexcept { return value; }

            inline constexpr u16 operator"" _u16(unsigned long long int value) noexcept { return value; }
            inline constexpr i16 operator"" _i16(unsigned long long int value) noexcept { return value; }

            inline constexpr u32 operator"" _u32(unsigned long long int value) noexcept { return value; }
            inline constexpr i32 operator"" _i32(unsigned long long int value) noexcept { return value; }

            inline constexpr u64 operator"" _u64(unsigned long long int value) noexcept { return value; }
            inline constexpr i64 operator"" _i64(unsigned long long int value) noexcept { return value; }
        }
        // end namespace literals
    }
    // end namespace basic_types

    using namespace basic_types;
    using namespace basic_types::literals;

    namespace meta
    {
        namespace detail
        {
            template<class Tp>
            struct is_string_type_helper : public conditional_t<is_pointer_v<Tp>,
                                                      conditional_t<is_same_v<decay_t<remove_pointer_t<Tp>>, char>,
                                                          true_type,
                                                          false_type>,
                                                      false_type> { };

            template<>
            struct is_string_type_helper<string> : public true_type { };

            template<>
            struct is_string_type_helper<string_view> : public true_type { };

            template<typename Tuple, typename N, typename... Types>
            struct tuple_remove_last_helper
            {
                using type = typename tuple_remove_last_helper<Tuple, integral_constant<size_t, N::value - 1u>, tuple_element_t<N::value - 1u, Tuple>, Types...>::type;
            };

            template<typename Tuple, typename... Types>
            struct tuple_remove_last_helper<Tuple, integral_constant<size_t, 0u>, Types...>
            {
                using type = tuple<Types...>;
            };

            template<typename Tuple, typename N, typename... Ts>
            struct tuple_remove_first_helper
            {
                using type = typename tuple_remove_first_helper<Tuple, integral_constant<size_t, N::value + 1u>, Ts..., tuple_element_t<N::value, Tuple>>::type;
            };

            template<typename Tuple, typename... Ts>
            struct tuple_remove_first_helper<Tuple, integral_constant<size_t, tuple_size_v<Tuple>>, Ts...>
            {
                using type = tuple<Ts...>;
            };

            template<typename TupleA, typename IndexA, typename TupleB, typename IndexB, typename... Types>
            struct tuple_concat_two_helper
            {
                using type = typename tuple_concat_two_helper<TupleA, integral_constant<size_t, IndexA::value + 1u>,
                                                              TupleB, integral_constant<size_t, IndexB::value>,
                                                              Types..., tuple_element_t<IndexA::value, TupleA>>::type;
            };

            template<typename TupleA, typename TupleB, typename IndexB, typename... Types>
            struct tuple_concat_two_helper<TupleA, integral_constant<size_t, tuple_size_v<TupleA>>, TupleB, IndexB, Types...>
            {
                using type = typename tuple_concat_two_helper<TupleA, integral_constant<size_t, tuple_size_v<TupleA>>,
                                                              TupleB, integral_constant<size_t, IndexB::value + 1u>,
                                                              Types..., tuple_element_t<IndexB::value, TupleB>>::type;
            };

            template<typename TupleA, typename TupleB, typename... Types>
            struct tuple_concat_two_helper<TupleA, integral_constant<size_t, tuple_size_v<TupleA>>, TupleB,
                                           integral_constant<size_t, tuple_size_v<TupleB>>, Types...>
            {
                using type = tuple<Types...>;
            };

            template<typename TupleA, typename TupleB>
            struct tuple_concat_two
            {
                using type = typename tuple_concat_two_helper<TupleA, integral_constant<size_t, 0u>, TupleB, integral_constant<size_t, 0u>>::type;
            };

            template<typename Result, typename Tuple, typename... Tuples>
            struct tuple_concat_helper
            {
                using type = typename tuple_concat_helper<typename tuple_concat_two<Result, Tuple>::type, Tuples...>::type;
            };

            template<typename Result, typename Tuple>
            struct tuple_concat_helper<Result, Tuple>
            {
                using type = typename tuple_concat_two<Result, Tuple>::type;
            };

            template<typename Tuple, size_t N, typename... Types>
            struct tuple_push_back_helper
            {
                using type = typename tuple_push_back_helper<Tuple, N - 1u, tuple_element_t<N - 1u, Tuple>, Types...>::type;
            };

            template<typename Tuple, typename... Types>
            struct tuple_push_back_helper<Tuple, 0u, Types...>
            {
                using type = tuple<Types...>;
            };

            template<typename Tuple, typename N, typename... Types>
            struct tuple_push_front_helper
            {
                using type = typename tuple_push_front_helper<Tuple, integral_constant<size_t, N::value + 1u>, Types..., tuple_element_t<N::value, Tuple>>::type;
            };

            template<typename Tuple, typename... Types>
            struct tuple_push_front_helper<Tuple, integral_constant<size_t, tuple_size_v<Tuple>>, Types...>
            {
                using type = tuple<Types...>;
            };

            template<typename Tp, size_t Index, typename Tuple, typename CurIndex, typename... Ts>
            struct tuple_replace_index_helper
            {
                using type = typename tuple_replace_index_helper<Tp, Index, Tuple,
                             integral_constant<size_t, CurIndex::value + 1>, Ts..., tuple_element_t<CurIndex::value, Tuple>>::type;
            };

            template<typename Tp, size_t Index, typename Tuple, typename... Ts>
            struct tuple_replace_index_helper<Tp, Index, Tuple, integral_constant<size_t, Index>, Ts...>
            {
                using type = typename tuple_replace_index_helper<Tp, Index, Tuple, integral_constant<size_t, Index + 1>, Ts..., Tp>::type;
            };

            template<typename Tp, size_t Index, typename Tuple, typename... Ts>
            struct tuple_replace_index_helper<Tp, Index, Tuple, integral_constant<size_t, tuple_size_v<Tuple>>, Ts...>
            {
                using type = tuple<Ts...>;
            };

            template<typename Tp, size_t N, typename... Types>
            struct tuple_same_n_helper
            {
                using type = typename tuple_same_n_helper<Tp, N - 1u, Tp, Types...>::type;
            };

            template<typename Tp, typename... Types>
            struct tuple_same_n_helper<Tp, 0u, Types...>
            {
                using type = tuple<Types...>;
            };

            template<template<typename...> typename Clean, typename Tuple>
            struct tuple_apply_to_clean_helper;

            template<template<typename...> typename Clean, typename... Ts>
            struct tuple_apply_to_clean_helper<Clean, tuple<Ts...>>
            {
                using type = Clean<Ts...>;
            };

            template<template<typename...> typename Expected, typename Tested>
            struct is_same_tmpl_helper : public false_type { };

            template<template<typename...> typename Expected, typename... Ts>
            struct is_same_tmpl_helper<Expected, Expected<Ts...>> : public true_type { };
        }
        // end namespace detail

        template<class Tp>
        struct is_string_type
        {
            using type = typename detail::is_string_type_helper<decay_t<Tp>>::type;
        };

        template<class Tp>
        inline constexpr bool is_string_type_v = is_string_type<Tp>::value;

        template<typename Tuple, typename = enable_if_t<(tuple_size_v<Tuple> >= 1u)>>
        struct tuple_remove_last
        {
            using type = typename detail::tuple_remove_last_helper<Tuple, integral_constant<size_t, tuple_size_v<Tuple> - 1u>>::type;
        };

        template<typename Tuple>
        using tuple_remove_last_t = typename tuple_remove_last<Tuple>::type;

        template<typename Tuple, size_t N, typename = enable_if_t<(tuple_size_v<Tuple> >= N)>>
        struct tuple_remove_last_n
        {
            using type = typename detail::tuple_remove_last_helper<Tuple, integral_constant<size_t, tuple_size_v<Tuple> - N>>::type;
        };

        template<typename Tuple, size_t N>
        using tuple_remove_last_n_t = typename tuple_remove_last_n<Tuple, N>::type;

        template<typename Tuple, typename = enable_if_t<(tuple_size_v<Tuple> >= 1)>>
        struct tuple_remove_first
        {
            using type = typename detail::tuple_remove_first_helper<Tuple, integral_constant<size_t, 1u>>::type;
        };

        template<typename Tuple>
        using tuple_remove_first_t = typename tuple_remove_first<Tuple>::type;

        template<typename Tuple, size_t N, typename = enable_if_t<(tuple_size_v<Tuple> >= N)>>
        struct tuple_remove_first_n
        {
            using type = typename detail::tuple_remove_first_helper<Tuple, integral_constant<size_t, N>>::type;
        };

        template<typename Tuple, size_t N>
        using tuple_remove_first_n_t = typename tuple_remove_first_n<Tuple, N>::type;

        template<typename Tuple, typename... Types>
        struct tuple_push_back
        {
            using type = typename detail::tuple_push_back_helper<Tuple, tuple_size_v<Tuple>, Types...>::type;
        };

        template<typename Tuple, typename... Types>
        using tuple_push_back_t = typename tuple_push_back<Tuple, Types...>::type;

        template<typename Tuple, typename... Types>
        struct tuple_push_front
        {
            using type = typename detail::tuple_push_front_helper<Tuple, integral_constant<size_t, 0u>, Types...>::type;
        };

        template<typename Tuple, typename... Types>
        using tuple_push_front_t = typename tuple_push_front<Tuple, Types...>::type;

        template<typename... Tuples>
        struct tuple_concat
        {
            using type = typename detail::tuple_concat_helper<tuple<>, Tuples...>::type;
        };

        template<typename... Tuples>
        using tuple_concat_t = typename tuple_concat<Tuples...>::type;

        template<typename Tp, size_t N>
        struct tuple_same_n
        {
            using type = typename detail::tuple_same_n_helper<Tp, N>::type;
        };

        template<typename Tp, size_t N>
        using tuple_same_n_t = typename tuple_same_n<Tp, N>::type;

        template<typename Tp, size_t Index, typename Tuple>
        struct tuple_replace_index
        {
            using type = typename detail::tuple_replace_index_helper<Tp, Index, Tuple, integral_constant<size_t, 0u>>::type;
        };

        template<typename Tp, size_t Index, typename Tuple>
        using tuple_replace_index_t = typename tuple_replace_index<Tp, Index, Tuple>::type;

        template<template<typename...> typename Clean, typename Tuple>
        struct tuple_apply_to_clean
        {
            using type = typename detail::tuple_apply_to_clean_helper<Clean, Tuple>::type;
        };

        template<template<typename...> typename Clean, typename Tuple>
        using tuple_apply_to_clean_t = typename tuple_apply_to_clean<Clean, Tuple>::type;

        template<typename Tp>
        struct clean;

        template<template<typename...> typename Tp, typename... Ts>
        struct clean<Tp<Ts...>>
        {
            template<typename... Types>
            using type = Tp<Types...>;

            using args_tuple = tuple<Ts...>;
        };

        template<template<typename...> typename Expected, typename Tested>
        struct is_same_tmpl
        {
            static constexpr bool value = detail::is_same_tmpl_helper<Expected, decay_t<Tested>>::value;
        };

        template<template<typename...> typename Expected, typename Tested>
        inline constexpr bool is_same_tmpl_v =  is_same_tmpl<Expected, decay_t<Tested>>::value;

        template<typename... Ts>
        using first_t = tuple_element_t<0, tuple<Ts...>>;

        template<typename... Ts>
        using last_t = tuple_element_t<sizeof...(Ts) - 1, tuple<Ts...>>;

        template<typename... Ts>
        struct is_same_all
        {
            using type = first_t<Ts...>;

            static constexpr bool value = (is_same_v<Ts, type> && ...);
        };

        template<typename... Ts>
        inline constexpr bool is_same_all_v = is_same_all<Ts...>::value;

        template<class Function>
        struct function_info;

        template<class R, class... Args>
        struct function_info<function<R(Args...)>>
        {
            static constexpr size_t arg_n = sizeof...(Args);

            using res_t = R;

            template<size_t N>
            using arg_t = tuple_element_t<N, tuple<Args...>>;
        };
    }
    // end namespace meta

    namespace utils
    {
        namespace detail
        {
            template<bool Minimum, typename Result, typename... Ts, enable_if_t<!meta::is_same_all_v<decay_t<Ts>...>, int> = 0>
            constexpr auto min_max_1(Ts&&... args)
            {
                static_assert(is_same_v<Result, decay_t<Result>>);

                Result m;
                m = *std::get<0>(tuple<remove_reference_t<Ts>*...>{&args...});
                if constexpr (Minimum)
                    ((m = (args < m) ? std::forward<Ts>(args) : std::move(m)), ...);
                else
                    ((m = (args > m) ? std::forward<Ts>(args) : std::move(m)), ...);
                return m;
            }

            template<bool Minimum, typename Result, typename... Ts, enable_if_t<meta::is_same_all_v<decay_t<Ts>...>, int> = 0>
            constexpr auto min_max_1(Ts&&... args)
            {
                static_assert(is_same_v<Result, decay_t<Result>>);
                using type = decay_t<meta::first_t<Ts...>>;

                const type* m = std::get<0>(tuple<remove_reference_t<Ts>*...>{&args...});
                if constexpr (Minimum)
                    ((m = (args < *m) ? &args : m), ...);
                else
                    ((m = (args > *m) ? &args : m), ...);
                if constexpr (is_same_v<Result, type>)
                    return *m;
                else
                    return Result(*m);
            }

            template<bool Minimum, typename... Ts>
            constexpr auto& min_max_ref_1(Ts&&... args)
            {
                static_assert((is_lvalue_reference_v<Ts> && ...) && meta::is_same_all_v<decay_t<Ts>...>, "All arguments must be an lvalues of the same type");
                using type = conditional_t<(is_const_v<remove_reference_t<Ts>> || ...),
                                           const remove_reference_t<meta::first_t<Ts...>>,
                                           remove_reference_t<meta::first_t<Ts...>>>;

                reference_wrapper<type> m = *std::get<0>(tuple<remove_reference_t<Ts>*...>{&args...});
                if constexpr (Minimum)
                    ((m = (args < m.get()) ? reference_wrapper<type>(args) : m), ...);
                else
                    ((m = (args > m.get()) ? reference_wrapper<type>(args) : m), ...);
                return m.get();
            }
        }
        // end namespace detail

        template<typename Owner, typename E>
        constexpr auto&& fwd_elem(E&& e)
        {
            using type = remove_reference_t<E>;

            if constexpr (!is_lvalue_reference_v<E>)
                return static_cast<type&&>(e);
            else if constexpr (is_lvalue_reference_v<Owner>)
                return static_cast<type&>(e);
            else
                return static_cast<type&&>(e);
        }

        template<typename Result, typename... Ts>
        constexpr auto min(Ts&&... args) { return detail::min_max_1<true, Result>(std::forward<Ts>(args)...); }

        template<typename... Ts>
        constexpr auto min(Ts&&... args) { return detail::min_max_1<true, decay_t<meta::first_t<Ts...>>>(std::forward<Ts>(args)...); }

        template<typename... Ts>
        constexpr auto& min_ref(Ts&&... args) { return detail::min_max_ref_1<true, Ts...>(std::forward<Ts>(args)...); }

        template<typename Result, typename... Ts>
        constexpr auto max(Ts&&... args) { return detail::min_max_1<false, Result>(std::forward<Ts>(args)...); }

        template<typename... Ts>
        constexpr auto max(Ts&&... args) { return detail::min_max_1<false, decay_t<meta::first_t<Ts...>>>(std::forward<Ts>(args)...); }

        template<typename... Ts>
        constexpr auto& max_ref(Ts&&... args) { return detail::min_max_ref_1<false, Ts...>(std::forward<Ts>(args)...); }
    }
    // end namespace utils

    namespace detail
    {
        template<class Container, class Bounds>
        auto create_split_vector(Bounds&& bounds)
        {
            vector<Container> result;
            result.reserve(bounds.size());
            for (const auto& [b, e] : bounds)
                result.emplace_back(b, e);
            return result;
        }

        template<class SplitVector, size_t... Indexes>
        auto create_split_tuple(SplitVector&& vec, index_sequence<Indexes...>)
        {
            using elem_t = typename remove_reference_t<SplitVector>::value_type;
            return meta::tuple_same_n_t<elem_t, sizeof...(Indexes)>{utils::fwd_elem<SplitVector>(vec[Indexes])...};
        }

        template<size_t N, class Tuple, class ValueType>
        bool split_check_helper(Tuple&& t, const ValueType& v)
        {
            using tpl_t = remove_reference_t<Tuple>;

            if constexpr (N == tuple_size_v<tpl_t>)
                return false;
            else
            {
                bool result;
                if constexpr (is_invocable_r_v<bool, remove_reference_t<tuple_element_t<N, tpl_t>>, const ValueType&> &&
                              !is_same_v<decay_t<tuple_element_t<N, tpl_t>>, decay_t<ValueType>>)
                    result = std::get<N>(t)(v);
                else
                    result = (v == std::get<N>(t));
                if (result)
                    return true;
                return split_check_helper<N + 1>(t, v);
            }
        }
    }
    // end namesapce detail

    template<class Container, class... Delimiters>
    auto split(Container&& container, Delimiters&&... delimiters)
    {
        using container_t = remove_reference_t<Container>;
        using iterator_t = typename container_t::const_iterator;
        using value_t = typename iterator_t::value_type;

        auto check = [&delimiters...](const value_t& value) -> bool
        {
            return detail::split_check_helper<0>(tuple{delimiters...}, value);
        };

        bool empty_seq = true;
        vector<pair<iterator_t, iterator_t>> bounds;
        for (auto i = container.cbegin(); i != container.cend(); ++i)
        {
            if (empty_seq)
            {
                if (check(*i))
                    continue;
                empty_seq = false;
                bounds.push_back({i, container.cend()});
                continue;
            }
            if (check(*i))
            {
                bounds.back().second = i;
                empty_seq = true;
            }
        }

        return detail::create_split_vector<container_t>(std::move(bounds));
    }

    template<u64 N, class Container, class... Delimiters>
    auto split(Container&& container, Delimiters&&... delimiters)
    {
        auto temp = split(std::forward<Container>(container), std::forward<Delimiters>(delimiters)...);
        return detail::create_split_tuple(std::move(temp), make_index_sequence<N>());
    }

    template<class Container>
    auto position_pairs(Container&& container)
    {
        using value_t = typename remove_reference_t<Container>::value_type;
        using result_t = typename meta::clean<remove_reference_t<Container>>::template type<pair<u64, value_t>>;

        u64 j = 0;
        result_t result;
        for (auto i = container.begin(); i != container.end(); ++i, ++j)
            result.push_back({j, utils::fwd_elem<Container>(*i)});
        return result;
    }

    template<class Container, class Compare>
    auto sort_save_position(Container&& container, const Compare& comp)
    {
        auto transformed = position_pairs(std::forward<Container>(container));
        sort(transformed.begin(), transformed.end(), [&comp](const auto& a, const auto& b)
        {
            return comp(a.second, b.second);
        });
        return transformed;
    }

    template<class AssociativeContainer, class Remover>
    void remove_associative(AssociativeContainer& container, Remover&& rm)
    {
        using mapped_t = typename AssociativeContainer::mapped_type;

        auto check = [&rm](const mapped_t& value)
        {
            if constexpr (is_invocable_r_v<bool, remove_reference_t<Remover>, const mapped_t&> && !is_same_v<decay_t<Remover>, decay_t<mapped_t>>)
                return rm(value);
            else
                return value == rm;
        };

        for (auto i = begin(container); i != end(container);)
        {
            if (check(i->second))
                i = container.erase(i);
            else
                ++i;
        }
    }

    class spinlock
    {
        atomic_flag flag_ = ATOMIC_FLAG_INIT;

    public:
        bool try_lock() { return !flag_.test_and_set(std::memory_order_acquire); }

        void lock() { while (flag_.test_and_set(std::memory_order_acquire)) std::this_thread::yield(); }

        void unlock() { flag_.clear(std::memory_order_release); }
    };

    template<class Tp, template<class> class Compare>
    class dereference_compare
    {
        const Compare<Tp> comp_;

    public:
        template<class Pointer>
        bool operator()(const Pointer& a, const Pointer& b) const
        {
            return comp_(*a, *b);
        }
    };
}
// end namespace uf
