#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_INTERFACE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_INTERFACE_HPP_

#include "../interface.hpp"


namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
class DefaultSSGTag : public SSGTag {};


/**
 * No additional interface required.
*/


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<SSG<SSGDispatcher<DefaultSSGTag, P, S>>>
{
    using PlanningModeTag = P;
    using StateTag = S;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_
