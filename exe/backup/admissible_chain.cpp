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

#include "mimir/mimir.hpp"

#include <fstream>
#include <iostream>

using namespace mimir;

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: planner_brfs <domain:str> <problem:str>" << std::endl;
        return 1;
    }

    const auto domain_file_path = fs::path { argv[1] };
    const auto problem_file_path = fs::path { argv[2] };

    auto parser = PDDLParser(domain_file_path, problem_file_path);
    auto grounder = std::make_shared<Grounder>(parser.get_problem(), parser.get_pddl_repositories());
    auto applicable_action_generator =
        std::dynamic_pointer_cast<IApplicableActionGenerator>(std::make_shared<LiftedApplicableActionGeneratorImpl>(grounder->get_action_grounder()));
    auto axiom_evaluator = std::dynamic_pointer_cast<IAxiomEvaluator>(std::make_shared<LiftedAxiomEvaluator>(grounder->get_axiom_grounder()));
    auto state_repository = std::make_shared<StateRepositoryImpl>(axiom_evaluator);

    const auto state_space = std::make_shared<StateSpace>(std::move(StateSpace::create(applicable_action_generator, state_repository).value()));

    auto fluent_goal_atoms = to_ground_atoms(parser.get_problem()->get_goal_condition<FluentTag>());

    for (size_t arity = 0; arity < MAX_ARITY; ++arity)
    {
        auto tuple_graph_factory = TupleGraphFactory(state_space, arity, false);

        auto tuple_graph = tuple_graph_factory.create(state_space->get_state_repository()->get_or_create_initial_state());

        if (tuple_graph.compute_admissible_chain(fluent_goal_atoms) != std::nullopt)
        {
            std::cout << "Problem width is: " << arity << std::endl;
            return 0;
        }
    }

    std::cout << "Problem width exceeds max arity of " << MAX_ARITY << std::endl;

    return 0;
}
