#ifndef MIMIR_SEARCH_CONFIG_HPP_
#define MIMIR_SEARCH_CONFIG_HPP_

#include "type_traits.hpp"

#include <concepts>


namespace mimir
{

/**
 * ID classes
*/
struct Grounded { };

struct Lifted { };


/**
 * Concepts
*/
template<typename Tag>
concept Config = std::is_same_v<Tag, Grounded> || std::is_same_v<Tag, Lifted>;


template<typename T>
concept hasConfig = requires {
    typename TypeTraits<T>::ConfigTagType;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_CONFIG_HPP_
