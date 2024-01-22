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
concept hasConfig = requires {
    // Check that the type trait was defined
    typename TypeTraits<T>::Config;
    // Check that the type trait is a config
    requires IsConfig<typename TypeTraits<T>::Config>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_CONFIG_HPP_
