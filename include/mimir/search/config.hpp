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
    // Check that the type trait was defined
    typename TypeTraits<T>::Config;
    // Check that the type trait is a config
    requires IsConfig<typename TypeTraits<T>::Config>;
};

template<typename T1, typename T2>
concept HaveEqualConfig = requires {
    requires HasConfig<T1>;
    requires HasConfig<T2>;
    requires std::is_same_v<typename TypeTraits<T1>::Config, typename TypeTraits<T2>::Config>;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_CONFIG_HPP_
