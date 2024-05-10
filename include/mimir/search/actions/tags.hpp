#ifndef MIMIR_SEARCH_ACTIONS_TAGS_HPP_
#define MIMIR_SEARCH_ACTIONS_TAGS_HPP_

#include "mimir/search/states/tags.hpp"

#include <concepts>

namespace mimir
{
/**
 * Dispatcher class.
 *
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsStateTag S>
struct ActionDispatcher
{
};

template<typename T>
struct is_action_dispatcher : std::false_type
{
};

template<IsStateTag S>
struct is_action_dispatcher<ActionDispatcher<S>> : std::true_type
{
};

template<typename T>
concept IsActionDispatcher = is_action_dispatcher<T>::value;

}

#endif