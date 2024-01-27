#ifndef MIMIR_SEARCH_SEARCH_NODES_COST_HPP_
#define MIMIR_SEARCH_SEARCH_NODES_COST_HPP_

#include "template.hpp"

#include "../../buffer/flatbuffers/search/search_nodes/cost_generated.h"

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

    flatbuffers::FlatBufferBuilder m_flatbuffers_builder;
    CostSearchNodeFlatBuilder m_search_node_builder;

    SearchNodeStatus m_status;
    int m_g_value;
    StateView m_parent_state;
    ActionView m_creating_action;

    /* Implement BuilderBase interface */
    template<typename>
    friend class BuilderBase;

    void finish_impl() {
        m_flatbuffers_builder.FinishSizePrefixed(m_search_node_builder.Finish());

        // Get a pointer to the serialized data
        uint8_t* buf = m_flatbuffers_builder.GetBufferPointer();

        // Get the size of the serialized data, including the size prefix
        auto size = m_flatbuffers_builder.GetSize();

        // Reinterpret the buffer, it can possible be moved to a different place before
        auto reinterpreted = GetMutableSizePrefixedCostSearchNodeFlat(buf);

        // TODO: how to get the size of the buffer?
        auto sizePrefix = *reinterpret_cast<const flatbuffers::uoffset_t*>(buf);
    }

    uint8_t* get_buffer_pointer_impl() {
        return m_flatbuffers_builder.GetBufferPointer();
    }

    const uint8_t* get_buffer_pointer_impl() const {
        return m_flatbuffers_builder.GetBufferPointer();
    }

    void clear_impl() {
        m_flatbuffers_builder.Clear();
    }


    /* Implement CostSearchNodeBuilderBase interface */
    template<typename>
    friend class CostSearchNodeBuilderBase;

    void set_status_impl(SearchNodeStatus status) {
        m_status = status;
        m_search_node_builder.add_status(static_cast<SearchNodeStatusFlat>(status));
    }
    void set_g_value_impl(int g_value) {
        m_g_value = g_value;
        m_search_node_builder.add_g_value(g_value);
    }
    void set_parent_state_impl(StateView parent_state) {
        m_parent_state = parent_state;
        m_search_node_builder.add_state(pointer_to_uint64_t(parent_state.get_buffer_pointer()));
    }
    void set_ground_action_impl(ActionView creating_action) {
        m_creating_action = creating_action;
        m_search_node_builder.add_state(pointer_to_uint64_t(creating_action.get_buffer_pointer()));
    }

public:
    Builder()
        : m_flatbuffers_builder(1024)
        , m_search_node_builder(m_flatbuffers_builder)
        , m_parent_state(StateView(nullptr))
        , m_creating_action(ActionView(nullptr)) { }

    /// @brief Construct a builder with custom default values.
    Builder(SearchNodeStatus status, int g_value, StateView parent_state, ActionView creating_action)
        : m_flatbuffers_builder(1024)
        , m_search_node_builder(m_flatbuffers_builder)
        , m_status(status), m_g_value(g_value), m_parent_state(parent_state), m_creating_action(creating_action) {
        this->set_status(status);
        this->set_g_value(g_value);
        this->set_parent_state(parent_state);
        this->set_ground_action(creating_action);
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
        m_flatbuffers_view->mutate_status(static_cast<SearchNodeStatusFlat>(status));
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
        , m_flatbuffers_view(GetMutableSizePrefixedCostSearchNodeFlat(reinterpret_cast<void*>(data))) { }
};





}  // namespace mimir

#endif  // MIMIR_SEARCH_SEARCH_NODES_COST_HPP_
