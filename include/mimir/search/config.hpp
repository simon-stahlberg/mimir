#ifndef MIMIR_SEARCH_CONFIG_HPP_
#define MIMIR_SEARCH_CONFIG_HPP_

#include <concepts>


namespace mimir
{

struct Grounded { };

struct Lifted { };

template<typename T>
concept Config = std::is_same_v<T, Grounded> || std::is_same_v<T, Lifted>;

}  // namespace mimir

#endif  // MIMIR_SEARCH_CONFIG_HPP_
