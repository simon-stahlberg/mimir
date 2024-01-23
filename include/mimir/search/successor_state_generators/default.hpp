#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_

#include "../successor_state_generator_template.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningModeTag P>
class DefaultSuccessorStateGeneratorTag : public SuccessorStateGeneratorBaseTag {};


/**
 * Aliases
*/
template<IsPlanningModeTag P>
using DefaultSuccessorStateGenerator = SuccessorStateGenerator<DefaultSuccessorStateGeneratorTag<P>, P>;



}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_
