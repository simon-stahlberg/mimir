#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_

#include "../applicable_action_generator_template.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
class DefaultApplicableActionGeneratorTag : public ApplicableActionGeneratorBaseTag {};


/**
 * Aliases
*/
template<IsPlanningModeTag P>
using DefaultApplicableActionGenerator = ApplicableActionGenerator<ApplicableActionGeneratorInstantiation<DefaultApplicableActionGeneratorTag, P>>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P>
struct TypeTraits<DefaultApplicableActionGenerator<P>> {
    using PlanningMode = P;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_
