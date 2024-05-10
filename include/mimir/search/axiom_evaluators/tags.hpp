#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_TAGS_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_TAGS_HPP_

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
struct AEDispatcher
{
};

template<typename T>
struct is_ae_dispatcher : std::false_type
{
};

template<IsStateTag S>
struct is_ae_dispatcher<AEDispatcher<S>> : std::true_type
{
};

template<typename T>
concept IsAEDispatcher = is_ae_dispatcher<T>::value;

}

#endif
