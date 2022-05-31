#ifndef __HELPERS_HPP__
#define __HELPERS_HPP__

#include <type_traits>
#include <utility>
#include <array>

#include "range_traits.hpp"
#include "concepts.hpp"

#define COPY_ASSIGN_ERROR "Each element i in range_src must be copy assignable to the corresponding range_src"
#define TRANSFORM_TWO_ERROR "f(e1I,e2I) must be copy-assignable to the I-th element of r3"
#define TRANSFORM_ONE_ERROR "f(e1I) must be copy-assignable to the I-th element of r2"

namespace static_ranges {

    template<typename Indices>
    struct copy_impl;

    template<std::size_t I, std::size_t ... Is>
    struct copy_impl<std::index_sequence<I, Is ...>>
    {
        template<typename RangeSrc, typename RangeDest>
        static void assign(RangeSrc && r1, RangeDest & r2) {

            static_assert(std::is_assignable_v<
                decltype(static_ranges::element<I>(r2)),
                decltype(static_ranges::element<I>(r1))>,
                COPY_ASSIGN_ERROR);

            static_ranges::element<I>(r2) = static_ranges::element<I>(std::move(r1));

            copy_impl<std::index_sequence<Is ...>>::assign(std::move(r1), r2);
        }
        
    };

    template<>
    struct copy_impl<std::index_sequence<>>
    {
        template<typename Range1, typename Range2>
        static void assign(Range1 && r1, Range2 & r2) {}
    };


    /*
    * Given two static ranges r1 and r2, copy all elements of r1 to r2.
    * r1 and r2 should have the same size and each i in r1 must be copy-assignable to
    * the corresponding i in r2.
    */
    template<typename RangeSrc, typename RangeDest>
        requires (range_nocvref<RangeSrc>::value == range_nocvref<RangeDest>::value)
    constexpr void copy(RangeSrc && r1, RangeDest & r2) 
    {
        copy_impl<make_sequence<RangeSrc>>::assign(std::forward<RangeSrc>(r1), r2);
    }


    template<typename Indices>
    struct for_each_impl;

    template<std::size_t ... I>
    struct for_each_impl<std::index_sequence<I ...>>
    {
        template<typename Range, typename Func>
            requires range<std::remove_cvref_t<Range>>
        static void call(Range && r, Func && f) {
            (f(static_ranges::element<I>(std::forward<Range>(r))), ...);
        }
    };


    /*
    * Given a static range r and a functor f, apply 
    * f(e) to every element e of the static range r.
    * Note: f(e) may modify the element e inside the range.
    * 
    * example:
    * 
    * range_2 r = { 2, 4 };
    * static_ranges::for_each(r, [](auto && v) { std::cout << v << std::endl; });
    */
    template<typename Range, typename Func>
        requires range<std::remove_cvref_t<Range>>
    constexpr void for_each(Range && r, Func && f) {
        for_each_impl<make_sequence<Range>>::call(
            std::forward<Range>(r),
            std::forward<Func>(f));
    }


    template<typename Indices>
    struct transform_impl;


    template<std::size_t I, std::size_t ... Is>
    struct transform_impl<std::index_sequence<I, Is ...>> {

        template<typename Range1, typename Range2, typename Func>
            requires range<Range1> && range<Range2>
        static void call_two(Range1 && r1, Range2 & r2, Func && f) {
            static_assert(std::is_assignable_v<
                decltype(static_ranges::element<I>(r2)),
                decltype(f(static_ranges::element<I>(std::forward<Range1>(r1))))>,
                TRANSFORM_ONE_ERROR);

            static_ranges::element<I>(r2) = f(
                static_ranges::element<I>(std::forward<Range1>(r1)));

            transform_impl<std::index_sequence<Is ...>>::call_two(
                std::forward<Range1>(r1),
                r2,
                std::forward<Func>(f));
        }


        template<typename Range1, typename Range2, typename Range3, typename Func>
            requires range<Range1> && range<Range2> && range<Range3> 
        static void call_three(Range1 && r1, Range2 && r2, Range3 & r3, Func && f) {
            static_assert(std::is_assignable_v<
                decltype(static_ranges::element<I>(r3)),
                decltype(f(
                    static_ranges::element<I>(std::forward<Range1>(r1)),
                    static_ranges::element<I>(std::forward<Range2>(r2))))>,
                TRANSFORM_TWO_ERROR);

            static_ranges::element<I>(r3) = f(
                static_ranges::element<I>(std::forward<Range1>(r1)),
                static_ranges::element<I>(std::forward<Range2>(r2)));

            transform_impl<std::index_sequence<Is ...>>::call_three(
                std::forward<Range1>(r1),
                std::forward<Range2>(r2),
                r3,
                std::forward<Func>(f));
        }
    };

    template<>
    struct transform_impl<std::index_sequence<>> {
        template<typename Range1, typename Range2, typename Func>
        static void call_two(Range1 && r1, Range2 & r2, Func && f) {}

        template<typename Range1, typename Range2, typename Range3, typename Func>
        static void call_three(Range1 && r1, Range2 && r2, Range3 & r3, Func && f) {}
    };


    /*
    * Given a source static range, a destination static range and a functor,
    * apply f(src)_i and store the result to the corresponding ith place of the
    * destination static range.
    * 
    * example:
    * 
    * range_2 src = { 2, 55 };
    * range_2 dest;
    * 
    * static_ranges::transform(src, dest, [](auto && v) { return v + 1 };)
    */
    template<typename RangeSrc, typename RangeDest, typename Func>
        requires (range_nocvref<RangeSrc>::value == range_nocvref<RangeDest>::value)
    constexpr void transform(RangeSrc && r1, RangeDest & r2, Func && f) {
        transform_impl<make_sequence<RangeSrc>>::call_two(
            std::forward<RangeSrc>(r1),
            r2,
            std::forward<Func>(f));
    }


    /*
    * Given two source static ranges, a destination static range and a functor,
    * apply f(src1, src2)_i and store the result to the corresponding ith place of the
    * destination static range.
    * 
    * example:
    * 
    * const range_three r = { 42, 3.14, "Some_quite_large_string_containing_Foo_and_Bar" };
    * range_three r3 = { -40, -2.0, "_plus_suffix" };
	* range_three r4;
	* static_ranges::transform(r, std::move(r3), r4, [](auto&& x, auto&& y) { return x + y; });
    * std::cout << static_ranges::element<0>(r4); // would be  2
    * 
    */
    template<typename RangeSrc1, typename RangeSrc2, typename RangeDest, typename Func>
        requires (range_nocvref<RangeSrc1>::value == range_nocvref<RangeDest>::value &&
                  range_nocvref<RangeSrc1>::value == range_nocvref<RangeSrc2>::value)
    constexpr void transform(RangeSrc1 && r1, RangeSrc2 && r2, RangeDest & r3, Func && f) {
        transform_impl<make_sequence<RangeSrc1>>::call_three(
            std::forward<RangeSrc1>(r1),
            std::forward<RangeSrc2>(r2),
            r3,
            std::forward<Func>(f));
    }

}

#endif //__HELPERS_HPP__