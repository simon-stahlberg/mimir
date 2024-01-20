#ifndef MIMIR_SEARCH_SEARCH_NODE_VIEW_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_VIEW_HPP_

#include "config.hpp"
#include "search_node.hpp"
#include "state_base.hpp"
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
    [[nodiscard]] State<C> get_parent_state() { return self().get_parent_state_impl(); }
    [[nodiscard]] GroundAction get_ground_action() { return self().get_ground_action_impl(); }

    /* Immutable getters. */
    [[nodiscard]] const SearchNodeStatus& get_status() const { return self().get_status_impl(); }
    [[nodiscard]] const int& get_g_value() const { return self().get_g_value_impl(); }
    [[nodiscard]] const State<C> get_parent_state() const { return self().get_parent_state_impl(); }
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
    static constexpr size_t s_ground_action = s_parent_state_offset + sizeof(State<C>);

    [[nodiscard]] SearchNodeStatus& get_status_impl() {
        return *reinterpret_cast<SearchNodeStatus*>(this->get_data() + s_status_offset); }
    [[nodiscard]] int& get_g_value_impl() {
        return  *reinterpret_cast<int*>(this->get_data() + s_g_value_offset); }
    [[nodiscard]] State<C> get_parent_state_impl() {
        return reinterpret_cast<State<C>>(*reinterpret_cast<uintptr_t*>(this->get_data() + s_parent_state_offset)); }
    [[nodiscard]] GroundAction get_ground_action_impl() {
        return reinterpret_cast<GroundAction>(*reinterpret_cast<uintptr_t*>(this->get_data() + s_ground_action)); }

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
