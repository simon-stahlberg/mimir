#ifndef MIMIR_SEARCH_STATES_DEFAULT_HPP_
#define MIMIR_SEARCH_STATES_DEFAULT_HPP_

#include "../state_template.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningModeTag P>
class DefaultStateTag : public StateBaseTag {};


/**
 * Aliases
*/
template<IsPlanningModeTag P>
using DefaultStateBuilder = View<DefaultStateTag<P>>;

template<IsPlanningModeTag P>
using DefaultStateView = View<DefaultStateTag<P>>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P>
struct TypeTraits<DefaultStateTag<P>> {
    using PlanningMode = P;
};

template<IsPlanningModeTag P>
struct TypeTraits<DefaultStateBuilder<P>> {
    using PlanningMode = P;
};

template<IsPlanningModeTag P>
struct TypeTraits<DefaultStateView<P>> {
    using PlanningMode = P;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_
