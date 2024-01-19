#ifndef MIMIR_SEARCH_SEARCH_NODE_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_HPP_

#include "config.hpp"
#include "type_traits.hpp"
#include "state_base.hpp"

#include "declarations.hpp"
#include "../buffer/view_base.hpp"
#include "../formalism/problem/declarations.hpp"

#include <cstdint>


namespace mimir {

enum SearchNodeStatus {NEW = 0, OPEN = 1, CLOSED = 2, DEAD_END = 3};

// Interface class
template<typename Derived>
class SearchNodeViewBase {
private:
    using C = typename TypeTraits<Derived>::ConfigType;

    SearchNodeViewBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    SearchNodeStatus get_status() { return self().get_status_impl(); }
    int get_g_value() { return self().get_g_value_impl(); }
    State<C> get_parent_state() { return self().get_g_value_impl(); }
    GroundAction get_ground_action() { return self().get_ground_action(); }
};


// ID class
template<Config C> class SearchNode { };


// Implementation class (general)
template<Config C>
class View<SearchNode<C>> : public ViewBase<View<SearchNode<C>>>, public SearchNodeViewBase<View<SearchNode<C>>> {
private:
    static constexpr size_t s_status_offset = 0;
    static constexpr size_t s_g_value_offset = s_status_offset + sizeof(SearchNodeStatus);
    static constexpr size_t s_parent_state_offset = s_g_value_offset + sizeof(int);
    static constexpr size_t s_ground_action = s_parent_state_offset + sizeof(State<C>);

    SearchNodeStatus get_status_impl() { return reinterpret_cast<SearchNodeStatus>(this->get_data() + s_status_offset); }
    int get_g_value_impl() { return reinterpret_cast<int>(this->get_data() + s_g_value_offset); }
    State<C> get_parent_state_impl() { return reinterpret_cast<State<C>>(this->get_data() + s_parent_state_offset); }
    GroundAction get_ground_action_impl() { return reinterpret_cast<GroundAction>(this->get_data() + s_ground_action); }

    friend class SearchNodeViewBase<View<SearchNode<C>>>;

public:
    View(char* data, size_t size) : ViewBase<View<SearchNode<C>>>(data, size) { }
};


template<Config C>
struct TypeTraits<View<SearchNode<C>>> {
    using ConfigType = C;
};


} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_HPP_
