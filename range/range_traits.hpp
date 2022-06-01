#ifndef __RANGE_TRAITS_HPP__
#define __RANGE_TRAITS_HPP__

#include <type_traits>
#include <tuple>
#include <utility>

namespace static_ranges {

    struct view_base {};
    

    template<typename Range>
    struct range_traits;

    /*
    * Partial specialization for std::tuple, std::pair,
    * std::array, std::variant, etc.
    */
    template<typename StdRange>
        requires requires { std::tuple_size<StdRange>::value; } 
    struct range_traits<StdRange> : std::integral_constant
    <
        std::size_t,
        std::tuple_size<StdRange>::value
    >
    {
        template<std::size_t I>
        static constexpr decltype(auto) get(StdRange && t)
        {
            return std::get<I>(std::forward<StdRange>(t));
        }

        template<std::size_t I>
        static constexpr decltype(auto) get(StdRange & r)
        {
            return std::get<I>(r);
        }

        template<std::size_t I>
        static constexpr decltype(auto) get(const StdRange & r)
        {
            return std::get<I>(r);
        }
    };

    template<typename T>
    using nocvref_range = static_ranges::range_traits<std::remove_cvref_t<T>>;


    /**
    * Partial specialization for when view obj encapsulates
    * the range. 
    */
    template< template<typename> typename View, typename Range>
        requires std::is_base_of_v<view_base, View<Range>>
    struct range_traits<View<Range>> 
        : std::integral_constant<std::size_t, nocvref_range<Range>::value>
    {
        template<std::size_t I>
        static constexpr decltype(auto) get(View<Range> && t)
        {
            return nocvref_range<Range>::template get<I>(std::move(t).m_data);
        }

        template<std::size_t I>
        static constexpr decltype(auto) get(View<Range> & t)
        {
            return nocvref_range<Range>::template get<I>(t.m_data);
        }

        template<std::size_t I>
        static constexpr decltype(auto) get(const View<Range> & t)
        {
            return nocvref_range<Range>::template get<I>(t.m_data);
        }
    };

} // static_ranges namespace

#endif // __RANGE_TRAITS_HPP__