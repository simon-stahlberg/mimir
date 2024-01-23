#ifndef MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_

#include "config.hpp"
#include "search_node.hpp"
#include "lifted/state_view.hpp"
#include "grounded/state_view.hpp"
#include "type_traits.hpp"

#include "../buffer/builder_base.hpp"
#include "../formalism/problem/declarations.hpp"


namespace mimir {

/**
 * Data types
*/
enum SearchNodeStatus {NEW = 0, OPEN = 1, CLOSED = 2, DEAD_END = 3};

using g_value_type = int;


/**
 * Interface class
*/
template<typename Derived>
requires HasPlanningMode<Derived>
class SearchNodeBuilderBase {
private:
    using P = typename TypeTraits<Derived>::PlanningMode;

    SearchNodeBuilderBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void set_status(SearchNodeStatus status) { self().set_status_impl(status); }
    void set_g_value(int g_value) { self().set_g_value_impl(g_value); }
    void set_parent_state(View<State<P>> parent_state) { self().set_parent_state_impl(parent_state); }
    void set_ground_action(GroundAction creating_action) { self().set_ground_action_impl(creating_action); }
};


/**
 * Implementation class
 *
 * The search node builder extends the builder base memory layout as follows:
 *  ____________________________________________________________________
 * |                |        |         |              |                 |
 * | data_size_type | status | g_value | parent_state | creating_action |
 * |________________|________|_________|______________|_________________|
*/
template<IsPlanningMode P>
class Builder<SearchNode<P>> : public BuilderBase<Builder<SearchNode<P>>>, public SearchNodeBuilderBase<Builder<SearchNode<P>>> {
private:
    SearchNodeStatus m_status;
    int m_g_value;
    View<State<P>> m_parent_state;
    GroundAction m_creating_action;

    /* Implement BuilderBase interface */
    data_size_type calculate_size_impl() const {
        return sizeof(SearchNodeStatus) + sizeof(int) + sizeof(char*) + sizeof(GroundAction);
    }

    void finish_impl() {
        this->m_buffer.write(m_status);
        this->m_buffer.write(m_g_value);
        this->m_buffer.write(m_parent_state);
        this->m_buffer.write(m_creating_action);
    }

    friend class BuilderBase<Builder<SearchNode<P>>>;

    /* Implement SearchNodeBuilderBase interface */
    void set_status_impl(SearchNodeStatus status) { m_status = status; }
    void set_g_value_impl(int g_value) { m_g_value = g_value; }
    void set_parent_state_impl(View<State<P>> parent_state) { m_parent_state = parent_state; }
    void set_ground_action_impl(GroundAction creating_action) { m_creating_action = creating_action; }

    friend class SearchNodeBuilderBase<Builder<SearchNode<P>>>;

public:
    Builder() : m_parent_state(View<State<P>>(nullptr)) { }

    /// @brief Construct a builder with custom default values.
    Builder(SearchNodeStatus status, int g_value, View<State<P>> parent_state, GroundAction creating_action)
        : m_status(status), m_g_value(g_value), m_parent_state(parent_state), m_creating_action(creating_action) {
        this->finish();
    }
};


/**
 * Type traits.
*/
template<IsPlanningMode P>
struct TypeTraits<Builder<SearchNode<P>>> {
    using PlanningMode = P;
};



} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_
