#ifndef MIMIR_SEARCH_CONFIG_HPP_
#define MIMIR_SEARCH_CONFIG_HPP_

#include <concepts>


namespace mimir
{

struct GroundedTag { };

struct LiftedTag { };

template<typename Tag>
concept Config = std::is_same_v<Tag, GroundedTag> || std::is_same_v<Tag, LiftedTag>;

}  // namespace mimir

#endif  // MIMIR_SEARCH_CONFIG_HPP_
