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
 * 
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
*/
struct ActionBaseTag {};

template<typename DerivedTag>
concept IsActionTag = std::derived_from<DerivedTag, ActionBaseTag>;


/**
 * Wrapper dispatch class.
 *
 * Wrap the tag and variable number of template arguments.
 * 
 * Define required input template parameters using SFINAE
 * in the declaration file of your derived class.
*/
template<IsActionTag A, IsPlanningModeTag P, IsStateTag S>
struct WrappedActionTag {};

template<typename T>
struct is_wrapped_action_tag : std::false_type {};

template<typename T>
concept IsWrappedActionTag = is_wrapped_action_tag<T>::value;


} // namespace mimir



#endif  // MIMIR_SEARCH_ACTIONS_TEMPLATE_HPP_
