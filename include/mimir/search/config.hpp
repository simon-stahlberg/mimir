#ifndef MIMIR_SEARCH_CONFIG_HPP_
#define MIMIR_SEARCH_CONFIG_HPP_

#include "type_traits.hpp"

#include <concepts>


namespace mimir
{

/**
 * Configuration classes to dispatch the grounded or lifted implementation
*/
struct Grounded { };

struct Lifted { };


/**
 * Concepts
*/
template<typename C>
concept IsConfig = std::is_same_v<C, Grounded> || std::is_same_v<C, Lifted>;

template<typename T>
concept HasConfig = requires {
    // Test that the type trait was defined
    typename TypeTraits<T>::Config;
    // Test that the type trait is a config
    requires IsConfig<typename TypeTraits<T>::Config>;
};

template<typename T1, typename T2>
concept HaveEqualConfig =
    // Test all 4 cases where T1 or T2 can directly be a config or ae other types which must have a nested config.
    (IsConfig<T1> && IsConfig<T2> && std::is_same_v<T1, T2>) ||
    (IsConfig<T1> && requires { typename TypeTraits<T2>::Config; requires std::is_same_v<T1, typename TypeTraits<T2>::Config>; }) ||
    (IsConfig<T2> && requires { typename TypeTraits<T1>::Config; requires std::is_same_v<typename TypeTraits<T1>::Config, T2>; }) ||
    (requires {
        requires HasConfig<T1>;
        requires HasConfig<T2>;
        requires std::is_same_v<typename TypeTraits<T1>::Config, typename TypeTraits<T2>::Config>;
    });


}  // namespace mimir

#endif  // MIMIR_SEARCH_CONFIG_HPP_
