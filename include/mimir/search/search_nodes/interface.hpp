#ifndef MIMIR_SEARCH_SEARCH_NODES_INTERFACE_HPP_
#define MIMIR_SEARCH_SEARCH_NODES_INTERFACE_HPP_

#include "../config.hpp"
#include "../type_traits.hpp"

#include "../../buffer/view_base.hpp"
#include "../../buffer/byte_stream_utils.hpp"


namespace mimir
{

/**
 * Data types
*/
enum SearchNodeStatus {NEW = 0, OPEN = 1, CLOSED = 2, DEAD_END = 3};


/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
*/
struct SearchNodeTag {};

template<typename DerivedTag>
concept IsSearchNodeTag = std::derived_from<DerivedTag, SearchNodeTag>;


}

#endif
