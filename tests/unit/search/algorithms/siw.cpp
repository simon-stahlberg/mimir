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

#include "mimir/search/algorithms/siw.hpp"

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/state_repository.hpp"

#include <gtest/gtest.h>

using namespace mimir::search;
using namespace mimir::formalism;

namespace mimir::tests
{

/// @brief Instantiate a lifted SIW search
class LiftedSIWPlanner
{
private:
    Problem m_problem;
    size_t m_arity;
    LiftedApplicableActionGeneratorImpl::EventHandler m_applicable_action_generator_event_handler;
    LiftedApplicableActionGenerator m_applicable_action_generator;
    LiftedAxiomEvaluatorImpl::EventHandler m_axiom_evaluator_event_handler;
    LiftedAxiomEvaluator m_axiom_evaluator;
    StateRepository m_state_repository;
    brfs::EventHandler m_brfs_event_handler;
    iw::EventHandler m_iw_event_handler;
    siw::EventHandler m_siw_event_handler;
    SearchContext m_search_context;

public:
    LiftedSIWPlanner(const fs::path& domain_file, const fs::path& problem_file, size_t arity) :
        m_problem(ProblemImpl::create(domain_file, problem_file)),
        m_arity(arity),
        m_applicable_action_generator_event_handler(LiftedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create()),
        m_applicable_action_generator(LiftedApplicableActionGeneratorImpl::create(m_problem, m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(LiftedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create()),
        m_axiom_evaluator(LiftedAxiomEvaluatorImpl::create(m_problem, m_axiom_evaluator_event_handler)),
        m_state_repository(StateRepositoryImpl::create(m_axiom_evaluator)),
        m_brfs_event_handler(brfs::DefaultEventHandlerImpl::create(m_problem)),
        m_iw_event_handler(iw::DefaultEventHandlerImpl::create(m_problem)),
        m_siw_event_handler(siw::DefaultEventHandlerImpl::create(m_problem)),
        m_search_context(SearchContextImpl::create(m_problem, m_applicable_action_generator, m_state_repository))
    {
    }

    SearchResult find_solution()
    {
        return siw::find_solution(m_search_context, nullptr, m_arity, m_siw_event_handler, m_iw_event_handler, m_brfs_event_handler);
    }

    const siw::Statistics& get_iw_statistics() const { return m_siw_event_handler->get_statistics(); }

    const LiftedApplicableActionGeneratorImpl::Statistics& get_applicable_action_generator_statistics() const
    {
        return m_applicable_action_generator_event_handler->get_statistics();
    }

    const LiftedAxiomEvaluatorImpl::Statistics& get_axiom_evaluator_statistics() const { return m_axiom_evaluator_event_handler->get_statistics(); }
};

/// @brief Instantiate a grounded SIW search
class GroundedSIWPlanner
{
private:
    Problem m_problem;
    size_t m_arity;
    DeleteRelaxedProblemExplorator m_delete_relaxed_problem_explorator;
    GroundedApplicableActionGeneratorImpl::EventHandler m_applicable_action_generator_event_handler;
    GroundedApplicableActionGenerator m_applicable_action_generator;
    GroundedAxiomEvaluatorImpl::EventHandler m_axiom_evaluator_event_handler;
    GroundedAxiomEvaluator m_axiom_evaluator;
    StateRepository m_state_repository;
    brfs::EventHandler m_brfs_event_handler;
    iw::EventHandler m_iw_event_handler;
    siw::EventHandler m_siw_event_handler;
    SearchContext m_search_context;

public:
    GroundedSIWPlanner(const fs::path& domain_file, const fs::path& problem_file, size_t arity) :
        m_problem(ProblemImpl::create(domain_file, problem_file)),
        m_arity(arity),
        m_delete_relaxed_problem_explorator(m_problem),
        m_applicable_action_generator_event_handler(GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create()),
        m_applicable_action_generator(
            m_delete_relaxed_problem_explorator.create_grounded_applicable_action_generator(match_tree::Options(),
                                                                                            m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(GroundedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create()),
        m_axiom_evaluator(m_delete_relaxed_problem_explorator.create_grounded_axiom_evaluator(match_tree::Options(), m_axiom_evaluator_event_handler)),
        m_state_repository(StateRepositoryImpl::create(m_axiom_evaluator)),
        m_brfs_event_handler(brfs::DefaultEventHandlerImpl::create(m_problem)),
        m_iw_event_handler(iw::DefaultEventHandlerImpl::create(m_problem)),
        m_siw_event_handler(siw::DefaultEventHandlerImpl::create(m_problem)),
        m_search_context(SearchContextImpl::create(m_problem, m_applicable_action_generator, m_state_repository))
    {
    }

    SearchResult find_solution()
    {
        return siw::find_solution(m_search_context, nullptr, m_arity, m_siw_event_handler, m_iw_event_handler, m_brfs_event_handler);
    }

    const siw::Statistics& get_iw_statistics() const { return m_siw_event_handler->get_statistics(); }

    const GroundedApplicableActionGeneratorImpl::Statistics& get_applicable_action_generator_statistics() const
    {
        return m_applicable_action_generator_event_handler->get_statistics();
    }

    const GroundedAxiomEvaluatorImpl::Statistics& get_axiom_evaluator_statistics() const { return m_axiom_evaluator_event_handler->get_statistics(); }
};

/**
 * Gripper
 */

TEST(MimirTests, SearchAlgorithmsSIWGroundedGripperTest)
{
    auto siw = GroundedSIWPlanner(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem2.pddl"), 3);
    const auto result = siw.find_solution();
    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 7);

    const auto& siw_statistics = siw.get_iw_statistics();

    EXPECT_EQ(siw_statistics.get_iw_statistics_by_subproblem().size(), 2);
    EXPECT_EQ(siw_statistics.get_num_generated_until_last_g_layer(), 94);
    EXPECT_EQ(siw_statistics.get_num_expanded_until_last_g_layer(), 25);
    EXPECT_EQ(siw_statistics.get_maximum_effective_width(), 2);
    EXPECT_EQ(siw_statistics.get_average_effective_width(), 2);
}

TEST(MimirTests, SearchAlgorithmsSIWLiftedGripperTest)
{
    auto siw = LiftedSIWPlanner(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem2.pddl"), 3);
    const auto result = siw.find_solution();
    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 7);

    const auto& siw_statistics = siw.get_iw_statistics();

    EXPECT_EQ(siw_statistics.get_iw_statistics_by_subproblem().size(), 2);
    EXPECT_EQ(siw_statistics.get_num_generated_until_last_g_layer(), 94);
    EXPECT_EQ(siw_statistics.get_num_expanded_until_last_g_layer(), 25);
    EXPECT_EQ(siw_statistics.get_maximum_effective_width(), 2);
    EXPECT_EQ(siw_statistics.get_average_effective_width(), 2);
}

}
