#ifndef MIMIR_SEARCH_ALGORITHMS_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_ALGORITHMS_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/factories.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/algorithms/event_handlers/statistics.hpp"
#include "mimir/search/states.hpp"

#include <chrono>
#include <concepts>

namespace mimir
{

/**
 * Interface class
 */
class IAlgorithmEventHandler
{
public:
    virtual ~IAlgorithmEventHandler() = default;

    /// @brief React on generating a successor_state by applying an action.
    virtual void on_generate_state(GroundAction action, State successor_state, const PDDLFactories& pddl_factories) = 0;

    /// @brief React on finishing expanding a g-layer.
    virtual void on_finish_g_layer(uint64_t g_value, uint64_t num_states) = 0;

    /// @brief React on expanding a state.
    virtual void on_expand_state(State state, const PDDLFactories& pddl_factories) = 0;

    /// @brief React on starting a search.
    virtual void on_start_search(State initial_state, const PDDLFactories& pddl_factories) = 0;

    /// @brief React on ending a search.
    virtual void on_end_search() = 0;

    /// @brief React on solving a search.
    virtual void on_solved(const GroundActionList& ground_action_plan) = 0;

    /// @brief React on exhausting a search.
    virtual void on_exhausted() = 0;

    virtual const Statistics& get_statistics() const = 0;
};

/**
 * Base class
 *
 * Collect statistics and call implementation of derived class.
 */
template<typename Derived>
class AlgorithmEventHandlerBase : public IAlgorithmEventHandler
{
protected:
    Statistics m_statistics;

private:
    AlgorithmEventHandlerBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void on_generate_state(GroundAction action, State successor_state, const PDDLFactories& pddl_factories) override
    {
        m_statistics.increment_num_generated();

        self().on_generate_state_impl(action, successor_state, pddl_factories);
    }

    void on_finish_g_layer(uint64_t g_value, uint64_t num_states) override
    {
        m_statistics.set_g_value(g_value);
        m_statistics.set_num_states_until_g_value(num_states);

        self().on_finish_g_layer_impl(g_value, num_states);
    }

    void on_expand_state(State state, const PDDLFactories& pddl_factories) override
    {
        m_statistics.increment_num_expanded();

        self().on_expand_state_impl(state, pddl_factories);
    }

    void on_start_search(State initial_state, const PDDLFactories& pddl_factories) override
    {
        m_statistics.set_search_start_time_point(std::chrono::high_resolution_clock::now());

        self().on_start_search_impl(initial_state, pddl_factories);
    }

    void on_end_search() override
    {
        m_statistics.set_search_end_time_point(std::chrono::high_resolution_clock::now());

        self().on_end_search_impl();
    }

    void on_solved(const GroundActionList& ground_action_plan) override { self().on_solved_impl(ground_action_plan); }

    void on_exhausted() override { self().on_exhausted_impl(); }

    /// @brief Get the statistics.
    const Statistics& get_statistics() const override { return m_statistics; }
};

}

#endif