#ifndef MIMIR_SEARCH_SEARCH_NODE_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_HPP_

#include "config.hpp"
#include "type_traits.hpp"
#include "state_base.hpp"

#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"

#include <cstdint>


namespace mimir {

enum SearchNodeStatus {NEW = 0, OPEN = 1, CLOSED = 2, DEAD_END = 3};

template<Config C>
class SearchNodeImpl {
private:
    SearchNodeStatus* get_status() { return reinterpret_cast<SearchNodeStatus*>(this); }
    int32_t* get_g_value() { return reinterpret_cast<uint32_t*>(this + sizeof(SearchNodeStatus)); }
    State<C> get_parent_state() { return reinterpret_cast<State<C>>(this + sizeof(SearchNodeStatus) + sizeof(int32_t)); }
    GroundAction get_ground_action() { return reinterpret_cast<GroundAction>(this + sizeof(SearchNodeStatus) + sizeof(int32_t) + sizeof(int32_t)); }
};




} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_HPP_
