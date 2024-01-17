#ifndef MIMIR_SEARCH_SEARCH_NODE_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_HPP_

#include "type_traits.hpp"
#include "state_base.hpp"

#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"


namespace mimir {

template<typename Config>
class SearchNode {
private:
    enum NodeStatus {NEW = 0, OPEN = 1, CLOSED = 2, DEAD_END = 3};

    int status : 2;
    int g_value;
    ID<State<Config>> parent_state_id;
    GroundAction creating_action;

public:
    SearchNode();

    bool is_new() const;
    bool is_open() const;
    bool is_closed() const;
    bool is_dead_end() const;

    int get_g() const;

    void open_initial();
    void open(const SearchNode& parent_node,
              const GroundAction& parent_action,
              int adjusted_cost);
    void reopen(const SearchNode& parent_node,
                const GroundAction& parent_action,
                int adjusted_cost);
    void update_parent(const SearchNode& parent_node,
                       const GroundAction& parent_action,
                       int adjusted_cost);
    void close();
    void mark_as_dead_end();
};


template<typename Config>
SearchNode<Config>::SearchNode()
    : status(SearchNode<Config>::NodeStatus::CLOSED)
    , g_value(0)
    , parent_state_id(ID<State<Config>>::no_id) { }



} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_HPP_
