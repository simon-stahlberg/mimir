#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_

#include "../successor_state_generator_template.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningMode P>
class DefaultSuccessorStateGeneratorTag : public SuccessorStateGeneratorBaseTag {};


/**
 * Aliases
*/
template<IsPlanningMode P>
using DefaultSuccessorStateGenerator = SuccessorStateGenerator<DefaultSuccessorStateGeneratorTag<P>>;


/**
 * Type traits.
*/
template<IsPlanningMode P>
struct TypeTraits<DefaultSuccessorStateGeneratorTag<P>> {
    using PlanningMode = P;
};

template<IsPlanningMode P>
struct TypeTraits<DefaultSuccessorStateGenerator<P>> {
    using PlanningMode = P;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_
