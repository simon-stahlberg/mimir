#ifndef MIMIR_SEARCH_STATES_VECTOR_HPP_
#define MIMIR_SEARCH_STATES_VECTOR_HPP_

#include "template.hpp"


namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
class VectorStateTag : public StateBaseTag {};


/**
 * Type traits.
*/
template<IsPlanningModeTag P>
struct TypeTraits<Builder<StateDispatcher<VectorStateTag, P>>> {
    using PlanningModeTag = P;
};

template<IsPlanningModeTag P>
struct TypeTraits<View<StateDispatcher<VectorStateTag, P>>> {
    using PlanningModeTag = P;
};


}

#endif