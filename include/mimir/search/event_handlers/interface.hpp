#ifndef MIMIR_SEARCH_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/factories.hpp"
#include "mimir/search/compile_flags.hpp"
#include "mimir/search/statistics.hpp"

#include <chrono>
#include <concepts>

namespace mimir
{

/**
 * Interface class
 */
class IEventHandler
{
public:
    virtual ~IEventHandler() = default;

    /// @brief React on generating a successor_state by applying an action.
    virtual void on_generate_state(ConstView<ActionDispatcher<StateReprTag>> action,
                                   ConstView<StateDispatcher<StateReprTag>> successor_state,
                                   const PDDLFactories& pddl_factories) = 0;

    /// @brief React on expanding a state.
    virtual void on_expand_state(ConstView<StateDispatcher<StateReprTag>> state, const PDDLFactories& pddl_factories) = 0;

    /// @brief React on starting a search.
    virtual void on_start_search(ConstView<StateDispatcher<StateReprTag>> initial_state, const PDDLFactories& pddl_factories) = 0;

    /// @brief React on ending a search.
    virtual void on_end_search() = 0;

    /// @brief React on solving a search.
    virtual void on_solved(const std::vector<ConstView<ActionDispatcher<StateReprTag>>>& ground_action_plan) = 0;

    /// @brief React on exhausting a search.
    virtual void on_exhausted() = 0;
};

/**
 * Base class
 */
template<typename Derived>
class EventHandlerBase : public IEventHandler
{
protected:
    Statistics m_statistics;

private:
    EventHandlerBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void on_generate_state(ConstView<ActionDispatcher<StateReprTag>> action,
                           ConstView<StateDispatcher<StateReprTag>> successor_state,
                           const PDDLFactories& pddl_factories) override
    {
        m_statistics.increment_num_generated();

        self().on_generate_state_impl(action, successor_state, pddl_factories);
    }

    void on_expand_state(ConstView<StateDispatcher<StateReprTag>> state, const PDDLFactories& pddl_factories) override
    {
        m_statistics.increment_num_expanded();

        self().on_expand_state_impl(state, pddl_factories);
    }

    void on_start_search(ConstView<StateDispatcher<StateReprTag>> initial_state, const PDDLFactories& pddl_factories) override
    {
        m_statistics.set_search_start_time_point(std::chrono::high_resolution_clock::now());

        self().on_start_search_impl(initial_state, pddl_factories);
    }

    void on_end_search() override
    {
        m_statistics.set_search_end_time_point(std::chrono::high_resolution_clock::now());

        self().on_end_search_impl();
    }

    void on_solved(const std::vector<ConstView<ActionDispatcher<StateReprTag>>>& ground_action_plan) override { self().on_solved_impl(ground_action_plan); }

    void on_exhausted() override { self().on_exhausted_impl(); }

    /// @brief Get the statistics.
    const Statistics& get_statistics() const { return m_statistics; }
};

}

#endif