#ifndef MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_

#include "config.hpp"
#include "type_traits.hpp"
#include "state_base.hpp"
#include "declarations.hpp"
#include "search_node.hpp"

#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"

#include <sstream>


namespace mimir {

template<Config C>
class SearchNodeBuilder {
private:

    SearchNodeStatus status;
    int g_value;
    State<C> parent_state;
    GroundAction creating_action;

    std::ostringstream m_buffer;

public:
    void set_status(SearchNodeStatus status);
    void set_g_value(int g_value);
    void set_parent_state(State<C> state);
    void set_ground_action(GroundAction action);

    void finish();

    const char* get_buffer_pointer() const;
    int get_size() const;
};




} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_
