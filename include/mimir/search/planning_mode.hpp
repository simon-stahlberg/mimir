#ifndef MIMIR_SEARCH_CONFIG_HPP_
#define MIMIR_SEARCH_CONFIG_HPP_

#include <concepts>

namespace mimir
{

/**
 * Configuration classes to dispatch the grounded or lifted implementation
 */
struct GroundedTag
{
};

struct LiftedTag
{
};

/**
 * Concepts
 */
template<typename Tag>
concept IsPlanningModeTag = std::is_same_v<Tag, GroundedTag> || std::is_same_v<Tag, LiftedTag>;

}

#endif
