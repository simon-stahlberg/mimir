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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/repositories.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/algorithms/brfs/event_handlers/statistics.hpp"
#include "mimir/search/algorithms/iw/event_handlers/statistics.hpp"

#include <chrono>
#include <concepts>

namespace mimir
{

/**
 * Interface class
 */
class IIWAlgorithmEventHandler
{
public:
    virtual ~IIWAlgorithmEventHandler() = default;

    /// @brief React on starting a search.
    virtual void on_start_search(const Problem problem, const State initial_state, const PDDLRepositories& pddl_repositories) = 0;

    /// @brief React on starting a search.
    virtual void on_start_arity_search(const Problem problem, const State initial_state, const PDDLRepositories& pddl_repositories, int arity) = 0;

    /// @brief React on starting a search.
    virtual void on_end_arity_search(const BrFSAlgorithmStatistics& brfs_statistics) = 0;

    /// @brief React on ending a search.
    virtual void on_end_search() = 0;

    /// @brief React on solving a search.
    virtual void on_solved(const Plan& plan, const PDDLRepositories& pddl_repositories) = 0;

    /// @brief React on proving unsolvability during a search.
    virtual void on_unsolvable() = 0;

    /// @brief React on exhausting a search.
    virtual void on_exhausted() = 0;

    virtual const IWAlgorithmStatistics& get_statistics() const = 0;
    virtual bool is_quiet() const = 0;
};

/**
 * Base class
 *
 * Collect statistics and call implementation of derived class.
 */
template<typename Derived_>
class IWAlgorithmEventHandlerBase : public IIWAlgorithmEventHandler
{
protected:
    IWAlgorithmStatistics m_statistics;
    bool m_quiet;

private:
    IWAlgorithmEventHandlerBase() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    explicit IWAlgorithmEventHandlerBase(bool quiet = true) : m_statistics(), m_quiet(quiet) {}

    void on_start_search(const Problem problem, const State initial_state, const PDDLRepositories& pddl_repositories) override
    {
        m_statistics = IWAlgorithmStatistics();

        m_statistics.set_search_start_time_point(std::chrono::high_resolution_clock::now());

        if (!m_quiet)
        {
            self().on_start_search_impl(problem, initial_state, pddl_repositories);
        }
    }

    void on_start_arity_search(const Problem problem, const State initial_state, const PDDLRepositories& pddl_repositories, int arity) override
    {
        if (!m_quiet)
        {
            self().on_start_arity_search_impl(problem, initial_state, pddl_repositories, arity);
        }
    }

    void on_end_arity_search(const BrFSAlgorithmStatistics& brfs_statistics) override
    {
        m_statistics.push_back_algorithm_statistics(brfs_statistics);

        if (!m_quiet)
        {
            self().on_end_arity_search_impl(brfs_statistics);
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

    void on_solved(const Plan& plan, const PDDLRepositories& pddl_repositories) override
    {
        if (!m_quiet)
        {
            self().on_solved_impl(plan, pddl_repositories);
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
    const IWAlgorithmStatistics& get_statistics() const override { return m_statistics; }
    bool is_quiet() const override { return m_quiet; }
};

}

#endif