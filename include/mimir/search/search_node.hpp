#ifndef MIMIR_SEARCH_SEARCH_NODE_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_HPP_

#include "config.hpp"
#include "type_traits.hpp"
#include "state_view_base.hpp"

#include "../buffer/view_base.hpp"
#include "../common/mixins.hpp"

#include <mimir/formalism/problem/declarations.hpp>

#include <cstdint>


namespace mimir {

enum SearchNodeStatus {NEW = 0, OPEN = 1, CLOSED = 2, DEAD_END = 3};

// Common interface, do not add members to keep the view lightweight.
template<typename Derived>
class SearchNodeBase {
private:
    SearchNodeBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    // Define common interface for states.
};


// ID class for a search node
template<Config C>
struct SearchNode { };


// View contains a general implementation.
template<Config C>
class View<SearchNode<C>> : public SearchNodeBase<View<SearchNode<C>>> {
private:
    static constexpr size_t s_status_offset = 0;
    static constexpr size_t s_g_value_offset = s_status_offset + sizeof(SearchNodeStatus);
    static constexpr size_t s_parent_state_offset = s_g_value_offset + sizeof(int);
    static constexpr size_t s_ground_action = s_parent_state_offset + sizeof(State<C>);

public:
    View(char* data, size_t size) : m_data(data), m_size(size) { }

    SearchNodeStatus& get_status() { return reinterpret_cast<SearchNodeStatus&>(this + s_status_offset); }
    const SearchNodeStatus& get_status() const { return reinterpret_cast<const SearchNodeStatus&>(this + s_status_offset); }

    int& get_g_value() { return reinterpret_cast<int&>(this + s_g_value_offset); }
    const int& get_g_value() const { return reinterpret_cast<const int&>(this + s_g_value_offset); }

    View<State<C>>& get_parent_state() { return reinterpret_cast<View<State<C>>&>(this + s_parent_state_offset); }
    const View<State<C>>& get_parent_state() const { return reinterpret_cast<const View<State<C>>&>(this + s_parent_state_offset); }

    GroundAction& get_ground_action() { return reinterpret_cast<GroundAction&>(this + s_ground_action); }
    const GroundAction& get_ground_action() const { return reinterpret_cast<GroundAction&>(this + s_ground_action); }
};


} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_HPP_
