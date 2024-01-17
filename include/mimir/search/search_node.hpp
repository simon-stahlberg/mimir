#ifndef MIMIR_SEARCH_SEARCH_NODE_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_HPP_

#include "type_traits.hpp"
#include "state_base.hpp"

#include "../common/mixins.hpp"


namespace mimir {

template<typename Config>
class SearchNode {
private:
    enum NodeStatus {NEW = 0, OPEN = 1, CLOSED = 2, DEAD_END = 3};
    
    int status : 2;
    int g_value;
    ID<State<Config>> parent_state_id;
    // Action ID.

public:
};

} // namespace mimir

#endif  // MIMIR_SEARCH_SEARCH_NODE_HPP_
