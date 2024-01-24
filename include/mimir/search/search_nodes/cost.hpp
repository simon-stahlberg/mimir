#ifndef MIMIR_SEARCH_SEARCH_NODES_COST_HPP_
#define MIMIR_SEARCH_SEARCH_NODES_COST_HPP_

#include "template.hpp"

#include "../states.hpp"


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation.
*/
template<IsPlanningModeTag P, IsStateTag S, IsGroundActionTag G>
class CostSearchNodeTag {};


/**
 * Aliases
*/
template<IsPlanningModeTag P, IsStateTag S, IsGroundActionTag G>
using CostStateBuilder = View<CostSearchNodeTag<P, S, G>>;

template<IsPlanningModeTag P, IsStateTag S, IsGroundActionTag G>
using CostStateView = View<CostSearchNodeTag<P, S, G>>;


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S, IsGroundActionTag G>
struct TypeTraits<CostStateBuilder<P, S, G>> {
    using PlanningModeTag = P;
    using StateTag = S;
    using GroundActionTag = G;
};

template<IsPlanningModeTag P, IsStateTag S, IsGroundActionTag G>
struct TypeTraits<CostStateView<P, S, G>> {
    using PlanningModeTag = P;
    using StateTag = S;
    using GroundActionTag = G;
};



/**
 * Interface class
*/
template<typename Derived>
class SearchNodeViewBase {
private:
    using P = typename TypeTraits<Derived>::PlanningMode;

    SearchNodeViewBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /* Mutable getters. */
    [[nodiscard]] SearchNodeStatus& get_status() { return self().get_status_impl(); }
    [[nodiscard]] g_value_type& get_g_value() { return self().get_g_value_impl(); }
    [[nodiscard]] View<State<P>> get_parent_state() { return self().get_parent_state_impl(); }
    [[nodiscard]] GroundAction get_ground_action() { return self().get_ground_action_impl(); }

    /* Immutable getters. */
    [[nodiscard]] const SearchNodeStatus& get_status() const { return self().get_status_impl(); }
    [[nodiscard]] const g_value_type& get_g_value() const { return self().get_g_value_impl(); }
    [[nodiscard]] const View<State<P>> get_parent_state() const { return self().get_parent_state_impl(); }
    [[nodiscard]] const GroundAction get_ground_action() const { return self().get_ground_action_impl(); }
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
template<IsPlanningModeTag P, IsStateTag S, IsGroundActionTag G>
class Builder<CostSearchNodeTag<P, S, G>>
    : public BuilderBase<Builder<CostSearchNodeTag<P, S, G>>>
    , public SearchNodeBuilderBase<Builder<CostSearchNodeTag<P, S, G>>> {
private:
    using StateView = View<WrappedStateTag<P>>;
    using GroundActionView = View<GroundActionTag<P>>;

    SearchNodeStatus m_status;
    int m_g_value;
    StateView m_parent_state;
    GroundActionView m_creating_action;

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

    // Give access to the private interface implementations.
    template<typename>
    friend class BuilderBase;

    /* Implement SearchNodeBuilderBase interface */
    void set_status_impl(SearchNodeStatus status) { m_status = status; }
    void set_g_value_impl(int g_value) { m_g_value = g_value; }
    void set_parent_state_impl(View<State<P>> parent_state) { m_parent_state = parent_state; }
    void set_ground_action_impl(GroundAction creating_action) { m_creating_action = creating_action; }

    // Give access to the private interface implementations.
    template<typename>
    friend class SearchNodeBuilderBase;

public:
    Builder() : m_parent_state(StateView(nullptr)), m_creating_action(GroundActionView(nullptr)) { }

    /// @brief Construct a builder with custom default values.
    Builder(SearchNodeStatus status, int g_value, StateView parent_state, GroundActionView creating_action)
        : m_status(status), m_g_value(g_value), m_parent_state(parent_state), m_creating_action(creating_action) {
        this->finish();
    }
};


/**
 * Interface class
*/
template<typename Derived>
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
 * Reads the memory layout generated by the search node builder.
*/
template<IsPlanningModeTag P, IsStateTag S, IsGroundActionTag G>
class View<SearchNode<P>> : public ViewBase<View<SearchNode<P>>>, public SearchNodeViewBase<View<SearchNode<P>>> {
private:
    static constexpr size_t s_status_offset =       sizeof(data_size_type);
    static constexpr size_t s_g_value_offset =      sizeof(data_size_type) + sizeof(SearchNodeStatus);
    static constexpr size_t s_parent_state_offset = sizeof(data_size_type) + sizeof(SearchNodeStatus) + sizeof(g_value_type);
    static constexpr size_t s_ground_action =       sizeof(data_size_type) + sizeof(SearchNodeStatus) + sizeof(g_value_type) + sizeof(View<State<P>>);

    /* Implement ViewBase interface: */
    [[nodiscard]] size_t get_offset_to_representative_data_impl() const { return 0; }

    /* Implement SearchNodeViewBase interface */
    [[nodiscard]] SearchNodeStatus& get_status_impl() {
        return read_value<SearchNodeStatus>(this->get_data() + s_status_offset);
    }

    [[nodiscard]] g_value_type& get_g_value_impl() {
        return read_value<g_value_type>(this->get_data() + s_g_value_offset);
    }

    [[nodiscard]] View<State<P>> get_parent_state_impl() {
        return read_value<View<State<P>>>(this->get_data() + s_parent_state_offset);
    }

    [[nodiscard]] GroundAction get_ground_action_impl() {
        return read_pointer<const GroundActionImpl>(this->get_data() + s_ground_action);
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class SearchNodeViewBase;

public:
    /// @brief Create a view on a SearchNode.
    explicit View(char* data) : ViewBase<View<SearchNode<P>>>(data) { }
};





}  // namespace mimir

#endif  // MIMIR_SEARCH_SEARCH_NODES_COST_HPP_
