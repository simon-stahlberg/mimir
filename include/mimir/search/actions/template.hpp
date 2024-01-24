#ifndef MIMIR_SEARCH_ACTIONS_TEMPLATE_HPP_
#define MIMIR_SEARCH_ACTIONS_TEMPLATE_HPP_

#include "../config.hpp"
#include "../type_traits.hpp"

#include "../../buffer/view_base.hpp"
#include "../../buffer/byte_stream_utils.hpp"


namespace mimir {

/**
 * ID class
*/
struct ActionBaseTag {};

template<typename DerivedTag>
concept IsActionTag = std::derived_from<DerivedTag, ActionBaseTag>;




} // namespace mimir



#endif  // MIMIR_SEARCH_ACTIONS_TEMPLATE_HPP_
