#ifndef MIMIR_SEARCH_SEARCH_NODES_COST_HPP_
#define MIMIR_SEARCH_SEARCH_NODES_COST_HPP_

#include "template.hpp"

#include "../states.hpp"
#include "../actions.hpp"


namespace mimir
{

/**
 * Derived ID class.
 * 
 * Define name and template parameters of your own implementation.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
class CostSearchNodeTag : public SearchNodeBaseTag {};


/**
 * Aliases
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
using CostSearchNodeBuilder = Builder<CostSearchNodeTag<P, S, A>>;

template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
using CostSearchNodeView = View<CostSearchNodeTag<P, S, A>>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct TypeTraits<CostSearchNodeBuilder<P, S, A>> {
    using PlanningModeTag = P;
    using StateTag = S;
    using ActionTag = A;
};

template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct TypeTraits<CostSearchNodeView<P, S, A>> {
    using PlanningModeTag = P;
    using StateTag = S;
    using ActionTag = A;
};


/**
 * Interface class
*/
template<typename Derived>
class CostSearchNodeBuilderBase {
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;
    using A = typename TypeTraits<Derived>::ActionTag;
    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<A, P, S>>;

    CostSearchNodeBuilderBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void set_status(SearchNodeStatus status) { self().set_status_impl(status); }
    void set_g_value(int g_value) { self().set_g_value_impl(g_value); }
    void set_parent_state(StateView parent_state) { self().set_parent_state_impl(parent_state); }
    void set_ground_action(ActionView creating_action) { self().set_ground_action_impl(creating_action); }
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
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
class Builder<CostSearchNodeTag<P, S, A>>
    : public BuilderBase<Builder<CostSearchNodeTag<P, S, A>>>
    , public CostSearchNodeBuilderBase<Builder<CostSearchNodeTag<P, S, A>>> {
private:
    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<A, P, S>>;

    SearchNodeStatus m_status;
    int m_g_value;
    StateView m_parent_state;
    ActionView m_creating_action;

    /* Implement BuilderBase interface */
    data_size_type calculate_size_impl() const {
        return sizeof(SearchNodeStatus) + sizeof(int) + sizeof(StateView) + sizeof(ActionView);
    }

    void finish_impl() {
        this->m_buffer.write(m_status);
        this->m_buffer.write(m_g_value);
        this->m_buffer.write(m_parent_state);
        this->m_buffer.write(m_creating_action);
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class BuilderBase;

    /* Implement CostSearchNodeBuilderBase interface */
    void set_status_impl(SearchNodeStatus status) { m_status = status; }
    void set_g_value_impl(int g_value) { m_g_value = g_value; }
    void set_parent_state_impl(StateView parent_state) { m_parent_state = parent_state; }
    void set_ground_action_impl(ActionView creating_action) { m_creating_action = creating_action; }

    // Give access to the private interface implementations.
    template<typename>
    friend class CostSearchNodeBuilderBase;

public:
    Builder() : m_parent_state(StateView(nullptr)), m_creating_action(ActionView(nullptr)) { }

    /// @brief Construct a builder with custom default values.
    Builder(SearchNodeStatus status, int g_value, StateView parent_state, ActionView creating_action)
        : m_status(status), m_g_value(g_value), m_parent_state(parent_state), m_creating_action(creating_action) {
        this->finish();
    }
};


/**
 * Interface class
*/
template<typename Derived>
class CostSearchNodeViewBase {
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;
    using A = typename TypeTraits<Derived>::ActionTag;
    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<A, P, S>>;

    CostSearchNodeViewBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] SearchNodeStatus& get_status() { return self().get_status_impl(); }
    [[nodiscard]] g_value_type& get_g_value() { return self().get_g_value_impl(); }
    [[nodiscard]] StateView get_parent_state() { return self().get_parent_state_impl(); }
    [[nodiscard]] ActionView get_ground_action() { return self().get_ground_action_impl(); }
};


/**
 * Implementation class
 *
 * Reads the memory layout generated by the search node builder.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
class View<CostSearchNodeTag<P, S, A>>
    : public ViewBase<View<CostSearchNodeTag<P, S, A>>>
    , public CostSearchNodeViewBase<View<CostSearchNodeTag<P, S, A>>> {
private:
    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<A, P, S>>;

    static constexpr size_t s_status_offset =       sizeof(data_size_type);
    static constexpr size_t s_g_value_offset =      sizeof(data_size_type) + sizeof(SearchNodeStatus);
    static constexpr size_t s_parent_state_offset = sizeof(data_size_type) + sizeof(SearchNodeStatus) + sizeof(g_value_type);
    static constexpr size_t s_ground_action =       sizeof(data_size_type) + sizeof(SearchNodeStatus) + sizeof(g_value_type) + sizeof(StateView);
    /* Implement ViewBase interface: */
    [[nodiscard]] size_t get_offset_to_representative_data_impl() const { return 0; }

    /* Implement SearchNodeViewBase interface */
    [[nodiscard]] SearchNodeStatus& get_status_impl() {
        return read_value<SearchNodeStatus>(this->get_data() + s_status_offset);
    }

    [[nodiscard]] g_value_type& get_g_value_impl() {
        return read_value<g_value_type>(this->get_data() + s_g_value_offset);
    }

    [[nodiscard]] StateView get_parent_state_impl() {
        return read_value<StateView>(this->get_data() + s_parent_state_offset);
    }

    [[nodiscard]] ActionView get_ground_action_impl() {
        return read_value<ActionView>(this->get_data() + s_ground_action);
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class CostSearchNodeViewBase;

public:
    /// @brief Create a view on a SearchNode.
    explicit View(char* data) : ViewBase<View<CostSearchNodeTag<P, S, A>>>(data) { }
};





}  // namespace mimir

#endif  // MIMIR_SEARCH_SEARCH_NODES_COST_HPP_
