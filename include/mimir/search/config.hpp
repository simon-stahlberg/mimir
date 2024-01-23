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

template<typename Tag1, typename Tag2>
concept HaveEqualPlanningModeTags =
    // Test all 4 cases where T1 or T2 can directly be a config or ae other types which must have a nested config.
    (IsPlanningModeTag<Tag1> && IsPlanningModeTag<Tag2> && std::is_same_v<Tag1, Tag2>) ||
    (IsPlanningModeTag<Tag1> && requires { typename TypeTraits<Tag2>::PlanningMode; requires std::is_same_v<Tag1, typename TypeTraits<Tag2>::PlanningMode>; }) ||
    (IsPlanningModeTag<Tag2> && requires { typename TypeTraits<Tag1>::PlanningMode; requires std::is_same_v<typename TypeTraits<Tag1>::PlanningMode, Tag2>; }) ||
    (requires {
        requires HasPlanningModeTag<Tag1>;
        requires HasPlanningModeTag<Tag2>;
        requires std::is_same_v<typename TypeTraits<Tag1>::PlanningMode, typename TypeTraits<Tag2>::PlanningMode>;
    });


}  // namespace mimir

#endif  // MIMIR_SEARCH_CONFIG_HPP_
