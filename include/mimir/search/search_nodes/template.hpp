#ifndef MIMIR_SEARCH_SEARCH_NODES_TEMPLATE_HPP_
#define MIMIR_SEARCH_SEARCH_NODES_TEMPLATE_HPP_

#include "../config.hpp"
#include "../type_traits.hpp"

#include "../../buffer/view_base.hpp"
#include "../../buffer/byte_stream_utils.hpp"


namespace mimir {

/**
 * Data types
*/
enum SearchNodeStatus {NEW = 0, OPEN = 1, CLOSED = 2, DEAD_END = 3};

using state_id_type = uint32_t;



} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODES_TEMPLATE_HPP_
