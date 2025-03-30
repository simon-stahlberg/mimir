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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDED_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/dense_state.hpp"
#include "mimir/search/match_tree/match_tree.hpp"

#include <variant>

namespace mimir::search
{

/// @brief `GroundedApplicableActionGenerator` implements grounded applicable action generation
/// using the `LiftedApplicableActionGenerator` and `AxiomEvaluator` to create an overapproximation
/// of applicable ground actions and ground actions and storing them in a match tree
/// as described by Helmert
class GroundedApplicableActionGenerator : public IApplicableActionGenerator
{
public:
    struct Statistics
    {
        match_tree::Statistics statistics;
    };

    class IEventHandler
    {
    public:
        virtual ~IEventHandler() = default;

        virtual void on_start_ground_action_instantiation() = 0;

        virtual void on_finish_ground_action_instantiation(std::chrono::milliseconds total_time) = 0;

        virtual void on_start_build_action_match_tree() = 0;

        virtual void on_finish_build_action_match_tree(const match_tree::MatchTree<formalism::GroundActionImpl>& match_tree) = 0;

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

        void on_start_ground_action_instantiation() override
        {
            if (!m_quiet)
                self().on_start_ground_action_instantiation_impl();
        }

        void on_finish_ground_action_instantiation(std::chrono::milliseconds total_time) override
        {
            if (!m_quiet)
                self().on_finish_ground_action_instantiation_impl(total_time);
        }

        void on_start_build_action_match_tree() override
        {
            if (!m_quiet)
                self().on_start_build_action_match_tree_impl();
        }

        void on_finish_build_action_match_tree(const match_tree::MatchTree<formalism::GroundActionImpl>& match_tree) override
        {
            m_statistics.statistics = match_tree.get_statistics();

            if (!m_quiet)
                self().on_finish_build_action_match_tree_impl(match_tree);
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

        void on_start_ground_action_instantiation_impl() const;

        void on_finish_ground_action_instantiation_impl(std::chrono::milliseconds total_time) const;

        void on_start_build_action_match_tree_impl() const;

        void on_finish_build_action_match_tree_impl(const match_tree::MatchTree<formalism::GroundActionImpl>& action_match_tree);

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

        void on_start_ground_action_instantiation_impl() const;

        void on_finish_ground_action_instantiation_impl(std::chrono::milliseconds total_time) const;

        void on_start_build_action_match_tree_impl() const;

        void on_finish_build_action_match_tree_impl(const match_tree::MatchTree<formalism::GroundActionImpl>& action_match_tree);

        void on_finish_search_layer_impl() const;

        void on_end_search_impl() const;

    public:
        explicit DefaultEventHandler(bool quiet = true) : EventHandlerBase<DefaultEventHandler>(quiet) {}
    };

    /// @brief Simplest construction
    GroundedApplicableActionGenerator(formalism::Problem problem, std::unique_ptr<match_tree::MatchTree<formalism::GroundActionImpl>>&& match_tree);

    /// @brief Complete construction
    GroundedApplicableActionGenerator(formalism::Problem problem,
                                      std::unique_ptr<match_tree::MatchTree<formalism::GroundActionImpl>>&& match_tree,
                                      std::shared_ptr<IEventHandler> event_handler);

    // Uncopyable
    GroundedApplicableActionGenerator(const GroundedApplicableActionGenerator& other) = delete;
    GroundedApplicableActionGenerator& operator=(const GroundedApplicableActionGenerator& other) = delete;
    // Unmovable
    GroundedApplicableActionGenerator(GroundedApplicableActionGenerator&& other) = delete;
    GroundedApplicableActionGenerator& operator=(GroundedApplicableActionGenerator&& other) = delete;

    /// @brief Create a grounded applicable action generator for the given state.
    /// @param state is the state.
    /// @param workspace is the workspace. There is no specific workspace in the grounded case and it should not be initialized.
    /// @return a generator to yield the applicable actions for the given state.
    mimir::generator<formalism::GroundAction> create_applicable_action_generator(State state) override;
    mimir::generator<formalism::GroundAction> create_applicable_action_generator(const DenseState& dense_state) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    /**
     * Getters
     */

    const formalism::Problem& get_problem() const override;

private:
    formalism::Problem m_problem;
    std::unique_ptr<match_tree::MatchTree<formalism::GroundActionImpl>> m_match_tree;

    std::shared_ptr<IEventHandler> m_event_handler;

    /* Memory for reuse */
    DenseState m_dense_state;
};

}

#endif