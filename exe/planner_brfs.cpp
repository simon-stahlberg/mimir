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
    if (argc != 6)
    {
        std::cout << "Usage: planner_brfs <domain:str> <problem:str> <plan:str> <grounded:bool> <debug:bool>" << std::endl;
        return 1;
    }

    const auto start_time = std::chrono::high_resolution_clock::now();

    const auto domain_file_path = fs::path { argv[1] };
    const auto problem_file_path = fs::path { argv[2] };
    const auto plan_file_name = argv[3];
    const auto grounded = static_cast<bool>(std::atoi(argv[4]));
    const auto debug = static_cast<bool>(std::atoi(argv[5]));

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
        applicable_action_generator =
            std::dynamic_pointer_cast<IApplicableActionGenerator>(delete_relaxed_problem_explorator.create_grounded_applicable_action_generator(
                match_tree::Options(),
                std::make_shared<GroundedApplicableActionGenerator::DefaultEventHandler>(false)));
        axiom_evaluator = std::dynamic_pointer_cast<IAxiomEvaluator>(
            delete_relaxed_problem_explorator.create_grounded_axiom_evaluator(match_tree::Options(),
                                                                              std::make_shared<GroundedAxiomEvaluator::DefaultEventHandler>(false)));
        state_repository = std::make_shared<StateRepositoryImpl>(axiom_evaluator);
    }
    else
    {
        applicable_action_generator = std::dynamic_pointer_cast<IApplicableActionGenerator>(
            std::make_shared<LiftedApplicableActionGenerator>(problem, std::make_shared<LiftedApplicableActionGenerator::DefaultEventHandler>(false)));
        axiom_evaluator = std::dynamic_pointer_cast<IAxiomEvaluator>(
            std::make_shared<LiftedAxiomEvaluator>(problem, std::make_shared<LiftedAxiomEvaluator::DefaultEventHandler>(false)));
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

    if (debug)
    {
        std::shared_ptr<LiftedApplicableActionGenerator> lifted_applicable_action_generator =
            std::dynamic_pointer_cast<LiftedApplicableActionGenerator>(applicable_action_generator);

        if (lifted_applicable_action_generator)
        {
            // std::cout << *lifted_applicable_action_generator << std::endl;
        }
    }

    auto event_handler = (debug) ? brfs::EventHandler { std::make_shared<brfs::DebugEventHandler>(problem, false) } :
                                   brfs::EventHandler { std::make_shared<brfs::DefaultEventHandler>(problem, false) };

    auto search_context = SearchContext(problem, applicable_action_generator, state_repository);

    auto result = brfs::find_solution(search_context, nullptr, event_handler);

    std::cout << "[BrFS] Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time)
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
