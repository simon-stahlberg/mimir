#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_

#include "template.hpp"


namespace mimir
{

/**
 * Derived ID class.
 * 
 * Define name and template parameters of your own implementation.
*/
struct DefaultAAGTag : public AAGBaseTag {};


/**
 * Wrapper dispatch class.
 * 
 * Define the required template arguments of your implementation.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct is_wrapped_aag_tag<WrappedAAGTag<DefaultAAGTag, P, S, A>> : std::true_type {};


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
