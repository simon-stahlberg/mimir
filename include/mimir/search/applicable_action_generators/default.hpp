#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_

#include "../applicable_action_generator_template.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningMode P>
class DefaultApplicableActionGeneratorTag : public ApplicableActionGeneratorBaseTag {};


/**
 * Aliases
*/
template<IsPlanningMode P>
using DefaultApplicableActionGenerator = ApplicableActionGenerator<DefaultApplicableActionGeneratorTag<P>>;


/**
 * Type traits.
*/
template<IsPlanningMode P>
struct TypeTraits<DefaultApplicableActionGeneratorTag<P>> {
    using PlanningMode = P;
};

template<IsPlanningMode P>
struct TypeTraits<DefaultApplicableActionGenerator<P>> {
    using PlanningMode = P;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_
