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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_LIFTED_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_LIFTED_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/axiom_evaluators/lifted/event_handlers/statistics.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir
{
template<typename T>
class MatchTree;

/**
 * Interface class
 */
class ILiftedAxiomEvaluatorEventHandler
{
public:
    virtual ~ILiftedAxiomEvaluatorEventHandler() = default;

    virtual void on_start_generating_applicable_axioms() = 0;

    virtual void on_ground_axiom(GroundAxiom axiom) = 0;

    virtual void on_ground_axiom_cache_hit(GroundAxiom axiom) = 0;

    virtual void on_ground_axiom_cache_miss(GroundAxiom axiom) = 0;

    virtual void on_end_generating_applicable_axioms() = 0;

    virtual void on_end_search() = 0;

    virtual void on_finish_search_layer() = 0;

    virtual const LiftedAxiomEvaluatorStatistics& get_statistics() const = 0;
};

/**
 * Base class
 *
 * Collect statistics and call implementation of derived class.
 */
template<typename Derived_>
class LiftedAxiomEvaluatorEventHandlerBase : public ILiftedAxiomEvaluatorEventHandler
{
protected:
    LiftedAxiomEvaluatorStatistics m_statistics;
    bool m_quiet;

private:
    LiftedAxiomEvaluatorEventHandlerBase() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    explicit LiftedAxiomEvaluatorEventHandlerBase(bool quiet = true) : m_statistics(), m_quiet(quiet) {}

    void on_start_generating_applicable_axioms() override
    {
        if (!m_quiet)
        {
            self().on_start_generating_applicable_axioms_impl();
        }
    }

    void on_ground_axiom(GroundAxiom axiom) override
    {
        if (!m_quiet)
        {
            self().on_ground_axiom_impl(axiom);
        }
    }

    void on_ground_axiom_cache_hit(GroundAxiom axiom) override
    {
        m_statistics.increment_num_ground_axiom_cache_hits();

        if (!m_quiet)
        {
            self().on_ground_axiom_cache_hit_impl(axiom);
        }
    }

    void on_ground_axiom_cache_miss(GroundAxiom axiom) override
    {
        m_statistics.increment_num_ground_axiom_cache_misses();

        if (!m_quiet)
        {
            self().on_ground_axiom_cache_miss_impl(axiom);
        }
    }

    void on_end_generating_applicable_axioms() override
    {
        if (!m_quiet)
        {
            self().on_end_generating_applicable_axioms_impl();
        }
    }

    void on_finish_search_layer() override
    {
        m_statistics.on_finish_search_layer();

        if (!m_quiet)
        {
            self().on_finish_search_layer_impl();
        }
    }

    void on_end_search() override
    {
        if (!m_quiet)
        {
            self().on_end_search_impl();
        }
    }

    const LiftedAxiomEvaluatorStatistics& get_statistics() const override { return m_statistics; }
};
}

#endif