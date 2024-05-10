#ifndef MIMIR_SEARCH_HEURISTICS_TAGS_HPP_
#define MIMIR_SEARCH_HEURISTICS_TAGS_HPP_

#include "mimir/search/states/tags.hpp"

#include <concepts>

namespace mimir
{
/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
 */
struct HeuristicTag
{
};

template<class DerivedTag>
concept IsHeuristicTag = std::derived_from<DerivedTag, HeuristicTag>;

/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsHeuristicTag H, IsStateTag S>
struct HeuristicDispatcher
{
};

template<typename T>
struct is_heuristic_dispatcher : std::false_type
{
};

template<IsHeuristicTag H, IsStateTag S>
struct is_heuristic_dispatcher<HeuristicDispatcher<H, S>> : std::true_type
{
};

template<typename T>
concept IsHeuristicDispatcher = is_heuristic_dispatcher<T>::value;

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
 */
struct BlindTag : public HeuristicTag
{
};

}

#endif