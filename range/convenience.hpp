#ifndef __CONVENIENCE_HPP__
#define __CONVENIENCE_HPP__

#include <functional>

#include "range_traits.hpp"
#include "concepts.hpp"


template<typename Range>
using range_nocvref = static_ranges::range_traits<std::remove_cvref_t<Range>>;

template<typename Range>
using make_sequence = std::make_index_sequence<range_nocvref<Range>::value>;


namespace static_ranges
{

    /*
    * Get the size of a range, i.e. range_traits<Range>::value.
    */
    template<typename T>
        requires requires { range_nocvref<T>::value; }
    static constexpr std::size_t size_v = range_nocvref<T>::value;

    
    /*
    * Retrieve the Ith element of a static range r.
    * Calls the get<I> function of the corresponding range
    * to get the value.
    */
    template<std::size_t I, typename Range>
        requires static_ranges::range<std::remove_cvref_t<Range>>
    decltype(auto) constexpr element(Range && t) {
        static_assert(I < size_v<Range>,"Index out of bounds.");
        return range_nocvref<Range>::template get<I>(std::forward<Range>(t));
    }

} // static_ranges namespace

#endif // __CONVENIENCE_HPP__