#ifndef MIMIR_SEARCH_SEARCH_NODE_VIEW_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_VIEW_HPP_

#include "config.hpp"
#include "search_node.hpp"
#include "state_view.hpp"
#include "type_traits.hpp"

#include "declarations.hpp"
#include "../buffer/view_base.hpp"
#include "../formalism/problem/declarations.hpp"

#include <cstdint>
#include <iostream>


namespace mimir {

/**
 * Interface class
*/
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
    /* Mutable getters. */
    [[nodiscard]] SearchNodeStatus& get_status() { return self().get_status_impl(); }
    [[nodiscard]] int& get_g_value() { return self().get_g_value_impl(); }
    [[nodiscard]] View<State<C>> get_parent_state() { return self().get_parent_state_impl(); }
    [[nodiscard]] GroundAction get_ground_action() { return self().get_ground_action_impl(); }

    /* Immutable getters. */
    [[nodiscard]] const SearchNodeStatus& get_status() const { return self().get_status_impl(); }
    [[nodiscard]] const int& get_g_value() const { return self().get_g_value_impl(); }
    [[nodiscard]] const View<State<C>> get_parent_state() const { return self().get_parent_state_impl(); }
    [[nodiscard]] const GroundAction get_ground_action() const { return self().get_ground_action_impl(); }
};


/**
 * Implementation class (general)
*/
template<Config C>
class View<SearchNode<C>> : public ViewBase<View<SearchNode<C>>>, public SearchNodeViewBase<View<SearchNode<C>>> {
private:
    static constexpr size_t s_status_offset = sizeof(DataSizeType);
    static constexpr size_t s_g_value_offset = s_status_offset + sizeof(SearchNodeStatus);
    static constexpr size_t s_parent_state_offset = s_g_value_offset + sizeof(int);
    static constexpr size_t s_ground_action = s_parent_state_offset + sizeof(char*);

    /* Implement ViewBase has no interface: no methods must be overriden */

    /* Implement SearchNodeViewBase interface */
    [[nodiscard]] SearchNodeStatus& get_status_impl() {
        return *reinterpret_cast<SearchNodeStatus*>(this->get_data() + s_status_offset); }

    [[nodiscard]] int& get_g_value_impl() {
        return  *reinterpret_cast<int*>(this->get_data() + s_g_value_offset); }

    [[nodiscard]] View<State<C>> get_parent_state_impl() {
        uintptr_t ptr_address = *reinterpret_cast<uintptr_t*>(this->get_data() + s_parent_state_offset);
        char* data_address = reinterpret_cast<char*>(ptr_address);
        return View<State<C>>(data_address);
    }
    [[nodiscard]] GroundAction get_ground_action_impl() {
        uintptr_t ptr_address = *reinterpret_cast<uintptr_t*>(this->get_data() + s_ground_action);
        GroundAction action_ptr = reinterpret_cast<GroundAction>(ptr_address);
        return action_ptr;
    }

    friend class SearchNodeViewBase<View<SearchNode<C>>>;

public:
    /// @brief Create a view on a SearchNode.
    explicit View(char* data) : ViewBase<View<SearchNode<C>>>(data) { }
};


/**
 * Type traits
*/
template<Config C>
struct TypeTraits<View<SearchNode<C>>> {
    using ConfigType = C;
};


} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_VIEW_HPP_
