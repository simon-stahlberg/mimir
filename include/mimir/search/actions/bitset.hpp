#ifndef MIMIR_SEARCH_ACTIONS_BITSET_HPP_
#define MIMIR_SEARCH_ACTIONS_BITSET_HPP_

#include "interface.hpp"


namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
class BitsetActionTag : public ActionTag {};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<Builder<ActionDispatcher<BitsetActionTag, P, S>>>
{
    using PlanningModeTag = P;
    using StateTag = S;
};

template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<View<ActionDispatcher<BitsetActionTag, P, S>>>
{
    using PlanningModeTag = P;
    using StateTag = S;
};


}

#endif
