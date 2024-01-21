#ifndef MIMIR_SEARCH_CONFIG_HPP_
#define MIMIR_SEARCH_CONFIG_HPP_

#include <concepts>


namespace mimir
{

struct GroundedTag { };

struct LiftedTag { };

template<typename T>
concept Config = std::is_same_v<T, GroundedTag> || std::is_same_v<T, LiftedTag>;

}  // namespace mimir

#endif  // MIMIR_SEARCH_CONFIG_HPP_
