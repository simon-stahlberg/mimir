#ifndef MIMIR_SEARCH_ACTIONS_DEFAULT_HPP_
#define MIMIR_SEARCH_ACTIONS_DEFAULT_HPP_

#include "template.hpp"


namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
class DefaultActionTag : public ActionBaseTag {};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<Builder<ActionDispatcher<DefaultActionTag, P, S>>>
{
    using PlanningModeTag = P;
    using StateTag = S;
};

template<IsPlanningModeTag P, IsBitsetStateTag S>
struct TypeTraits<View<ActionDispatcher<DefaultActionTag, P, S>>>
{
    using PlanningModeTag = P;
    using StateTag = S;
};


}

#endif
