#ifndef MIMIR_SEARCH_STATES_BITSET_HPP_
#define MIMIR_SEARCH_STATES_BITSET_HPP_

#include "template.hpp"


namespace mimir
{

/**
 * Derived ID class.
 * 
 * Define name and template parameters of your own implementation.
*/
class BitsetStateTag : public StateBaseTag {};


/**
 * Wrapper dispatch class.
 * 
 * Define the required template arguments of your implementation.
*/
template<IsStateTag S, IsPlanningModeTag P>
struct is_wrapped_state_tag<WrappedStateTag<S, P>> : std::true_type {};


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
    using PlanningModeTag = P;
};

template<IsPlanningModeTag P>
struct TypeTraits<BitsetStateView<P>> {
    using PlanningModeTag = P;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_STATES_BITSET_HPP_
