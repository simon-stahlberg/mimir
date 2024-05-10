#ifndef MIMIR_SEARCH_STATES_TAGS_HPP_
#define MIMIR_SEARCH_STATES_TAGS_HPP_

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
struct StateTag
{
};

template<typename DerivedTag>
concept IsStateTag = std::derived_from<DerivedTag, StateTag>;

/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsStateTag S>
struct StateDispatcher
{
};

template<typename T>
struct is_state_dispatcher : std::false_type
{
};

template<IsStateTag S>
struct is_state_dispatcher<StateDispatcher<S>> : std::true_type
{
};

template<typename T>
concept IsStateDispatcher = is_state_dispatcher<T>::value;

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
 */
class DenseStateTag : public StateTag
{
};

}

#endif
