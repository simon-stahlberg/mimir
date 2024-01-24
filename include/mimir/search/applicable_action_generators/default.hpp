#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_

#include "template.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
class DefaultAAGTag : public AAGBaseTag {};


/**
 * Aliases
*/
template<IsPlanningModeTag P>
using DefaultAAG = AAG<WrappedAAGTag<DefaultAAGTag, P>>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P>
struct TypeTraits<DefaultAAG<P>> {
    using PlanningModeTag = P;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_HPP_
