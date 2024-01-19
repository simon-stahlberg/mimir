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
    static constexpr size_t s_status_offset = 0;
    static constexpr size_t s_g_value_offset = s_status_offset + sizeof(SearchNodeStatus);
    static constexpr size_t s_parent_state_offset = s_g_value_offset + sizeof(int);
    static constexpr size_t s_ground_action = s_parent_state_offset + sizeof(State<C>);

    SearchNodeStatus get_status() { return reinterpret_cast<SearchNodeStatus>(this + s_status_offset); }
    int get_g_value() { return reinterpret_cast<int>(this + s_g_value_offset); }
    State<C> get_parent_state() { return reinterpret_cast<State<C>>(this + s_parent_state_offset); }
    GroundAction get_ground_action() { return reinterpret_cast<GroundAction>(this + s_ground_action); }
};




} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_HPP_
