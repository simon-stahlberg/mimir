#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_BITSET_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_BITSET_HPP_

#include "template.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
class DefaultSuccessorStateGeneratorTag : public SuccessorStateGeneratorBaseTag {};


/**
 * Aliases
*/
template<IsPlanningModeTag P>
using DefaultSuccessorStateGenerator = SuccessorStateGenerator<WrappedSuccessorStateGeneratorTag<DefaultSuccessorStateGeneratorTag, P>>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P>
struct TypeTraits<DefaultSuccessorStateGenerator<P>> {
    using PlanningModeTag = P;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_BITSET_HPP_
