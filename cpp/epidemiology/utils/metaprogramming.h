#ifndef EPI_UTILS_METAPROGRAMMING_H
#define EPI_UTILS_METAPROGRAMMING_H

#include <type_traits>

namespace epi
{

namespace details
{
    template <typename... Ts>
    struct make_void {
        typedef void type;
    };
} // namespace details

/**
 * utility for meta programming that produces void for any valid type.
 */
template <class... Ts>
using void_t = typename details::make_void<Ts...>::type;

namespace details
{
    template <template <class...> class Expr, class X, class... T>
    struct is_expression_valid : std::false_type {
    };

    template <template <class...> class Expr, class... T>
    struct is_expression_valid<Expr, void_t<Expr<T...>>, T...> : std::true_type {
    };
} // namespace details

/**
 * defines static constant value = true if Expr<T...> produces a valid type.
 * This technique is sometimes called detection idiom.
 * Can be used to detect e.g. a specific member functions:
 * @code
 *  struct Foo
 *  {
 *    void mem_fun(){}
 *  };
 * 
 *  template<class T>
 *  using mem_fun_t = decltype(std::declval<T>().mem_fun());
 * 
 *  static_assert(is_expression_valid<mem_fun_t, Foo>);
 * @endcode
 */
template <template <class...> class Expr, class... T>
struct is_expression_valid : details::is_expression_valid<Expr, void, T...>
{
};

} // namespace epi

#endif