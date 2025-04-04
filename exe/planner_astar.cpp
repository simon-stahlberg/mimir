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
using namespace mimir::search;
using namespace mimir::formalism;

int main(int argc, char** argv)
{
    if (argc != 7)
    {
        std::cout << "Usage: planner_astar <domain:str> <problem:str> <plan:str> <heuristic_type:int> <grounded:bool> <debug:bool>" << std::endl;
        return 1;
    }

    const auto start_time = std::chrono::high_resolution_clock::now();

    const auto domain_file_path = fs::path { argv[1] };
    const auto problem_file_path = fs::path { argv[2] };
    const auto plan_file_name = argv[3];
    const auto heuristic_type = atoi(argv[4]);
    const auto grounded = static_cast<bool>(std::atoi(argv[5]));
    const auto debug = static_cast<bool>(std::atoi(argv[6]));

    std::cout << "Parsing PDDL files..." << std::endl;

    auto problem = ProblemImpl::create(domain_file_path, problem_file_path);

    if (debug)
    {
        std::cout << "Domain:" << std::endl;
        std::cout << problem->get_domain() << std::endl;

        std::cout << std::endl;
        std::cout << "Problem:" << std::endl;
        std::cout << *problem << std::endl;

        std::cout << std::endl;
        std::cout << "Static Predicates:" << std::endl;
        std::cout << problem->get_domain()->get_predicates<StaticTag>() << std::endl;

        std::cout << std::endl;
        std::cout << "Fluent Predicates:" << std::endl;
        std::cout << problem->get_domain()->get_predicates<FluentTag>() << std::endl;

        std::cout << std::endl;
        std::cout << "Derived Predicates:" << std::endl;
        std::cout << problem->get_domain()->get_predicates<DerivedTag>() << std::endl;
        std::cout << std::endl;
    }

    auto applicable_action_generator = ApplicableActionGenerator(nullptr);
    auto axiom_evaluator = AxiomEvaluator(nullptr);
    auto state_repository = StateRepository(nullptr);

    if (grounded)
    {
        auto delete_relaxed_problem_explorator = DeleteRelaxedProblemExplorator(problem);
        applicable_action_generator = delete_relaxed_problem_explorator.create_grounded_applicable_action_generator(
            match_tree::Options(),
            GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create(false));
        axiom_evaluator = delete_relaxed_problem_explorator.create_grounded_axiom_evaluator(match_tree::Options(),
                                                                                            GroundedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create(false));
        state_repository = StateRepositoryImpl::create(axiom_evaluator);
    }
    else
    {
        applicable_action_generator =
            LiftedApplicableActionGeneratorImpl::create(problem, LiftedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create(false));
        axiom_evaluator = LiftedAxiomEvaluatorImpl::create(problem, LiftedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create(false));
        state_repository = StateRepositoryImpl::create(axiom_evaluator);
    }

    auto event_handler = (debug) ? astar::EventHandler { astar::DebugEventHandlerImpl::create(problem, false) } :
                                   astar::EventHandler { astar::DefaultEventHandlerImpl::create(problem, false) };

    auto heuristic = Heuristic(nullptr);
    if (heuristic_type == 0)
    {
        heuristic = BlindHeuristicImpl::create(problem);
    }
    assert(heuristic);

    auto search_context = SearchContextImpl::create(problem, applicable_action_generator, state_repository);

    auto result = astar::find_solution(search_context, heuristic, nullptr, event_handler);

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
        plan_file << std::make_tuple(std::cref(result.plan.value()), std::cref(*problem));
        plan_file.close();
    }

    return 0;
}
