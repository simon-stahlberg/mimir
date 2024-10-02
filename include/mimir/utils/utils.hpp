
#ifndef MIMIR_UTILS_UTILS_HPP
#define MIMIR_UTILS_UTILS_HPP

#include <memory>
#include <optional>
#include <type_traits>

#ifndef FWD
#define FWD(x) std::forward< decltype(x) >(x)
#endif  // FWD

namespace mimir
{

/// is_specialization checks whether T is a specialized template class of 'Template'
/// This has the limitation of
/// usage:
///     constexpr bool is_vector = is_specialization< std::vector< int >, std::vector>;
///
/// Note that this trait has 2 limitations:
///  1) Does not work with non-type parameters.
///     (i.e. templates such as std::array will not be compatible with this type trait)
///  2) Generally, template aliases do not get captured as the underlying template but as the
///     alias template. As such the following scenarios will return for an alias
///     template <typename T> using uptr = std::unique_ptr< T >:
///          specialization<uptr<int>, std::unique_ptr>;              -> true
///          specialization<uptr<int>, uptr> << std::endl;            -> false
///          specialization<std::unique_ptr<int>, std::unique_ptr>;   -> true
///          specialization<std::unique_ptr<int>, uptr>;              -> false
;
template<class T, template<class...> class Template>
struct is_specialization : std::false_type
{
};

template<template<class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : std::true_type
{
};

template<class T, template<class...> class Template>
constexpr bool is_specialization_v = is_specialization<T, Template>::value;

template<typename T>
using raw = std::remove_cvref<T>;

template<typename T>
using raw_t = typename raw<T>::type;

template<typename T>
decltype(auto) deref(T&& t)
{
    return FWD(t);
}

template<typename T>
    requires std::is_pointer_v<raw_t<T>>                               //
             or is_specialization_v<raw_t<T>, std::reference_wrapper>  //
             or is_specialization_v<raw_t<T>, std::optional>           //
             or is_specialization_v<raw_t<T>, std::shared_ptr>         //
             or is_specialization_v<raw_t<T>, std::unique_ptr>         //
             or std::input_or_output_iterator<raw_t<T>>                //
decltype(auto) deref(T&& t)
{
    if constexpr (std::is_pointer_v<raw_t<T>>                 //
                  or std::input_or_output_iterator<raw_t<T>>  //
                  or is_specialization_v<raw_t<T>, std::optional>)
    {
        return *FWD(t);
    }
    else
    {
        return deref(FWD(t).get());
    }
}

template<typename T>
using dereffed_t = decltype(deref(std::declval<T>()));
}

#endif  // MIMIR_UTILS_UTILS_HPP
