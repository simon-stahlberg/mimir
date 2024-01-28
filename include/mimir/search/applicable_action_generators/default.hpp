#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_

#include "interface.hpp"


namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
struct DefaultAAGTag : public AAGTag {};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct TypeTraits<AAG<AAGDispatcher<DefaultAAGTag, P, S, A>>>
{
    using PlanningModeTag = P;
    using StateTag = S;
    using ActionTag = A;
};


}

#endif
