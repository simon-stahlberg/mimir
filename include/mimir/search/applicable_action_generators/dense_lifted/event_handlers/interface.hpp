#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/event_handlers/statistics.hpp"
#include "mimir/search/axioms.hpp"
#include "mimir/search/states.hpp"

#include <chrono>

namespace mimir
{
template<typename T>
class MatchTree;

/**
 * Interface class
 */
class ILiftedAAGEventHandler
{
public:
    virtual ~ILiftedAAGEventHandler() = default;

    virtual void on_start_generating_applicable_actions() = 0;

    virtual void on_ground_action(const Action action, const ObjectList& binding) = 0;

    virtual void on_ground_action_cache_hit(const Action action, const ObjectList& binding) = 0;

    virtual void on_ground_action_cache_miss(const Action action, const ObjectList& binding) = 0;

    virtual void on_ground_inapplicable_action(const GroundAction action, const PDDLFactories& pddl_factories) = 0;

    virtual void on_end_generating_applicable_actions(const GroundActionList& ground_actions, const PDDLFactories& pddl_factories) = 0;

    virtual void on_end_search() = 0;

    virtual const LiftedAAGStatistics& get_statistics() const = 0;
};

/**
 * Base class
 *
 * Collect statistics and call implementation of derived class.
 */
template<typename Derived>
class LiftedAAGEventHandlerBase : public ILiftedAAGEventHandler
{
protected:
    LiftedAAGStatistics m_statistics;

private:
    LiftedAAGEventHandlerBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void on_start_generating_applicable_actions() override
    {  //
        self().on_start_generating_applicable_actions_impl();
    }

    void on_ground_action(const Action action, const ObjectList& binding) override
    {  //
        self().on_ground_action_impl(action, binding);
    }

    void on_ground_action_cache_hit(const Action action, const ObjectList& binding) override
    {  //
        m_statistics.increment_num_cache_hits();

        self().on_ground_action_cache_hit_impl(action, binding);
    }

    void on_ground_action_cache_miss(const Action action, const ObjectList& binding) override
    {  //
        m_statistics.increment_num_cache_misses();

        self().on_ground_action_cache_miss_impl(action, binding);
    }

    void on_ground_inapplicable_action(const GroundAction action, const PDDLFactories& pddl_factories) override
    {  //
        m_statistics.increment_num_inapplicable_grounded_actions();

        self().on_ground_inapplicable_action_impl(action, pddl_factories);
    }

    void on_end_generating_applicable_actions(const GroundActionList& ground_actions, const PDDLFactories& pddl_factories) override
    {  //
        self().on_end_generating_applicable_actions_impl(ground_actions, pddl_factories);
    }

    void on_end_search() override
    {  //
        self().on_end_search_impl();
    }

    const LiftedAAGStatistics& get_statistics() const override { return m_statistics; }
};
}

#endif