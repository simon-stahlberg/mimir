#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_INTERFACE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_INTERFACE_HPP_

#include "../interface.hpp"


namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
struct DefaultAAGTag : public AAGTag {};


/**
 * No additional interface required.
*/


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<AAG<AAGDispatcher<DefaultAAGTag, P, S>>>
{
    using PlanningModeTag = P;
    using StateTag = S;
};


}

#endif
