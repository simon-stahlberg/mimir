#ifndef MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_

#include "config.hpp"
#include "type_traits.hpp"
#include "state_view_base.hpp"
#include "declarations.hpp"
#include "search_node_view.hpp"

#include "../buffer/builder_base.hpp"
#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"

#include <sstream>


namespace mimir {

template<Config C>
class Builder<SearchNode<C>> {
private:
    SearchNodeStatus m_status;
    int m_g_value;
    View<State<C>> m_parent_state;
    GroundAction m_creating_action;

    void finish_impl() {
        m_buffer.write(reinterpret_cast<const char*>(&m_status), sizeof(SearchNodeStatus));
        m_buffer.write(reinterpret_cast<const char*>(&m_g_value), sizeof(int));
        m_buffer.write(reinterpret_cast<const char*>(&m_parent_state), sizeof(View<State<C>>));
        m_buffer.write(reinterpret_cast<const char*>(&m_creating_action), sizeof(GroundAction));
    }

public:
    void set_status(SearchNodeStatus status) { m_status = status; }
    void set_g_value(int g_value) { m_g_value = g_value; }
    void set_parent_state(View<State<C>> parent_state) { m_parent_state = parent_state; }
    void set_ground_action(GroundAction creating_action) { m_creating_action = creating_action; }
};




} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_
