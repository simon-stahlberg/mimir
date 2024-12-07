/*
 * Copyright (C) 2024 Dominik Drexler and Till Hofmann
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

#include "mimir/graphs/object_graph_pruning_strategy.hpp"

#include "mimir/formalism/parser.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/axiom_evaluators.hpp"

#include <gtest/gtest.h>

namespace mimir
{

TEST(MimirTests, GraphsObjectGraphPruningStrategySingleSCCTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "delivery/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "delivery/test_problem.pddl");
    const auto parser = PDDLParser(domain_file, problem_file);
    const auto applicable_action_generator = std::make_shared<LiftedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_repositories());
    const auto axiom_evaluator =
        std::dynamic_pointer_cast<IAxiomEvaluator>(std::make_shared<LiftedAxiomEvaluator>(parser.get_problem(), parser.get_pddl_repositories()));
    const auto state_repository = std::make_shared<StateRepository>(axiom_evaluator);
    const auto pruning_strategy =
        ObjectGraphStaticSccPruningStrategy::create(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator, state_repository)
            .value();

    EXPECT_EQ(pruning_strategy.get_pruning_components().size(), 1);
    EXPECT_EQ(pruning_strategy.get_num_components(), 1);
}

TEST(MimirTests, GraphsObjectGraphPruningStrategyMultiSCCTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "spanner/test_problem.pddl");
    const auto parser = PDDLParser(domain_file, problem_file);
    const auto applicable_action_generator = std::make_shared<LiftedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_repositories());
    const auto axiom_evaluator =
        std::dynamic_pointer_cast<IAxiomEvaluator>(std::make_shared<LiftedAxiomEvaluator>(parser.get_problem(), parser.get_pddl_repositories()));
    const auto state_repository = std::make_shared<StateRepository>(axiom_evaluator);
    const auto pruning_strategy =
        ObjectGraphStaticSccPruningStrategy::create(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator, state_repository)
            .value();

    EXPECT_EQ(pruning_strategy.get_pruning_components().size(), 6);
    EXPECT_EQ(pruning_strategy.get_num_components(), 6);

    // initial state, no pruning
    const auto& pruning_component_0 = pruning_strategy.get_pruning_components().at(pruning_strategy.get_component_map().at(0));
    EXPECT_EQ(pruning_component_0.m_pruned_objects.count(), 0);

    // moved 1 forward
    const auto& pruning_component_1 = pruning_strategy.get_pruning_components().at(pruning_strategy.get_component_map().at(1));
    EXPECT_EQ(pruning_component_1.m_pruned_objects.count(), 1);

    // moved 1 forward + picked spanner
    const auto& pruning_component_2 = pruning_strategy.get_pruning_components().at(pruning_strategy.get_component_map().at(2));
    EXPECT_EQ(pruning_component_2.m_pruned_objects.count(), 1);

    // moved 2 forward -> deadend state
    const auto& pruning_component_3 = pruning_strategy.get_pruning_components().at(pruning_strategy.get_component_map().at(3));
    EXPECT_EQ(pruning_component_3.m_pruned_objects.count(), 5);

    // moved 2 forward + picked spanner
    const auto& pruning_component_4 = pruning_strategy.get_pruning_components().at(pruning_strategy.get_component_map().at(4));
    EXPECT_EQ(pruning_component_4.m_pruned_objects.count(), 2);

    // moved 2 forward + picked spanner + tightned nut -> goal state, maximum pruning
    const auto& pruning_component_5 = pruning_strategy.get_pruning_components().at(pruning_strategy.get_component_map().at(5));
    EXPECT_EQ(pruning_component_5.m_pruned_objects.count(), 6);
}

}
