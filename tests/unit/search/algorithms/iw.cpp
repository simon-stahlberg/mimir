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

#include "mimir/search/algorithms/iw.hpp"

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/algorithms/iw/event_handlers.hpp"
#include "mimir/search/algorithms/iw/tuple_index_generators.hpp"
#include "mimir/search/algorithms/iw/tuple_index_mapper.hpp"
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

/// @brief Instantiate a lifted IW search
class LiftedIWPlanner
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
    SearchContext m_search_context;

public:
    LiftedIWPlanner(const fs::path& domain_file, const fs::path& problem_file, size_t arity) :
        m_problem(ProblemImpl::create(domain_file, problem_file)),
        m_arity(arity),
        m_applicable_action_generator_event_handler(LiftedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create()),
        m_applicable_action_generator(LiftedApplicableActionGeneratorImpl::create(m_problem, m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(LiftedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create()),
        m_axiom_evaluator(LiftedAxiomEvaluatorImpl::create(m_problem, m_axiom_evaluator_event_handler)),
        m_state_repository(StateRepositoryImpl::create(m_axiom_evaluator)),
        m_brfs_event_handler(brfs::DefaultEventHandlerImpl::create(m_problem)),
        m_iw_event_handler(iw::DefaultEventHandlerImpl::create(m_problem)),
        m_search_context(SearchContextImpl::create(m_problem, m_applicable_action_generator, m_state_repository))
    {
    }

    SearchResult find_solution() { return iw::find_solution(m_search_context, nullptr, m_arity, m_iw_event_handler, m_brfs_event_handler); }

    const iw::Statistics& get_iw_statistics() const { return m_iw_event_handler->get_statistics(); }

    const LiftedApplicableActionGeneratorImpl::Statistics& get_applicable_action_generator_statistics() const
    {
        return m_applicable_action_generator_event_handler->get_statistics();
    }

    const LiftedAxiomEvaluatorImpl::Statistics& get_axiom_evaluator_statistics() const { return m_axiom_evaluator_event_handler->get_statistics(); }
};

/// @brief Instantiate a grounded IW search
class GroundedIWPlanner
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
    SearchContext m_search_context;

public:
    GroundedIWPlanner(const fs::path& domain_file, const fs::path& problem_file, size_t arity) :
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
        m_search_context(SearchContextImpl::create(m_problem, m_applicable_action_generator, m_state_repository))
    {
    }

    SearchResult find_solution() { return iw::find_solution(m_search_context, nullptr, m_arity, m_iw_event_handler, m_brfs_event_handler); }

    const iw::Statistics& get_iw_statistics() const { return m_iw_event_handler->get_statistics(); }

    const GroundedApplicableActionGeneratorImpl::Statistics& get_applicable_action_generator_statistics() const
    {
        return m_applicable_action_generator_event_handler->get_statistics();
    }

    const GroundedAxiomEvaluatorImpl::Statistics& get_axiom_evaluator_statistics() const { return m_axiom_evaluator_event_handler->get_statistics(); }
};

TEST(MimirTests, SearchAlgorithmsIWSingleStateTupleIndexGeneratorWidth0Test)
{
    const int arity = 0;
    const int num_atoms = 3;

    const auto tuple_index_mapper = iw::TupleIndexMapper(arity, num_atoms);
    auto generator = iw::StateTupleIndexGenerator(&tuple_index_mapper);
    const auto atom_indices = iw::AtomIndexList({
        num_atoms,  // placeholder to generate tuples of size less than arity
    });

    auto iter = generator.begin(atom_indices);

    EXPECT_EQ("()", tuple_index_mapper.tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWSingleStateTupleIndexGeneratorWidth1Test)
{
    const int arity = 1;
    const int num_atoms = 3;

    const auto tuple_index_mapper = iw::TupleIndexMapper(arity, num_atoms);
    auto generator = iw::StateTupleIndexGenerator(&tuple_index_mapper);
    const auto atom_indices = iw::AtomIndexList({
        0,
        2,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });

    auto iter = generator.begin(atom_indices);

    EXPECT_EQ("(0,)", tuple_index_mapper.tuple_index_to_string(*iter));
    EXPECT_EQ("(2,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("()", tuple_index_mapper.tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWSingleStateTupleIndexGeneratorWidth2Test)
{
    const int arity = 2;
    const int num_atoms = 3;

    const auto tuple_index_mapper = iw::TupleIndexMapper(arity, num_atoms);
    auto generator = iw::StateTupleIndexGenerator(&tuple_index_mapper);
    const auto atom_indices = iw::AtomIndexList({
        0,
        2,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });

    auto iter = generator.begin(atom_indices);

    EXPECT_EQ("(0,2,)", tuple_index_mapper.tuple_index_to_string(*iter));
    EXPECT_EQ("(0,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("()", tuple_index_mapper.tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWStatePairTupleIndexGeneratorWidth1Test)
{
    const int arity = 1;
    const int num_atoms = 4;

    const auto tuple_index_mapper = iw::TupleIndexMapper(arity, num_atoms);
    auto generator = iw::StatePairTupleIndexGenerator(&tuple_index_mapper);
    const auto atom_indices = iw::AtomIndexList({
        0,
        2,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });
    const auto add_atom_indices = iw::AtomIndexList({
        1,
        3,
    });

    auto iter = generator.begin(atom_indices, add_atom_indices);

    EXPECT_EQ("(1,)", tuple_index_mapper.tuple_index_to_string(*iter));
    EXPECT_EQ("(3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWStatePairTupleIndexGeneratorWidth2Test1)
{
    const int arity = 2;
    const int num_atoms = 4;

    const auto tuple_index_mapper = iw::TupleIndexMapper(arity, num_atoms);
    auto generator = iw::StatePairTupleIndexGenerator(&tuple_index_mapper);
    const auto atom_indices = iw::AtomIndexList({
        0,
        2,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });
    const auto add_atom_indices = iw::AtomIndexList({
        1,
        3,
    });

    auto iter = generator.begin(atom_indices, add_atom_indices);

    EXPECT_EQ("(1,2,)", tuple_index_mapper.tuple_index_to_string(*iter));
    EXPECT_EQ("(1,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,1,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(1,3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWStatePairTupleIndexGeneratorWidth2Test2)
{
    const int arity = 3;
    const int num_atoms = 64;

    const auto tuple_index_mapper = iw::TupleIndexMapper(arity, num_atoms);
    auto generator = iw::StatePairTupleIndexGenerator(&tuple_index_mapper);
    const auto atom_indices = iw::AtomIndexList({
        0,
        2,
        3,
        4,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });
    const auto add_atom_indices = iw::AtomIndexList({
        6,
    });

    auto iter = generator.begin(atom_indices, add_atom_indices);

    EXPECT_EQ("(6,)", tuple_index_mapper.tuple_index_to_string(*iter));
    EXPECT_EQ("(0,6,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,6,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(3,6,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(4,6,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,2,6,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,3,6,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,4,6,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,3,6,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,4,6,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(3,4,6,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWStatePairTupleIndexGeneratorWidth2Test3)
{
    const int arity = 2;
    const int num_atoms = 64;

    const auto tuple_index_mapper = iw::TupleIndexMapper(arity, num_atoms);
    auto generator = iw::StatePairTupleIndexGenerator(&tuple_index_mapper);
    const auto atom_indices = iw::AtomIndexList({
        0,
        1,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });
    const auto add_atom_indices = iw::AtomIndexList({
        2,
        3,
    });

    auto iter = generator.begin(atom_indices, add_atom_indices);

    EXPECT_EQ("(2,)", tuple_index_mapper.tuple_index_to_string(*iter));
    EXPECT_EQ("(3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,2,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(1,2,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(1,3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

/**
 * Delivery
 */

TEST(MimirTests, SearchAlgorithmsIWGroundedDeliveryTest)
{
    auto iw = GroundedIWPlanner(fs::path(std::string(DATA_DIR) + "delivery/domain.pddl"), fs::path(std::string(DATA_DIR) + "delivery/test_problem.pddl"), 3);
    const auto result = iw.find_solution();
    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 4);

    const auto& iw_statistics = iw.get_iw_statistics();

    EXPECT_EQ(iw_statistics.get_brfs_statistics_by_arity().back().get_num_generated_until_g_value().back(), 18);
    EXPECT_EQ(iw_statistics.get_brfs_statistics_by_arity().back().get_num_expanded_until_g_value().back(), 7);
    EXPECT_EQ(iw_statistics.get_effective_width(), 2);
}

TEST(MimirTests, SearchAlgorithmsIWLiftedDeliveryTest)
{
    auto iw = LiftedIWPlanner(fs::path(std::string(DATA_DIR) + "delivery/domain.pddl"), fs::path(std::string(DATA_DIR) + "delivery/test_problem.pddl"), 3);
    const auto result = iw.find_solution();
    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 4);

    const auto& applicable_action_generator_statistics = iw.get_applicable_action_generator_statistics();
    const auto& axiom_evaluator_statistics = iw.get_axiom_evaluator_statistics();

    EXPECT_EQ(applicable_action_generator_statistics.get_num_ground_action_cache_hits_per_search_layer().back(), 25);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_ground_action_cache_misses_per_search_layer().back(), 12);

    EXPECT_EQ(axiom_evaluator_statistics.get_num_ground_axiom_cache_hits_per_search_layer().back(), 0);
    EXPECT_EQ(axiom_evaluator_statistics.get_num_ground_axiom_cache_misses_per_search_layer().back(), 0);

    const auto& iw_statistics = iw.get_iw_statistics();

    EXPECT_EQ(iw_statistics.get_brfs_statistics_by_arity().back().get_num_generated_until_g_value().back(), 18);
    EXPECT_EQ(iw_statistics.get_brfs_statistics_by_arity().back().get_num_expanded_until_g_value().back(), 7);
    EXPECT_EQ(iw_statistics.get_effective_width(), 2);
}

/**
 * Miconic-fulladl
 */

TEST(MimirTests, SearchAlgorithmsIWGroundedMiconicFullAdlTest)
{
    auto iw = GroundedIWPlanner(fs::path(std::string(DATA_DIR) + "miconic-fulladl/domain.pddl"),
                                fs::path(std::string(DATA_DIR) + "miconic-fulladl/test_problem.pddl"),
                                3);
    const auto result = iw.find_solution();
    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 7);

    const auto& iw_statistics = iw.get_iw_statistics();

    EXPECT_EQ(iw_statistics.get_brfs_statistics_by_arity().back().get_num_generated_until_g_value().back(), 69);
    EXPECT_EQ(iw_statistics.get_brfs_statistics_by_arity().back().get_num_expanded_until_g_value().back(), 27);
    EXPECT_EQ(iw_statistics.get_effective_width(), 2);
}

TEST(MimirTests, SearchAlgorithmsIWLiftedMiconicFullAdlTest)
{
    auto iw = LiftedIWPlanner(fs::path(std::string(DATA_DIR) + "miconic-fulladl/domain.pddl"),
                              fs::path(std::string(DATA_DIR) + "miconic-fulladl/test_problem.pddl"),
                              3);
    const auto result = iw.find_solution();
    EXPECT_EQ(result.status, SearchStatus::SOLVED);
    EXPECT_EQ(result.plan.value().get_actions().size(), 7);

    const auto& applicable_action_generator_statistics = iw.get_applicable_action_generator_statistics();
    const auto& axiom_evaluator_statistics = iw.get_axiom_evaluator_statistics();

    EXPECT_EQ(applicable_action_generator_statistics.get_num_ground_action_cache_hits_per_search_layer().back(), 89);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_ground_action_cache_misses_per_search_layer().back(), 10);

    EXPECT_EQ(axiom_evaluator_statistics.get_num_ground_axiom_cache_hits_per_search_layer().back(), 345);
    EXPECT_EQ(axiom_evaluator_statistics.get_num_ground_axiom_cache_misses_per_search_layer().back(), 16);

    const auto& iw_statistics = iw.get_iw_statistics();

    EXPECT_EQ(iw_statistics.get_brfs_statistics_by_arity().back().get_num_generated_until_g_value().back(), 69);
    EXPECT_EQ(iw_statistics.get_brfs_statistics_by_arity().back().get_num_expanded_until_g_value().back(), 27);
    EXPECT_EQ(iw_statistics.get_effective_width(), 2);
}

}
