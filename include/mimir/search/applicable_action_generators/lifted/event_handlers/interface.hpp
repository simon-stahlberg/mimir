/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/formalism.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers/statistics.hpp"
#include "mimir/search/axiom.hpp"
#include "mimir/search/condition_grounders/event_handlers/interface.hpp"
#include "mimir/search/state.hpp"

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

    virtual void on_ground_action(Action action, const ObjectList& binding) = 0;

    virtual void on_ground_action_cache_hit(Action action, const ObjectList& binding) = 0;

    virtual void on_ground_action_cache_miss(Action action, const ObjectList& binding) = 0;

    virtual void on_end_generating_applicable_actions(const GroundActionList& ground_actions, const PDDLFactories& pddl_factories) = 0;

    virtual void on_start_generating_applicable_axioms() = 0;

    virtual void on_ground_axiom(Axiom axiom, const ObjectList& binding) = 0;

    virtual void on_ground_axiom_cache_hit(Axiom axiom, const ObjectList& binding) = 0;

    virtual void on_ground_axiom_cache_miss(Axiom axiom, const ObjectList& binding) = 0;

    virtual void on_end_generating_applicable_axioms(const GroundAxiomList& ground_axioms, const PDDLFactories& pddl_factories) = 0;

    virtual void on_end_search() = 0;

    virtual void on_finish_f_layer() = 0;

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
    bool m_quiet;

private:
    LiftedAAGEventHandlerBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    explicit LiftedAAGEventHandlerBase(bool quiet = true) : m_statistics(), m_quiet(quiet) {}

    void on_start_generating_applicable_actions() override
    {
        if (!m_quiet)
        {
            self().on_start_generating_applicable_actions_impl();
        }
    }

    void on_ground_action(Action action, const ObjectList& binding) override
    {
        if (!m_quiet)
        {
            self().on_ground_action_impl(action, binding);
        }
    }

    void on_ground_action_cache_hit(Action action, const ObjectList& binding) override
    {
        m_statistics.increment_num_ground_action_cache_hits();

        if (!m_quiet)
        {
            self().on_ground_action_cache_hit_impl(action, binding);
        }
    }

    void on_ground_action_cache_miss(Action action, const ObjectList& binding) override
    {
        m_statistics.increment_num_ground_action_cache_misses();

        if (!m_quiet)
        {
            self().on_ground_action_cache_miss_impl(action, binding);
        }
    }

    void on_end_generating_applicable_actions(const GroundActionList& ground_actions, const PDDLFactories& pddl_factories) override
    {
        if (!m_quiet)
        {
            self().on_end_generating_applicable_actions_impl(ground_actions, pddl_factories);
        }
    }

    void on_start_generating_applicable_axioms() override
    {
        if (!m_quiet)
        {
            self().on_start_generating_applicable_axioms_impl();
        }
    }

    void on_ground_axiom(Axiom axiom, const ObjectList& binding) override
    {
        if (!m_quiet)
        {
            self().on_ground_axiom_impl(axiom, binding);
        }
    }

    void on_ground_axiom_cache_hit(Axiom axiom, const ObjectList& binding) override
    {
        m_statistics.increment_num_ground_axiom_cache_hits();

        if (!m_quiet)
        {
            self().on_ground_axiom_cache_hit_impl(axiom, binding);
        }
    }

    void on_ground_axiom_cache_miss(Axiom axiom, const ObjectList& binding) override
    {
        m_statistics.increment_num_ground_axiom_cache_misses();

        if (!m_quiet)
        {
            self().on_ground_axiom_cache_miss_impl(axiom, binding);
        }
    }

    void on_end_generating_applicable_axioms(const GroundAxiomList& ground_axioms, const PDDLFactories& pddl_factories) override
    {
        if (!m_quiet)
        {
            self().on_end_generating_applicable_axioms_impl(ground_axioms, pddl_factories);
        }
    }

    void on_finish_f_layer() override
    {
        m_statistics.on_finish_f_layer();

        if (!m_quiet)
        {
            self().on_finish_f_layer_impl();
        }
    }

    void on_end_search() override
    {
        if (!m_quiet)
        {
            self().on_end_search_impl();
        }
    }

    const LiftedAAGStatistics& get_statistics() const override { return m_statistics; }
};
}

#endif
