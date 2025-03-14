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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_GROUNDED_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_GROUNDED_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/axiom_evaluators/grounded/event_handlers/statistics.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/match_tree/match_tree.hpp"

namespace mimir::search
{

/**
 * Interface class
 */
class IGroundedAxiomEvaluatorEventHandler
{
public:
    virtual ~IGroundedAxiomEvaluatorEventHandler() = default;

    virtual void on_start_ground_axiom_instantiation() = 0;

    virtual void on_finish_ground_axiom_instantiation(std::chrono::milliseconds total_time) = 0;

    virtual void on_start_build_axiom_match_trees() = 0;

    virtual void on_start_build_axiom_match_tree(size_t partition_index) = 0;

    virtual void on_finish_build_axiom_match_tree(const match_tree::MatchTree<formalism::GroundAxiomImpl>& match_tree) = 0;

    virtual void on_finish_build_axiom_match_trees(std::chrono::milliseconds total_time) = 0;

    virtual void on_finish_search_layer() = 0;

    virtual void on_end_search() = 0;

    virtual const GroundedAxiomEvaluatorStatistics& get_statistics() const = 0;
};

/**
 * Base class
 *
 * Collect statistics and call implementation of derived class.
 */
template<typename Derived_>
class GroundedAxiomEvaluatorEventHandlerBase : public IGroundedAxiomEvaluatorEventHandler
{
protected:
    GroundedAxiomEvaluatorStatistics m_statistics;
    bool m_quiet;

private:
    GroundedAxiomEvaluatorEventHandlerBase() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    explicit GroundedAxiomEvaluatorEventHandlerBase(bool quiet = true) : m_statistics(), m_quiet(quiet) {}

    void on_start_ground_axiom_instantiation() override
    {
        if (!m_quiet)
        {
            self().on_start_ground_axiom_instantiation_impl();
        }
    }

    void on_finish_ground_axiom_instantiation(std::chrono::milliseconds total_time) override
    {
        if (!m_quiet)
        {
            self().on_finish_ground_axiom_instantiation_impl(total_time);
        }
    }

    void on_start_build_axiom_match_trees() override
    {
        if (!m_quiet)
        {
            self().on_start_build_axiom_match_trees_impl();
        }
    }

    void on_start_build_axiom_match_tree(size_t partition_index) override
    {
        if (!m_quiet)
        {
            self().on_start_build_axiom_match_tree_impl(partition_index);
        }
    }

    void on_finish_build_axiom_match_tree(const match_tree::MatchTree<formalism::GroundAxiomImpl>& match_tree) override
    {  //
        m_statistics.m_match_tree_partition_statistics.push_back(match_tree.get_statistics());

        if (!m_quiet)
        {
            self().on_finish_build_axiom_match_tree_impl(match_tree);
        }
    }

    void on_finish_build_axiom_match_trees(std::chrono::milliseconds total_time) override
    {
        if (!m_quiet)
        {
            self().on_finish_build_axiom_match_trees_impl(total_time);
        }
    }

    void on_finish_search_layer() override
    {  //
        if (!m_quiet)
        {
            self().on_finish_search_layer_impl();
        }
    }

    void on_end_search() override
    {  //
        if (!m_quiet)
        {
            self().on_end_search_impl();
        }
    }

    const GroundedAxiomEvaluatorStatistics& get_statistics() const override { return m_statistics; }
};
}

#endif
