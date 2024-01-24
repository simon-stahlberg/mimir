#ifndef MIMIR_SEARCH_SEARCH_NODES_DEFAULT_HPP_
#define MIMIR_SEARCH_SEARCH_NODES_DEFAULT_HPP_

#include "../search_node_template.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
class DefaultSearchNodeTag : public StateBaseTag {};


/**
 * Aliases
*/
template<IsPlanningModeTag P>
using DefaultStateBuilder = View<WrappedStateTag<DefaultStateTag, P>>;

template<IsPlanningModeTag P>
using DefaultStateView = View<WrappedStateTag<DefaultStateTag, P>>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P>
struct TypeTraits<DefaultStateBuilder<P>> {
    using PlanningMode = P;
};

template<IsPlanningModeTag P>
struct TypeTraits<DefaultStateView<P>> {
    using PlanningMode = P;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_STATES_DEFAULT_HPP_
