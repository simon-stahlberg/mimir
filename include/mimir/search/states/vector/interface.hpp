#ifndef MIMIR_SEARCH_STATES_VECTOR_INTERFACE_HPP_
#define MIMIR_SEARCH_STATES_VECTOR_INTERFACE_HPP_

#include "../interface.hpp"

namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
 */
class VectorStateTag : public StateTag
{
};

/**
 * Type traits.
 */
template<IsPlanningModeTag P>
struct TypeTraits<Builder<StateDispatcher<VectorStateTag, P>>>
{
    using PlanningModeTag = P;
};

template<IsPlanningModeTag P>
struct TypeTraits<View<StateDispatcher<VectorStateTag, P>>>
{
    using PlanningModeTag = P;
};

}

#endif