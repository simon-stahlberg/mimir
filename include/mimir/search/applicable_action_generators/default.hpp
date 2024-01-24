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
template<IsPlanningModeTag P, IsStateTag S>
using DefaultAAG = AAG<WrappedAAGTag<DefaultAAGTag, P>>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<DefaultAAG<P, S>> {
    using PlanningModeTag = P;
    using StateTag = S;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_
