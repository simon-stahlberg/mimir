#ifndef MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_

#include "config.hpp"
#include "type_traits.hpp"
#include "state_base.hpp"
#include "declarations.hpp"
#include "search_node.hpp"

#include "../buffers/buffer.hpp"
#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"

#include <sstream>


namespace mimir {

template<Config C>
class SearchNodeBuilder {
private:
    SearchNodeStatus m_status;
    int m_g_value;
    State<C> m_parent_state;
    GroundAction m_creating_action;

    BinaryBuffer m_buffer;

public:
    void set_status(SearchNodeStatus status) { m_status = status; }
    void set_g_value(int g_value) { m_g_value = g_value; }
    void set_parent_state(State<C> parent_state) { m_parent_state = parent_state; }
    void set_ground_action(GroundAction creating_action) { m_creating_action = creating_action; }

    void finish() { 
        m_buffer.write(reinterpret_cast<const char*>(&m_status), sizeof(SearchNodeStatus));
        m_buffer.write(reinterpret_cast<const char*>(&m_g_value), sizeof(int));
        m_buffer.write(reinterpret_cast<const char*>(&m_parent_state), sizeof(State<C>));
        m_buffer.write(reinterpret_cast<const char*>(&m_creating_action), sizeof(GroundAction));
    }

    const char* get_buffer_pointer() const { return m_buffer.get_data(); }
    int get_size() const { return m_buffer.get_size(); }

    void clear() { m_buffer.clear(); }
};




} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_
