#ifndef __VIEW_OBJECTS_HPP__
#define __VIEW_OBJECTS_HPP__

#include <functional>

#include "concepts.hpp"
#include "std.hpp"
#include "range_traits.hpp" 

namespace static_ranges {

    namespace views {

        /*
        * Used by static_views::all() to keep a reference of the range
        * so that the original range gets modified if the object returned
        * by all gets changed too.
        * 
        * example:
        * 
        * range_three r = { 42, 3.14, "Some_quite_large_string_containing_Foo_and_Bar" };
	    * auto r_view = static_ranges::views::all(r); //view_obj<range_three>
        * static_ranges::element<0>(r_view) -= 40; // r_view<0> would be 2
        * cout << static_ranges::element<0>(r); // r<0> would be 2 because of the reference wrapper.
        * 
        */
        template<typename Range>
            requires static_ranges::range<Range>
        struct view_obj
            : public static_ranges::view_base 
        {
        public:

            constexpr explicit view_obj(Range & t) : m_data(t) {}

            std::reference_wrapper<std::remove_cvref_t<Range>> m_data;
            
        };


        /*
        * Used by static_views::iota<N. I>() and static_views::static_iota<N, I>()
        */
        template<typename Range>
            requires static_ranges::range<Range>
        struct view_obj_iota : public static_ranges::view_base
        {
        public:

            constexpr explicit view_obj_iota(Range && t) 
                : m_data(std::forward<Range>(t)) {}

            std::remove_cvref_t<Range> m_data;
        };

        struct transformable
        {};


        /*
        * A static view object returned when a view adaptive closure 
        * object is applied to a static range.
        * 
        * example:
        * 
        * auto view = static_views::iota<int, 6>() | transform([](auto && v) { return v + 22; } );
        * //decltype(r) -> transform_view_object<iota_object, lambda>
        * 
        */
        template< typename View, typename Func>
            requires static_ranges::view<View>
        struct transform_view_object 
            : public static_ranges::view_base, public transformable
        {
        public:

            constexpr explicit transform_view_object(View && view, Func && func)
                : m_view(std::forward<View>(view)),
                  m_func(std::forward<Func>(func))
            {}

            View m_view;
            Func m_func;
        };
        
    } // views namespace

} //static_ranges namespace

#endif //__VIEW_OBJECTS_HPP__