#ifndef MIMIR_SEARCH_SEARCH_NODES_COST_HPP_
#define MIMIR_SEARCH_SEARCH_NODES_COST_HPP_

#include "template.hpp"

#include "../../buffer/flatbuffers/search/search_nodes/cost_generated.h"

#include "../states.hpp"
#include "../actions.hpp"

#include <iostream>
#include <iomanip>


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

    flatbuffers::FlatBufferBuilder m_flatbuffers_builder;

    SearchNodeStatus m_status;
    int m_g_value;
    StateView m_parent_state;
    ActionView m_creating_action;

    /* Implement BuilderBase interface */
    template<typename>
    friend class BuilderBase;

    void finish_impl() {
        auto builder = CostSearchNodeFlatBuilder(this->m_flatbuffers_builder);
        builder.add_status(m_status);
        builder.add_g_value(m_g_value);
        builder.add_state(pointer_to_uint64_t(m_parent_state.get_buffer_pointer()));
        builder.add_action(pointer_to_uint64_t(m_creating_action.get_buffer_pointer()));
        this->m_flatbuffers_builder.FinishSizePrefixed(builder.Finish());
    }

    void clear_impl() {
        m_flatbuffers_builder.Clear();
    }

    [[nodiscard]] uint8_t* get_buffer_pointer_impl() { return m_flatbuffers_builder.GetBufferPointer(); }
    [[nodiscard]] const uint8_t* get_buffer_pointer_impl() const { return m_flatbuffers_builder.GetBufferPointer(); }
    [[nodiscard]] uint32_t get_size_impl() const { return *reinterpret_cast<const flatbuffers::uoffset_t*>(this->get_buffer_pointer()) + sizeof(flatbuffers::uoffset_t); }

    /* Implement CostSearchNodeBuilderBase interface */
    template<typename>
    friend class CostSearchNodeBuilderBase;

    void set_status_impl(SearchNodeStatus status) { m_status = status; }
    void set_g_value_impl(int g_value) { m_g_value = g_value; }
    void set_parent_state_impl(StateView parent_state) { m_parent_state = parent_state; }
    void set_ground_action_impl(ActionView creating_action) { m_creating_action = creating_action; }

public:
    // TODO: get some better default constructed state and action
    Builder() : m_parent_state(nullptr), m_creating_action(nullptr) { }

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
    void set_status(SearchNodeStatus status) { self().set_status_impl(status); }
    void set_g_value(int g_value) { self().set_g_value_impl(g_value); }

    [[nodiscard]] SearchNodeStatus get_status() { return self().get_status_impl(); }
    [[nodiscard]] g_value_type get_g_value() { return self().get_g_value_impl(); }
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

    CostSearchNodeFlat* m_flatbuffers_view;

    /* Implement ViewBase interface: */
    template<typename>
    friend class ViewBase;

    /* Implement CostSearchNodeViewBase interface */
    template<typename>
    friend class CostSearchNodeViewBase;

    void set_status_impl(SearchNodeStatus status) {
        m_flatbuffers_view->mutate_status(static_cast<uint8_t>(status));
    }
    void set_g_value_impl(int g_value) {
        m_flatbuffers_view->mutate_g_value(g_value);
    }

    [[nodiscard]] SearchNodeStatus get_status_impl() {
        return static_cast<SearchNodeStatus>(m_flatbuffers_view->status());
    }

    [[nodiscard]] g_value_type get_g_value_impl() {
        return m_flatbuffers_view->g_value();
    }

    [[nodiscard]] StateView get_parent_state_impl() {
        return StateView(uint64_t_to_pointer<uint8_t>(m_flatbuffers_view->state()));
    }

    [[nodiscard]] ActionView get_ground_action_impl() {
        return ActionView(uint64_t_to_pointer<uint8_t>(m_flatbuffers_view->action()));
    }

public:
    /// @brief Create a view on a SearchNode.
    explicit View(uint8_t* data)
        : ViewBase<View<CostSearchNodeTag<P, S, A>>>(data)
        , m_flatbuffers_view(data ? GetMutableSizePrefixedCostSearchNodeFlat(reinterpret_cast<void*>(data)) : nullptr) { }
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_SEARCH_NODES_COST_HPP_
