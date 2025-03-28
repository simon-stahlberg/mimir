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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATOR_GROUNDED_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATOR_GROUNDED_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/match_tree/match_tree.hpp"

namespace mimir::search
{

class GroundedAxiomEvaluator : public IAxiomEvaluator
{
public:
    struct Statistics
    {
        std::vector<match_tree::Statistics> m_match_tree_partition_statistics = std::vector<match_tree::Statistics>();
    };

    class IEventHandler
    {
    public:
        virtual ~IEventHandler() = default;

        virtual void on_start_ground_axiom_instantiation() = 0;

        virtual void on_finish_ground_axiom_instantiation(std::chrono::milliseconds total_time) = 0;

        virtual void on_start_build_axiom_match_trees() = 0;

        virtual void on_start_build_axiom_match_tree(size_t partition_index) = 0;

        virtual void on_finish_build_axiom_match_tree(const match_tree::MatchTree<formalism::GroundAxiomImpl>& match_tree) = 0;

        virtual void on_finish_build_axiom_match_trees(std::chrono::milliseconds total_time) = 0;

        virtual void on_finish_search_layer() = 0;

        virtual void on_end_search() = 0;

        virtual const Statistics& get_statistics() const = 0;
    };

    /**
     * Base class
     *
     * Collect statistics and call implementation of derived class.
     */
    template<typename Derived_>
    class EventHandlerBase : public IEventHandler
    {
    protected:
        Statistics m_statistics;
        bool m_quiet;

    private:
        EventHandlerBase() = default;
        friend Derived_;

        /// @brief Helper to cast to Derived.
        constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
        constexpr auto& self() { return static_cast<Derived_&>(*this); }

    public:
        explicit EventHandlerBase(bool quiet = true) : m_statistics(), m_quiet(quiet) {}

        void on_start_ground_axiom_instantiation() override
        {
            if (!m_quiet)
                self().on_start_ground_axiom_instantiation_impl();
        }

        void on_finish_ground_axiom_instantiation(std::chrono::milliseconds total_time) override
        {
            if (!m_quiet)
                self().on_finish_ground_axiom_instantiation_impl(total_time);
        }

        void on_start_build_axiom_match_trees() override
        {
            if (!m_quiet)
                self().on_start_build_axiom_match_trees_impl();
        }

        void on_start_build_axiom_match_tree(size_t partition_index) override
        {
            if (!m_quiet)
                self().on_start_build_axiom_match_tree_impl(partition_index);
        }

        void on_finish_build_axiom_match_tree(const match_tree::MatchTree<formalism::GroundAxiomImpl>& match_tree) override
        {
            m_statistics.m_match_tree_partition_statistics.push_back(match_tree.get_statistics());

            if (!m_quiet)
                self().on_finish_build_axiom_match_tree_impl(match_tree);
        }

        void on_finish_build_axiom_match_trees(std::chrono::milliseconds total_time) override
        {
            if (!m_quiet)
                self().on_finish_build_axiom_match_trees_impl(total_time);
        }

        void on_finish_search_layer() override
        {
            if (!m_quiet)
                self().on_finish_search_layer_impl();
        }

        void on_end_search() override
        {
            if (!m_quiet)
                self().on_end_search_impl();
        }

        const Statistics& get_statistics() const override { return m_statistics; }
    };

    class DebugEventHandler : public EventHandlerBase<DebugEventHandler>
    {
    private:
        /* Implement EventHandlerBase interface */
        friend class EventHandlerBase<DebugEventHandler>;

        void on_start_ground_axiom_instantiation_impl() const;

        void on_finish_ground_axiom_instantiation_impl(std::chrono::milliseconds total_time) const;

        void on_start_build_axiom_match_trees_impl() const;

        void on_start_build_axiom_match_tree_impl(size_t partition_index) const;

        void on_finish_build_axiom_match_tree_impl(const match_tree::MatchTree<formalism::GroundAxiomImpl>& match_tree) const;

        void on_finish_build_axiom_match_trees_impl(std::chrono::milliseconds total_time) const;

        void on_finish_search_layer_impl() const;

        void on_end_search_impl() const;

    public:
        explicit DebugEventHandler(bool quiet = true) : EventHandlerBase<DebugEventHandler>(quiet) {}
    };

    class DefaultEventHandler : public EventHandlerBase<DefaultEventHandler>
    {
    private:
        /* Implement EventHandlerBase interface */
        friend class EventHandlerBase<DefaultEventHandler>;

        void on_start_ground_axiom_instantiation_impl() const;

        void on_finish_ground_axiom_instantiation_impl(std::chrono::milliseconds total_time) const;

        void on_start_build_axiom_match_trees_impl() const;

        void on_start_build_axiom_match_tree_impl(size_t partition_index) const;

        void on_finish_build_axiom_match_tree_impl(const match_tree::MatchTree<formalism::GroundAxiomImpl>& match_tree) const;

        void on_finish_build_axiom_match_trees_impl(std::chrono::milliseconds total_time) const;

        void on_finish_search_layer_impl() const;

        void on_end_search_impl() const;

    public:
        explicit DefaultEventHandler(bool quiet = true) : EventHandlerBase<DefaultEventHandler>(quiet) {}
    };

public:
    GroundedAxiomEvaluator(formalism::Problem problem,
                           std::vector<std::unique_ptr<match_tree::MatchTree<formalism::GroundAxiomImpl>>>&& match_tree_partitioning);

    GroundedAxiomEvaluator(formalism::Problem problem,
                           std::vector<std::unique_ptr<match_tree::MatchTree<formalism::GroundAxiomImpl>>>&& match_tree_partitioning,
                           std::shared_ptr<IEventHandler> event_handler);

    // Uncopyable
    GroundedAxiomEvaluator(const GroundedAxiomEvaluator& other) = delete;
    GroundedAxiomEvaluator& operator=(const GroundedAxiomEvaluator& other) = delete;
    // Unmovable
    GroundedAxiomEvaluator(GroundedAxiomEvaluator&& other) = delete;
    GroundedAxiomEvaluator& operator=(GroundedAxiomEvaluator&& other) = delete;

    void generate_and_apply_axioms(DenseState& dense_state) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    /**
     * Getters
     */

    const formalism::Problem& get_problem() const override;
    const std::shared_ptr<IEventHandler>& get_event_handler() const;

private:
    formalism::Problem m_problem;
    std::vector<std::unique_ptr<match_tree::MatchTree<formalism::GroundAxiomImpl>>> m_match_tree_partitioning;
    std::shared_ptr<IEventHandler> m_event_handler;
};

}

#endif