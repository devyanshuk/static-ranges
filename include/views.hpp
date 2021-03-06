#ifndef __VIEWS_HPP__
#define __VIEWS_HPP__

#include <functional>
#include <type_traits>

#include "view_objects.hpp"
#include "range_traits.hpp"
#include "convenience.hpp"
#include "concepts.hpp"
#include "helpers.hpp"
#include "std.hpp"

namespace static_views = static_ranges::views;

namespace static_ranges {

    template<typename View, typename Func>
    using transform_view_object = static_views::transform_view_object<View, Func>;


    /**
    * Given a transform_view_object struct, get the ith element lazily.
    * If multiple transformations were chained together, apply all the
    * corresponding transformations to the ith element and return it.
    */
    template<std::size_t I, typename View>
        requires std::is_base_of_v<static_views::transformable, std::remove_cvref_t<View>>
    constexpr auto get_lazily(View && v) {
        return std::forward<View>(v).m_func(get_lazily<I>(std::forward<View>(v).m_view));
    }

    /*
    * Base case: if the static view object isn't a base of transformable struct, then retrieve
    * the element of that static view to apply all the functors from the caller.
    */
    template<std::size_t I, typename View>
        requires (!std::is_base_of_v<static_views::transformable, std::remove_cvref_t<View>>)
    constexpr auto get_lazily(View && v) {
        return static_ranges::element<I>(std::forward<View>(v));
    }


    /**
    * Partial specialization for transform_view_object, i.e. the closure type
    * that performs lazy evaluation.
    */
    template<typename View, typename Func>
        requires static_ranges::view<View>
    struct range_traits<transform_view_object<View, Func>> 
        : std::integral_constant<std::size_t, range_nocvref<View>::value>
    {

        template<std::size_t I>
        static constexpr decltype(auto) get(transform_view_object<View, Func> && t)
        {
            return get_lazily<I>(std::move(t));
        }

        template<std::size_t I>
        static constexpr decltype(auto) get(transform_view_object<View, Func> & t)
        {
            return get_lazily<I>(t);
        }

        template<std::size_t I>
        static constexpr decltype(auto) get(const transform_view_object<View, Func> & t)
        {
            return get_lazily<I>(t);
        }
    };


    namespace views {

        /*
        * returns an rvalue identical to e, if e is a static view.
        */
        template<typename View>
            requires static_ranges::view<std::remove_cvref_t<View>>
        auto&& all(View && t) {
            return std::move(t);
        }

        /*
        * returns a view object which behaves as a reference to the
        * static range given as an argument.
        */
        template<typename Range>
            requires range_but_not_view<std::remove_cvref_t<Range>>
        decltype(auto) all(Range && t) {
            return view_obj(std::forward<Range>(t));
        }


        /*
        * return a static view containing the elements {0,1,...,N-1} of type T.
        * T shall be an integral type
        * 
        * example:
        * 
        * auto iota_view = static_views::iota<int, 5>();
        * std::cout << static_ranges::element<0>(iota_view); //0
        */
        template<typename T, std::size_t N>
            requires std::is_integral_v<T>
        constexpr decltype(auto) iota() {
            return view_obj_iota(
                static_ranges::to_array_impl<
                    T,
                    std::make_index_sequence<N>
                >::call_iota());
        }


        /*
        * returns a static view representing the values {0,1,...,N-1} in the following way:
        * 1) The I-th element eI is an empty object such that the expression 
        *    decltype(eI)::value is a constant expression with value I of type T
        * 2) The expression eI.value will work too
        * 
        * T shall be an integral type
        * 
        * example:
        * 
        * std::cout << static_ranges::element<0>(static_views::static_iota<int, 6>()); //0
        */
        template<typename T, std::size_t N>
            requires std::is_integral_v<T>
        constexpr auto static_iota() {
            return view_obj_iota(
                static_ranges::to_tuple_impl<
                    std::make_index_sequence<N>
                >::template call_static_iota<T>());
        }


        /*
        * Range adaptor closure object returned when a transformation is
        * applied.
        * 
        * example:
        * 
        * auto v = static_views::transform([](auto && v) { return v + 1; });
        * //decltype(v) -> range_adaptor_closure<Func>
        */
        template<typename Func>
        struct range_adaptor_closure {

            constexpr explicit range_adaptor_closure(Func && f) 
                : m_func(std::forward<Func>(f))
            {}

            /*
            * Returns a static view that stores a reference to the static range
            * proived in the function argument.
            * The size of the static view will be the same as the static range given
            * to it.
            * Lazy evaluation is done, i.e. the eI element shall be retrieved and the
            * f(eI) expression shall be evaluated when the corresponding element of 
            * the returned static view is accessed.
            */
            template<typename Range>
                requires static_ranges::range<std::remove_cvref_t<Range>>
            auto operator()(Range && r) {
                return transform_view_object(
                    all(std::forward<Range>(r)), std::move(m_func));
            }

            Func m_func;
        };


        /*
        * Returns a static view, which, when accessed, will lazily evaluate
        * and return values.
        * 
        * example:
        * auto view = iota<int, 6>() | transform([](auto && v) { return v + 1; })
        *                            | transform([](auto && v) { return v + 4; })
        *                            | transform([](auto && v) { return v + 5; });
        */
        template<typename Range, typename Closure>
            requires static_ranges::range<std::remove_cvref_t<Range>>
        constexpr auto operator | (Range && r, Closure && c) {
            return std::forward<Closure>(c)(std::forward<Range>(r));
        }


        /*
        * Given a functor as an input, return a view adaptive
        * closure object that could then be applied to transform
        * the static view using it.
        * 
        * example:
        * auto closure = transform([](auto && v) { return v + 1; });
        * auto view = iota<int, 3>() | closure;
        */
        template<typename Func>
        constexpr auto transform(Func && f) {
            return range_adaptor_closure<Func>(
                std::forward<Func>(f));
        }


        /*
        * Returns a static view of the same size as the static range
        * passed to it.
        * Lazy evaluation is performed when an element of the returned
        * static view is accessed.
        * 
        * transform(r, f) is equivalent to:
        *   1) r | transform(f)
        *   2) transform(f)(r)
        */
        template<typename Range, typename Func>
            requires static_ranges::range<Range>
        constexpr auto transform(Range && r, Func && f) {
            return std::forward<Range>(r) | transform(std::forward<Func>(f));
        }


    } // views namespace

}

#endif // __VIEWS_HPP__