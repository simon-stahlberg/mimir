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
#include "mimir/formalism/parser.hpp"
#include "mimir/search/algorithms/brfs/event_handlers.hpp"
#include "mimir/search/algorithms/iw/event_handlers.hpp"
#include "mimir/search/algorithms/iw/tuple_index_generators.hpp"
#include "mimir/search/algorithms/iw/tuple_index_mapper.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/axiom_evaluators/grounded/event_handlers.hpp"
#include "mimir/search/axiom_evaluators/lifted/event_handlers.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/state_repository.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

/// @brief Instantiate a lifted IW search
class LiftedIWPlanner
{
private:
    PDDLParser m_parser;

    std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> m_applicable_action_generator_event_handler;
    std::shared_ptr<LiftedApplicableActionGenerator> m_applicable_action_generator;
    std::shared_ptr<ILiftedAxiomEvaluatorEventHandler> m_axiom_evaluator_event_handler;
    std::shared_ptr<LiftedAxiomEvaluator> m_axiom_evaluator;
    std::shared_ptr<StateRepository> m_state_repository;
    std::shared_ptr<IBrFSAlgorithmEventHandler> m_brfs_event_handler;
    std::shared_ptr<IIWAlgorithmEventHandler> m_iw_event_handler;
    std::unique_ptr<IAlgorithm> m_algorithm;

public:
    LiftedIWPlanner(const fs::path& domain_file, const fs::path& problem_file, int arity) :
        m_parser(PDDLParser(domain_file, problem_file)),
        m_applicable_action_generator_event_handler(std::make_shared<DefaultLiftedApplicableActionGeneratorEventHandler>()),
        m_applicable_action_generator(std::make_shared<LiftedApplicableActionGenerator>(m_parser.get_problem(),
                                                                                        m_parser.get_pddl_repositories(),
                                                                                        m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(std::make_shared<DefaultLiftedAxiomEvaluatorEventHandler>()),
        m_axiom_evaluator(std::make_shared<LiftedAxiomEvaluator>(m_parser.get_problem(), m_parser.get_pddl_repositories(), m_axiom_evaluator_event_handler)),
        m_state_repository(std::make_shared<StateRepository>(m_axiom_evaluator)),
        m_brfs_event_handler(std::make_shared<DefaultBrFSAlgorithmEventHandler>()),
        m_iw_event_handler(std::make_shared<DefaultIWAlgorithmEventHandler>()),
        m_algorithm(std::make_unique<IWAlgorithm>(m_applicable_action_generator, m_state_repository, arity, m_brfs_event_handler, m_iw_event_handler))
    {
    }

    SearchResult find_solution() { return m_algorithm->find_solution(); }

    const IWAlgorithmStatistics& get_iw_statistics() const { return m_iw_event_handler->get_statistics(); }

    const LiftedApplicableActionGeneratorStatistics& get_applicable_action_generator_statistics() const
    {
        return m_applicable_action_generator_event_handler->get_statistics();
    }

    const LiftedAxiomEvaluatorStatistics& get_axiom_evaluator_statistics() const { return m_axiom_evaluator_event_handler->get_statistics(); }
};

/// @brief Instantiate a grounded IW search
class GroundedIWPlanner
{
private:
    PDDLParser m_parser;

    DeleteRelaxedProblemExplorator m_delete_relaxed_problem_explorator;
    std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> m_applicable_action_generator_event_handler;
    std::shared_ptr<GroundedApplicableActionGenerator> m_applicable_action_generator;
    std::shared_ptr<IGroundedAxiomEvaluatorEventHandler> m_axiom_evaluator_event_handler;
    std::shared_ptr<GroundedAxiomEvaluator> m_axiom_evaluator;
    std::shared_ptr<StateRepository> m_state_repository;
    std::shared_ptr<IBrFSAlgorithmEventHandler> m_brfs_event_handler;
    std::shared_ptr<IIWAlgorithmEventHandler> m_iw_event_handler;
    std::unique_ptr<IAlgorithm> m_algorithm;

public:
    GroundedIWPlanner(const fs::path& domain_file, const fs::path& problem_file, int arity) :
        m_parser(PDDLParser(domain_file, problem_file)),
        m_delete_relaxed_problem_explorator(m_parser.get_problem(), m_parser.get_pddl_repositories()),
        m_applicable_action_generator_event_handler(std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>()),
        m_applicable_action_generator(
            m_delete_relaxed_problem_explorator.create_grounded_applicable_action_generator(m_applicable_action_generator_event_handler)),
        m_axiom_evaluator_event_handler(std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>()),
        m_axiom_evaluator(m_delete_relaxed_problem_explorator.create_grounded_axiom_evaluator(m_axiom_evaluator_event_handler)),
        m_state_repository(std::make_shared<StateRepository>(m_axiom_evaluator)),
        m_brfs_event_handler(std::make_shared<DefaultBrFSAlgorithmEventHandler>()),
        m_iw_event_handler(std::make_shared<DefaultIWAlgorithmEventHandler>()),
        m_algorithm(std::make_unique<IWAlgorithm>(m_applicable_action_generator, m_state_repository, arity, m_brfs_event_handler, m_iw_event_handler))
    {
    }

    SearchResult find_solution() { return m_algorithm->find_solution(); }

    const IWAlgorithmStatistics& get_iw_statistics() const { return m_iw_event_handler->get_statistics(); }

    const GroundedApplicableActionGeneratorStatistics& get_applicable_action_generator_statistics() const
    {
        return m_applicable_action_generator_event_handler->get_statistics();
    }

    const GroundedAxiomEvaluatorStatistics& get_axiom_evaluator_statistics() const { return m_axiom_evaluator_event_handler->get_statistics(); }
};

TEST(MimirTests, SearchAlgorithmsIWSingleStateTupleIndexGeneratorWidth0Test)
{
    const int arity = 0;
    const int num_atoms = 3;

    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StateTupleIndexGenerator(tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        num_atoms,  // placeholder to generate tuples of size less than arity
    });

    auto iter = generator.begin(atom_indices);

    EXPECT_EQ("()", tuple_index_mapper->tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWSingleStateTupleIndexGeneratorWidth1Test)
{
    const int arity = 1;
    const int num_atoms = 3;

    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StateTupleIndexGenerator(tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        0,
        2,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });

    auto iter = generator.begin(atom_indices);

    EXPECT_EQ("(0,)", tuple_index_mapper->tuple_index_to_string(*iter));
    EXPECT_EQ("(2,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("()", tuple_index_mapper->tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWSingleStateTupleIndexGeneratorWidth2Test)
{
    const int arity = 2;
    const int num_atoms = 3;

    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StateTupleIndexGenerator(tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        0,
        2,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });

    auto iter = generator.begin(atom_indices);

    EXPECT_EQ("(0,2,)", tuple_index_mapper->tuple_index_to_string(*iter));
    EXPECT_EQ("(0,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("()", tuple_index_mapper->tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWStatePairTupleIndexGeneratorWidth1Test)
{
    const int arity = 1;
    const int num_atoms = 4;

    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StatePairTupleIndexGenerator(tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        0,
        2,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });
    const auto add_atom_indices = AtomIndexList({
        1,
        3,
    });

    auto iter = generator.begin(atom_indices, add_atom_indices);

    EXPECT_EQ("(1,)", tuple_index_mapper->tuple_index_to_string(*iter));
    EXPECT_EQ("(3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWStatePairTupleIndexGeneratorWidth2Test1)
{
    const int arity = 2;
    const int num_atoms = 4;

    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StatePairTupleIndexGenerator(tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        0,
        2,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });
    const auto add_atom_indices = AtomIndexList({
        1,
        3,
    });

    auto iter = generator.begin(atom_indices, add_atom_indices);

    EXPECT_EQ("(1,2,)", tuple_index_mapper->tuple_index_to_string(*iter));
    EXPECT_EQ("(1,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,1,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(1,3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWStatePairTupleIndexGeneratorWidth2Test2)
{
    const int arity = 3;
    const int num_atoms = 64;

    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StatePairTupleIndexGenerator(tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        0,
        2,
        3,
        4,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });
    const auto add_atom_indices = AtomIndexList({
        6,
    });

    auto iter = generator.begin(atom_indices, add_atom_indices);

    EXPECT_EQ("(6,)", tuple_index_mapper->tuple_index_to_string(*iter));
    EXPECT_EQ("(0,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(3,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(4,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,2,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,3,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,4,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,3,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,4,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(3,4,6,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWStatePairTupleIndexGeneratorWidth2Test3)
{
    const int arity = 2;
    const int num_atoms = 64;

    const auto tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, num_atoms);
    auto generator = StatePairTupleIndexGenerator(tuple_index_mapper);
    const auto atom_indices = AtomIndexList({
        0,
        1,
        num_atoms,  // placeholder to generate tuples of size less than arity
    });
    const auto add_atom_indices = AtomIndexList({
        2,
        3,
    });

    auto iter = generator.begin(atom_indices, add_atom_indices);

    EXPECT_EQ("(2,)", tuple_index_mapper->tuple_index_to_string(*iter));
    EXPECT_EQ("(3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,2,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(1,2,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(1,3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,3,)", tuple_index_mapper->tuple_index_to_string(*(++iter)));

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

    const auto& applicable_action_generator_statistics = iw.get_applicable_action_generator_statistics();
    const auto& axiom_evaluator_statistics = iw.get_axiom_evaluator_statistics();

    EXPECT_EQ(applicable_action_generator_statistics.get_num_delete_free_reachable_fluent_ground_atoms(), 10);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_delete_free_reachable_derived_ground_atoms(), 0);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_delete_free_actions(), 16);
    EXPECT_EQ(axiom_evaluator_statistics.get_num_delete_free_axioms(), 0);

    EXPECT_EQ(applicable_action_generator_statistics.get_num_ground_actions(), 16);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_nodes_in_action_match_tree(), 32);

    EXPECT_EQ(axiom_evaluator_statistics.get_num_ground_axioms(), 0);
    EXPECT_EQ(axiom_evaluator_statistics.get_num_nodes_in_axiom_match_tree(), 1);

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

    const auto& applicable_action_generator_statistics = iw.get_applicable_action_generator_statistics();
    const auto& axiom_evaluator_statistics = iw.get_axiom_evaluator_statistics();

    EXPECT_EQ(applicable_action_generator_statistics.get_num_delete_free_reachable_fluent_ground_atoms(), 9);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_delete_free_reachable_derived_ground_atoms(), 8);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_delete_free_actions(), 7);
    EXPECT_EQ(axiom_evaluator_statistics.get_num_delete_free_axioms(), 20);

    EXPECT_EQ(applicable_action_generator_statistics.get_num_ground_actions(), 10);
    EXPECT_EQ(applicable_action_generator_statistics.get_num_nodes_in_action_match_tree(), 45);

    EXPECT_EQ(axiom_evaluator_statistics.get_num_ground_axioms(), 16);
    EXPECT_EQ(axiom_evaluator_statistics.get_num_nodes_in_axiom_match_tree(), 12);

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
