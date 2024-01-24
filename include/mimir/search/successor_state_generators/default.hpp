#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_

#include "template.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
class DefaultSSGTag : public SSGBaseTag {};


/**
 * Aliases
*/
template<IsPlanningModeTag P>
using DefaultSSG = SSG<WrappedSSGTag<DefaultSSGTag, P>>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P>
struct TypeTraits<DefaultSSG<P>> {
    using PlanningModeTag = P;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_DEFAULT_HPP_
