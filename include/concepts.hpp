#ifndef __CONCEPTS_HPP__
#define __CONCEPTS_HPP__

#include <type_traits>

#include "range_traits.hpp"

namespace static_ranges {

    template<typename Range, typename Indices>
    struct has_get_range_impl;

    template<typename Range, std::size_t ... I>
    struct has_get_range_impl<Range, std::index_sequence<I ...>>
    {
        static constexpr bool value = requires (Range t) {
            (range_traits
            <
                std::remove_cvref_t<Range>
            >::template get<I>(t), ...);
        };
    };


    template<typename T, typename U>
    concept has_same_type = std::is_same_v
    <
        U,
        typename range_traits
        <
            std::remove_cvref_t<T>
        >::value_type
    >;


    template<typename T>
    concept is_constant_expression = std::is_const_v
    <
        decltype(range_traits
        <
            std::remove_cvref_t<T>
        >::value)
    >;


    template<typename Range>
    concept has_range_traits = requires {
        typename range_traits
        <
            std::remove_cvref_t<Range>
        >;
    };


    template<typename Range>
    concept has_get_range = has_get_range_impl
    <
        Range,
        std::make_index_sequence
        <
            range_traits
            <
                std::remove_cvref_t<Range>
            >::value
        >
    >::value;


    /*
    * The range<T> concept is satisfied if:
    * 1) range_traits<std::remove_cvref_t<T>> is defined
    * 2) The expression static_ranges::range_traits<std::remove_cvref_t<T>>::value 
    *    is a constant expression of type std::size_t
    * 3) The expression static_ranges::range_traits<std::remove_cvref_t<T>>::template get<I>(r)
    *    is defined for:
    *    a) any expression r of type T
    *    b) any integer I such that 0 <= I < static_ranges::range_traits<std::remove_cvref_t<T>>::value
    */
    template<typename T>
    concept range =
        has_range_traits<T> &&
        has_same_type<T, std::size_t> &&
        is_constant_expression<T> &&
        has_get_range<T>;


    template<typename T>
    concept is_view_base_child = std::is_base_of_v
    <
        static_ranges::view_base,
        std::remove_cvref_t<T>
    >;

    /*
    * The view<T> concept is satisfied if:
    * 1) T is a range
    * 2) T derives from static_ranges::view_base
    */
    template<typename View>
    concept view =
        range<View> &&
        is_view_base_child<View>;

    template<typename Range>
    concept range_but_not_view =
        range<Range> && !is_view_base_child<Range>;

}

#endif //__CONCEPTS_HPP__