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

#include <chrono>
#include <fstream>
#include <iostream>

using namespace mimir;

int main(int argc, char** argv)
{
    if (argc != 12)
    {
        std::cout << "Usage: planner_astar <domain:str> <problem:str> <plan:str> <heuristic_type:int> <grounded:bool> <debug:bool> "
                     "<mt-enable_dump_dot_file:bool> <mt-max_num_nodes:int> <mt-split_strategy:int> <mt-split_metric:int> <mt-opt_dir:int>"
                  << std::endl;
        return 1;
    }

    const auto start_time = std::chrono::high_resolution_clock::now();

    const auto domain_file_path = fs::path { argv[1] };
    const auto problem_file_path = fs::path { argv[2] };
    const auto plan_file_name = argv[3];
    const auto heuristic_type = atoi(argv[4]);
    const auto grounded = static_cast<bool>(std::atoi(argv[5]));
    const auto debug = static_cast<bool>(std::atoi(argv[6]));

    auto match_tree_options = match_tree::Options();
    match_tree_options.enable_dump_dot_file = static_cast<bool>(std::atoi(argv[7]));
    match_tree_options.max_num_nodes = static_cast<size_t>(std::atoi(argv[8]));
    match_tree_options.split_strategy = static_cast<match_tree::SplitStrategyEnum>(std::atoi(argv[9]));
    match_tree_options.split_metric = static_cast<match_tree::SplitMetricEnum>(std::atoi(argv[10]));
    match_tree_options.optimization_direction = static_cast<match_tree::OptimizationDirectionEnum>(std::atoi(argv[11]));

    std::cout << "Parsing PDDL files..." << std::endl;

    auto problem_context = ProblemContext(domain_file_path, problem_file_path);

    if (debug)
    {
        std::cout << "Domain:" << std::endl;
        std::cout << *problem_context.get_problem()->get_domain() << std::endl;

        std::cout << std::endl;
        std::cout << "Problem:" << std::endl;
        std::cout << *problem_context.get_problem() << std::endl;

        std::cout << std::endl;
        std::cout << "Static Predicates:" << std::endl;
        std::cout << problem_context.get_problem()->get_domain()->get_predicates<StaticTag>() << std::endl;

        std::cout << std::endl;
        std::cout << "Fluent Predicates:" << std::endl;
        std::cout << problem_context.get_problem()->get_domain()->get_predicates<FluentTag>() << std::endl;

        std::cout << std::endl;
        std::cout << "Derived Predicates:" << std::endl;
        std::cout << problem_context.get_problem()->get_domain()->get_predicates<DerivedTag>() << std::endl;
        std::cout << std::endl;
    }

    auto applicable_action_generator = ApplicableActionGenerator(nullptr);
    auto axiom_evaluator = AxiomEvaluator(nullptr);
    auto state_repository = StateRepository(nullptr);

    if (grounded)
    {
        auto delete_relaxed_problem_explorator = DeleteRelaxedProblemExplorator(problem_context);
        applicable_action_generator =
            std::dynamic_pointer_cast<IApplicableActionGenerator>(delete_relaxed_problem_explorator.create_grounded_applicable_action_generator(
                match_tree_options,
                std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>(false)));
        axiom_evaluator = std::dynamic_pointer_cast<IAxiomEvaluator>(
            delete_relaxed_problem_explorator.create_grounded_axiom_evaluator(match_tree_options,
                                                                              std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>(false)));
        state_repository = std::make_shared<StateRepositoryImpl>(axiom_evaluator);
    }
    else
    {
        applicable_action_generator = std::dynamic_pointer_cast<IApplicableActionGenerator>(
            std::make_shared<LiftedApplicableActionGenerator>(problem_context, std::make_shared<DefaultLiftedApplicableActionGeneratorEventHandler>(false)));
        axiom_evaluator = std::dynamic_pointer_cast<IAxiomEvaluator>(
            std::make_shared<LiftedAxiomEvaluator>(problem_context, std::make_shared<DefaultLiftedAxiomEvaluatorEventHandler>(false)));
        state_repository = std::make_shared<StateRepositoryImpl>(axiom_evaluator);
    }

    if (debug)
    {
        std::shared_ptr<LiftedApplicableActionGenerator> lifted_applicable_action_generator =
            std::dynamic_pointer_cast<LiftedApplicableActionGenerator>(applicable_action_generator);

        if (lifted_applicable_action_generator)
        {
            // std::cout << *lifted_applicable_action_generator << std::endl;
        }
    }

    auto event_handler = (debug) ? EventHandler { std::make_shared<DebugEventHandler>(false) } : EventHandler { std::make_shared<DefaultEventHandler>(false) };

    auto heuristic = Heuristic(nullptr);
    if (heuristic_type == 0)
    {
        heuristic = std::make_shared<BlindHeuristic>(problem_context.get_problem());
    }
    assert(heuristic);

    auto search_context = SearchContext(problem_context, applicable_action_generator, state_repository);

    auto result = find_solution_astar(search_context, heuristic, std::nullopt, event_handler);

    std::cout << "[AStar] Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time)
              << std::endl;

    if (result.status == SearchStatus::SOLVED)
    {
        std::ofstream plan_file;
        plan_file.open(plan_file_name);
        if (!plan_file.is_open())
        {
            std::cerr << "Error opening file!" << std::endl;
            return 1;
        }
        plan_file << std::make_tuple(std::cref(result.plan.value()), std::cref(*problem_context.get_repositories()));
        plan_file.close();
    }

    return 0;
}
