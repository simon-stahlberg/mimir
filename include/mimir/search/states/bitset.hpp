#ifndef MIMIR_SEARCH_STATES_BITSET_HPP_
#define MIMIR_SEARCH_STATES_BITSET_HPP_

#include "interface.hpp"


namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
class BitsetStateTag : public StateBaseTag {};

template<typename Tag>
concept IsBitsetStateTag = std::is_same_v<Tag, BitsetStateTag>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P>
struct TypeTraits<Builder<StateDispatcher<BitsetStateTag, P>>>
{
    using PlanningModeTag = P;
};

template<IsPlanningModeTag P>
struct TypeTraits<View<StateDispatcher<BitsetStateTag, P>>>
{
    using PlanningModeTag = P;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_STATES_BITSET_HPP_