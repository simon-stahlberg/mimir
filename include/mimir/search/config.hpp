#ifndef MIMIR_SEARCH_CONFIG_HPP_
#define MIMIR_SEARCH_CONFIG_HPP_

#include "type_traits.hpp"

#include <concepts>


namespace mimir
{

/**
 * Configuration classes to dispatch the grounded or lifted implementation
*/
struct GroundedTag {};

struct LiftedTag {};


/**
 * Concepts
*/
template<typename Tag>
concept IsPlanningModeTag = std::is_same_v<Tag, GroundedTag> || std::is_same_v<Tag, LiftedTag>;

template<typename Tag>
concept HasPlanningModeTag = requires {
    // Test that the type trait was defined
    typename TypeTraits<Tag>::PlanningMode;
    // Test that the type trait is a config
    requires IsPlanningModeTag<typename TypeTraits<Tag>::PlanningMode>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_CONFIG_HPP_
