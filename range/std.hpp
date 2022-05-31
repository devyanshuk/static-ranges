#ifndef __STD_HPP__
#define __STD_HPP__

#include <array>
#include <tuple>
#include <type_traits>

#include "concepts.hpp"
#include "range_traits.hpp"
#include "convenience.hpp"


namespace static_ranges {

    /*
    * Convert a static range to std::pair. There must be exactly
    * 2 elements in the static range.
    */
    template<typename Range>
        requires(range_nocvref<Range>::value == 2)
    auto to_pair(Range && r)
    {
        static_assert(
            static_ranges::range<std::remove_cvref_t<Range>>,
            "type Range in to_pair must satisfy the range concept");

        return std::make_pair(
            static_ranges::element<0>(std::forward<Range>(r)),
            static_ranges::element<1>(std::forward<Range>(r)));
    }


    template<typename Indices>
    struct to_tuple_impl;

    template<std::size_t ... Is>
    struct to_tuple_impl<std::index_sequence<Is ...>>
    {
        /*
        * Helper function used by static_ranges::to_tuple()
        */
        template<typename Range>
        static auto call(Range && r) {
            return std::make_tuple( 
                (static_ranges::element<Is>(std::forward<Range>(r)))... );
        }

        /*
        * Helper function used by static_views::static_iota<T, N>() to make
        * N std::integral_constant empty objects.
        */
        template<typename T>
        static constexpr auto call_static_iota() {
            return std::make_tuple( 
                (std::integral_constant<std::remove_cvref_t<T>, Is>())... );
        }

    };

    /*
    * returns a std::tuple containing copies of the
    * elements of the static range r
    */
    template<typename Range>
        requires static_ranges::range<Range>
    decltype(auto) to_tuple(Range && r)
    {
        return to_tuple_impl<make_sequence<Range>>::call(
            std::forward<Range>(r));
    }



    /*
    * Used to check if T(element) is defined. Used by the to_array
    * function to check if all elements of the static ranges are copy
    * constructible to the type T of the array.
    */
    template<typename T, typename Range, std::size_t I>
    concept is_copy_constructible_impl = requires (Range && r) {
        T(static_ranges::element<I>(std::forward<std::remove_cvref_t<Range>>(r)));
    };

    template<typename T, typename Range, typename Indices>
    struct is_copy_constructible_arr;

    template<typename T, typename Range, std::size_t I1, std::size_t I2, std::size_t ... Is>
        requires static_ranges::range<Range>
    struct is_copy_constructible_arr<T, Range, std::index_sequence<I1, I2, Is ...>>
    {
        static constexpr bool value = is_copy_constructible_impl<T, Range, I1> && 
            is_copy_constructible_arr<T, Range, std::index_sequence<I2, Is ...>>::value;
    };

    template<typename T, typename Range, std::size_t I>
        requires static_ranges::range<Range>
    struct is_copy_constructible_arr<T, Range, std::index_sequence<I>> 
    {
        static constexpr bool value = is_copy_constructible_impl<T, Range, I>;
    };


    template<typename ArrayType, typename Indices>
    struct to_array_impl;

    template<typename ArrayType, std::size_t ... Is>
    struct to_array_impl<ArrayType, std::index_sequence<Is ...>> 
    {
        /*
        * Helper function for static_ranges::to_array
        */
        template<typename Range>
            requires range<std::remove_cvref_t<Range>>
        static std::array
        <
            std::remove_cvref_t<ArrayType>,
            range_nocvref<Range>::value
        >
        call_to(Range && r) {
            static_assert(
                is_copy_constructible_arr<ArrayType, std::remove_cvref_t<Range>, make_sequence<Range>>::value,
                "Array must be copy-constructible from all the element types in the range");

            return { 
                { static_ranges::element<Is>(std::forward<Range>(r))... } 
            };   
        }

        /*
        * Helper function for static_views::iota<T, N>().
        * Creates a static range containing the elements {0,1,...,N-1} of type T,
        * which will then be used to create a static view.
        */
        static constexpr auto call_iota() {
            static_assert(std::is_integral_v<ArrayType>, "T must be an integral type");
            return std::to_array({ ((ArrayType)Is)... });
        }

    };


    /*
    * Converts a static range to std::array
    * ArrayType must be copy-constructible from all elements
    * in the range.
    */
    template<typename ArrayType, typename Range>
        requires static_ranges::range<std::remove_cvref_t<Range>>
    auto to_array(Range && r) {
       return to_array_impl<ArrayType, make_sequence<Range>>::call_to(
           std::forward<Range>(r));
    }

} // static_ranges namespace


#endif //__STD_HPP__