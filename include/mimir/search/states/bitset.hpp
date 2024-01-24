#ifndef MIMIR_SEARCH_STATES_BITSET_HPP_
#define MIMIR_SEARCH_STATES_BITSET_HPP_

#include "template.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
class BitsetStateTag : public StateBaseTag {};


/**
 * Aliases
*/
template<IsPlanningModeTag P>
using BitsetStateBuilder = Builder<WrappedStateTag<BitsetStateTag, P>>;

template<IsPlanningModeTag P>
using BitsetStateView = View<WrappedStateTag<BitsetStateTag, P>>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P>
struct TypeTraits<BitsetStateBuilder<P>> {
    using PlanningMode = P;
};

template<IsPlanningModeTag P>
struct TypeTraits<BitsetStateView<P>> {
    using PlanningMode = P;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_STATES_BITSET_HPP_
