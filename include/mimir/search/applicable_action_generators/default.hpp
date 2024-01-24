#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_

#include "template.hpp"


namespace mimir
{

/**
 * Derived ID dispatch class.
 *
 * Add template parameters if needed
*/
struct DefaultAAGTag : public AAGBaseTag {};


/**
 * Aliases
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
using DefaultAAG = AAG<WrappedAAGTag<DefaultAAGTag, P, S, A>>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct TypeTraits<DefaultAAG<P, S, A>> {
    using PlanningModeTag = P;
    using StateTag = S;
    using ActionTag = A;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_
