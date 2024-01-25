#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_

#include "template.hpp"


namespace mimir
{

/**
 * Derived ID class.
 * 
 * Define name and template parameters of your own implementation.
*/
class DefaultSSGTag : public SSGBaseTag {};


/**
 * Wrapper dispatch class.
 * 
 * Define the required template arguments of your implementation.
*/
template<IsSSGTag SSG, IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct is_wrapped_ssg_tag<WrappedSSGTag<SSG, P, S, A>> : std::true_type {};


/**
 * Aliases
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
using DefaultSSG = SSG<WrappedSSGTag<DefaultSSGTag, P, S, A>>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct TypeTraits<DefaultSSG<P, S, A>> {
    using PlanningModeTag = P;
    using StateTag = S;
    using ActionTag = A;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_
