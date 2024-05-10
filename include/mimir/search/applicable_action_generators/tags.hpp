#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_TAGS_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_TAGS_HPP_

#include "mimir/search/states/tags.hpp"

#include <concepts>

namespace mimir
{
/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsStateTag S>
struct LiftedAAGDispatcher
{
};

template<IsStateTag S>
struct GroundedAAGDispatcher
{
};

template<typename T>
struct is_lifted_aag_dispatcher : std::false_type
{
};

template<typename T>
struct is_grounded_aag_dispatcher : std::false_type
{
};

template<IsStateTag S>
struct is_lifted_aag_dispatcher<LiftedAAGDispatcher<S>> : std::true_type
{
};

template<IsStateTag S>
struct is_grounded_aag_dispatcher<GroundedAAGDispatcher<S>> : std::true_type
{
};

template<typename T>
concept IsAAGDispatcher = is_lifted_aag_dispatcher<T>::value || is_grounded_aag_dispatcher<T>::value;

}

#endif
