#ifndef MIMIR_SEARCH_ACTIONS_TEMPLATE_HPP_
#define MIMIR_SEARCH_ACTIONS_TEMPLATE_HPP_

#include "../config.hpp"
#include "../states.hpp"
#include "../type_traits.hpp"

#include "../../buffer/view_base.hpp"
#include "../../buffer/byte_stream_utils.hpp"


namespace mimir {

/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
*/
struct ActionBaseTag {};

template<typename DerivedTag>
concept IsActionTag = std::derived_from<DerivedTag, ActionBaseTag>;


/**
 * Wrapper class use for the actual dispatch.
 *
 * Use additional template arguments from the parent template.
*/
template<IsActionTag A, IsPlanningModeTag P, IsStateTag S>
struct WrappedActionTag {};

template<typename T>
struct is_wrapped_action_tag : std::false_type {};

template<IsActionTag A, IsPlanningModeTag P, IsStateTag S>
struct is_wrapped_action_tag<WrappedActionTag<A, P, S>> : std::true_type {};

template<typename T>
concept IsWrappedActionTag = is_wrapped_action_tag<T>::value;


} // namespace mimir



#endif  // MIMIR_SEARCH_ACTIONS_TEMPLATE_HPP_
