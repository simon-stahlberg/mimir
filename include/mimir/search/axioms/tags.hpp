#ifndef MIMIR_SEARCH_AXIOMS_TAGS_HPP_
#define MIMIR_SEARCH_AXIOMS_TAGS_HPP_

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
struct AxiomDispatcher
{
};

template<typename T>
struct is_axiom_dispatcher : std::false_type
{
};

template<IsStateTag S>
struct is_axiom_dispatcher<AxiomDispatcher<S>> : std::true_type
{
};

template<typename T>
concept IsAxiomDispatcher = is_axiom_dispatcher<T>::value;

}

#endif