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

#include "utils.hpp"

#include <argparse/argparse.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mimir/mimir.hpp>

using namespace mimir;
using namespace mimir::search;
using namespace mimir::formalism;

int main(int argc, char** argv)
{
    auto program = argparse::ArgumentParser("Greedy best first search.");
    program.add_argument("-D", "--domain-filepath").required().help("The path to the PDDL domain file.");
    program.add_argument("-P", "--problem-filepath").required().help("The path to the PDDL problem file.");
    program.add_argument("-O", "--plan-filepath").required().help("The path to the output plan file.");
    program.add_argument("-E", "--enable-eager")
        .default_value(size_t(0))
        .scan<'u', size_t>()
        .help("Non-zero values enable eager search. Defaults to lazy search.");
    program.add_argument("-W0", "--weight-queue-preferred")
        .default_value(size_t(64))
        .scan<'u', size_t>()
        .help("Weight of the heuristic preferred actions queue. Ignored in eager search.");
    program.add_argument("-W1", "--weight-queue-standard")
        .default_value(size_t(1))
        .scan<'u', size_t>()
        .help("Weight of the standard queue. Ignored in eager search.");
    program.add_argument("-H", "--heuristic-type").default_value("ff").choices("blind", "perfect", "max", "add", "setadd", "ff");
    program.add_argument("-G", "--enable-grounding")
        .default_value(size_t(0))
        .scan<'u', size_t>()
        .help("Non-zero values enabled grounding. Might be necessary for some features. Defaults to grounded.");
    program.add_argument("-V", "--verbosity")
        .default_value(size_t(0))
        .scan<'u', size_t>()
        .help("The verbosity level. Defaults to minimal amount of debug output.");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what() << "\n";
        std::cerr << program;
        std::exit(1);
    }

    auto domain_filepath = program.get<std::string>("--domain-filepath");
    auto problem_filepath = program.get<std::string>("--problem-filepath");
    auto plan_filepath = program.get<std::string>("--plan-filepath");

    auto eager = static_cast<bool>(program.get<size_t>("--enable-eager"));
    auto weight_queue_preferred = program.get<size_t>("--weight-queue-preferred");
    auto weight_queue_standard = program.get<size_t>("--weight-queue-standard");
    auto heuristic_type = get_heuristic_type(program.get<std::string>("--heuristic-type"));
    auto grounded = static_cast<bool>(program.get<size_t>("--enable-grounding"));
    auto verbosity = program.get<size_t>("--verbosity");

    const auto start_time = std::chrono::high_resolution_clock::now();

    std::cout << "Parsing PDDL files..." << std::endl;

    auto problem = ProblemImpl::create(domain_filepath, problem_filepath);

    if (verbosity > 0)
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

    auto heuristic = Heuristic(nullptr);

    if (grounded)
    {
        auto delete_relaxed_problem_explorator = DeleteRelaxedProblemExplorator(problem);
        applicable_action_generator = delete_relaxed_problem_explorator.create_grounded_applicable_action_generator(
            match_tree::Options(),
            GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create(false));
        axiom_evaluator = delete_relaxed_problem_explorator.create_grounded_axiom_evaluator(match_tree::Options(),
                                                                                            GroundedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create(false));
        state_repository = StateRepositoryImpl::create(axiom_evaluator);

        if (heuristic_type == HeuristicType::MAX)
            heuristic = MaxHeuristicImpl::create(delete_relaxed_problem_explorator);
        else if (heuristic_type == HeuristicType::ADD)
            heuristic = AddHeuristicImpl::create(delete_relaxed_problem_explorator);
        else if (heuristic_type == HeuristicType::SETADD)
            heuristic = SetAddHeuristicImpl::create(delete_relaxed_problem_explorator);
        else if (heuristic_type == HeuristicType::FF)
            heuristic = FFHeuristicImpl::create(delete_relaxed_problem_explorator);
    }
    else
    {
        applicable_action_generator =
            LiftedApplicableActionGeneratorImpl::create(problem, LiftedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create(false));
        axiom_evaluator = LiftedAxiomEvaluatorImpl::create(problem, LiftedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create(false));
        state_repository = StateRepositoryImpl::create(axiom_evaluator);

        if (heuristic_type == HeuristicType::MAX)
            throw std::runtime_error("Lifted h_max is not supported");
        else if (heuristic_type == HeuristicType::ADD)
            throw std::runtime_error("Lifted h_add is not supported");
        else if (heuristic_type == HeuristicType::SETADD)
            throw std::runtime_error("Lifted h_setadd is not supported");
        else if (heuristic_type == HeuristicType::FF)
            throw std::runtime_error("Lifted h_ff is not supported");
    }

    auto search_context = SearchContextImpl::create(problem, applicable_action_generator, state_repository);

    if (heuristic_type == HeuristicType::BLIND)
        heuristic = BlindHeuristicImpl::create(problem);
    else if (heuristic_type == HeuristicType::PERFECT)
        heuristic = PerfectHeuristicImpl::create(search_context);

    assert(heuristic);

    auto result = SearchResult();

    if (eager)
    {
        auto event_handler = (verbosity > 1) ? gbfs_eager::EventHandler { gbfs_eager::DebugEventHandlerImpl::create(problem, false) } :
                                               gbfs_eager::EventHandler { gbfs_eager::DefaultEventHandlerImpl::create(problem, false) };

        auto gbfs_options = gbfs_eager::Options();
        gbfs_options.event_handler = event_handler;

        result = gbfs_eager::find_solution(search_context, heuristic, gbfs_options);
    }
    else
    {
        auto event_handler = (verbosity > 1) ? gbfs_lazy::EventHandler { gbfs_lazy::DebugEventHandlerImpl::create(problem, false) } :
                                               gbfs_lazy::EventHandler { gbfs_lazy::DefaultEventHandlerImpl::create(problem, false) };

        auto gbfs_options = gbfs_lazy::Options();
        gbfs_options.event_handler = event_handler;
        gbfs_options.openlist_weights = std::array<size_t, 6> { 1, 1, 1, 1, weight_queue_preferred, weight_queue_standard };

        result = gbfs_lazy::find_solution(search_context, heuristic, gbfs_options);
    }

    std::cout << "[GBFS] Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time)
              << std::endl;

    std::cout << "Peak memory usage in bytes: " << get_peak_memory_usage_in_bytes() << std::endl;

    if (result.status == SearchStatus::SOLVED)
    {
        std::ofstream plan_file;
        plan_file.open(plan_filepath);
        if (!plan_file.is_open())
        {
            std::cerr << "Error opening file!" << std::endl;
            return 1;
        }
        plan_file << result.plan.value();
        plan_file.close();
    }

    return 0;
}
