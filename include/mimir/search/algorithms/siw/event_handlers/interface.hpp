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

#ifndef MIMIR_SEARCH_ALGORITHMS_SIW_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_ALGORITHMS_SIW_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/factories.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/algorithms/brfs/event_handlers/statistics.hpp"
#include "mimir/search/algorithms/iw/event_handlers/statistics.hpp"
#include "mimir/search/algorithms/siw/event_handlers/statistics.hpp"

#include <chrono>
#include <concepts>

namespace mimir
{

/**
 * Interface class
 */
class ISIWAlgorithmEventHandler
{
public:
    virtual ~ISIWAlgorithmEventHandler() = default;

    /// @brief React on starting a search.
    virtual void on_start_search(const Problem problem, const State initial_state, const PDDLFactories& pddl_factories) = 0;

    /// @brief React on starting a search.
    virtual void on_start_subproblem_search(const Problem problem, const State initial_state, const PDDLFactories& pddl_factories) = 0;

    /// @brief React on starting a search.
    virtual void on_end_subproblem_search(const IWAlgorithmStatistics& iw_statistics) = 0;

    /// @brief React on ending a search.
    virtual void on_end_search() = 0;

    /// @brief React on solving a search.
    virtual void on_solved(const GroundActionList& ground_action_plan) = 0;

    /// @brief React on proving unsolvability during a search.
    virtual void on_unsolvable() = 0;

    /// @brief React on exhausting a search.
    virtual void on_exhausted() = 0;

    virtual const SIWAlgorithmStatistics& get_statistics() const = 0;
    virtual bool is_quiet() const = 0;
};

/**
 * Base class
 *
 * Collect statistics and call implementation of derived class.
 */
template<typename Derived>
class SIWAlgorithmEventHandlerBase : public ISIWAlgorithmEventHandler
{
protected:
    SIWAlgorithmStatistics m_statistics;
    bool m_quiet;

private:
    SIWAlgorithmEventHandlerBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    explicit SIWAlgorithmEventHandlerBase(bool quiet = true) : m_statistics(), m_quiet(quiet) {}

    void on_start_search(const Problem problem, const State initial_state, const PDDLFactories& pddl_factories) override
    {
        m_statistics = SIWAlgorithmStatistics();

        m_statistics.set_search_start_time_point(std::chrono::high_resolution_clock::now());

        if (!m_quiet)
        {
            self().on_start_search_impl(problem, initial_state, pddl_factories);
        }
    }

    void on_start_subproblem_search(const Problem problem, const State initial_state, const PDDLFactories& pddl_factories) override
    {
        if (!m_quiet)
        {
            self().on_start_subproblem_search_impl(problem, initial_state, pddl_factories);
        }
    }

    void on_end_subproblem_search(const IWAlgorithmStatistics& iw_statistics) override
    {
        m_statistics.push_back_algorithm_statistics(iw_statistics);

        if (!m_quiet)
        {
            self().on_end_subproblem_search_impl(iw_statistics);
        }
    }

    void on_end_search() override
    {
        m_statistics.set_search_end_time_point(std::chrono::high_resolution_clock::now());

        if (!m_quiet)
        {
            self().on_end_search_impl();
        }
    }

    void on_solved(const GroundActionList& ground_action_plan) override
    {
        if (!m_quiet)
        {
            self().on_solved_impl(ground_action_plan);
        }
    }

    void on_unsolvable() override
    {
        if (!m_quiet)
        {
            self().on_unsolvable_impl();
        }
    }

    void on_exhausted() override
    {
        if (!m_quiet)
        {
            self().on_exhausted_impl();
        }
    }

    /// @brief Get the statistics.
    const SIWAlgorithmStatistics& get_statistics() const override { return m_statistics; }
    bool is_quiet() const override { return m_quiet; }
};

}

#endif