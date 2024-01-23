#ifndef MIMIR_SEARCH_CONFIG_HPP_
#define MIMIR_SEARCH_CONFIG_HPP_

#include "type_traits.hpp"

#include <concepts>


namespace mimir
{

/**
 * Configuration classes to dispatch the grounded or lifted implementation
*/
struct Grounded {};

struct Lifted {};


/**
 * Concepts
*/
template<typename T>
concept IsPlanningMode = std::is_same_v<T, Grounded> || std::is_same_v<T, Lifted>;

template<typename T>
concept HasPlanningMode = requires {
    // Test that the type trait was defined
    typename TypeTraits<T>::PlanningMode;
    // Test that the type trait is a config
    requires IsPlanningMode<typename TypeTraits<T>::PlanningMode>;
};

template<typename T1, typename T2>
concept HaveEqualPlanningMode =
    // Test all 4 cases where T1 or T2 can directly be a config or ae other types which must have a nested config.
    (IsPlanningMode<T1> && IsPlanningMode<T2> && std::is_same_v<T1, T2>) ||
    (IsPlanningMode<T1> && requires { typename TypeTraits<T2>::PlanningMode; requires std::is_same_v<T1, typename TypeTraits<T2>::PlanningMode>; }) ||
    (IsPlanningMode<T2> && requires { typename TypeTraits<T1>::PlanningMode; requires std::is_same_v<typename TypeTraits<T1>::PlanningMode, T2>; }) ||
    (requires {
        requires HasPlanningMode<T1>;
        requires HasPlanningMode<T2>;
        requires std::is_same_v<typename TypeTraits<T1>::PlanningMode, typename TypeTraits<T2>::PlanningMode>;
    });


}  // namespace mimir

#endif  // MIMIR_SEARCH_CONFIG_HPP_
