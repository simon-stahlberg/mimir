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
    auto program = argparse::ArgumentParser("AStar search.");
    program.add_argument("-D", "--domain-filepath").required().help("The path to the PDDL domain file.");
    program.add_argument("-P", "--problem-filepath").required().help("The path to the PDDL problem file.");
    program.add_argument("-O", "--plan-filepath").required().help("The path to the output plan file.");
    program.add_argument("-M", "--search-mode").default_value("lifted").choices("grounded", "lifted");
    program.add_argument("-L", "--lifted-mode").default_value("kpkc").choices("exhaustive", "kpkc");
    program.add_argument("-S", "--lifted-symmetry-pruning-mode").default_value("off").choices("off", "gi", "1-wl");
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
    auto search_mode = get_search_mode(program.get<std::string>("--search-mode"),
                                       program.get<std::string>("--lifted-mode"),
                                       program.get<std::string>("--lifted-symmetry-pruning-mode"));
    auto verbosity = program.get<size_t>("--verbosity");

    const auto start_time = std::chrono::high_resolution_clock::now();

    std::cout << "Parsing PDDL files..." << std::endl;

    auto problem = ProblemImpl::create(domain_filepath, problem_filepath);

    if (verbosity > 0)
    {
        std::cout << "Domain:" << std::endl;
        std::cout << *problem->get_domain() << std::endl;

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

    std::visit(
        [&](auto&& mode)
        {
            using ModeT = std::decay_t<decltype(mode)>;

            if constexpr (std::is_same_v<ModeT, SearchContextImpl::GroundedOptions>)
            {
                auto grounder = std::make_unique<LiftedGrounder>(problem);
                applicable_action_generator =
                    grounder->create_grounded_applicable_action_generator(match_tree::Options(),
                                                                          GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create(false));
                axiom_evaluator =
                    grounder->create_grounded_axiom_evaluator(match_tree::Options(), GroundedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create(false));
                state_repository = StateRepositoryImpl::create(axiom_evaluator);
            }
            else if constexpr (std::is_same_v<ModeT, SearchContextImpl::LiftedOptions>)
            {
                std::visit(
                    [&](auto&& option)
                    {
                        using OptionT = std::decay_t<decltype(option)>;

                        if constexpr (std::is_same_v<OptionT, SearchContextImpl::LiftedOptions::KPKCOptions>)
                        {
                            applicable_action_generator =
                                KPKCLiftedApplicableActionGeneratorImpl::create(problem,
                                                                                option,
                                                                                KPKCLiftedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create(false),
                                                                                satisficing_binding_generator::DefaultEventHandlerImpl::create(false));
                            axiom_evaluator = KPKCLiftedAxiomEvaluatorImpl::create(problem,
                                                                                   KPKCLiftedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create(false),
                                                                                   satisficing_binding_generator::DefaultEventHandlerImpl::create(false));
                            state_repository = StateRepositoryImpl::create(axiom_evaluator);
                        }
                        else if constexpr (std::is_same_v<OptionT, SearchContextImpl::LiftedOptions::ExhaustiveOptions>)
                        {
                            applicable_action_generator = ExhaustiveLiftedApplicableActionGeneratorImpl::create(
                                problem,
                                ExhaustiveLiftedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create(false),
                                satisficing_binding_generator::DefaultEventHandlerImpl::create(false));
                            axiom_evaluator =
                                ExhaustiveLiftedAxiomEvaluatorImpl::create(problem,
                                                                           ExhaustiveLiftedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create(false),
                                                                           satisficing_binding_generator::DefaultEventHandlerImpl::create(false));
                            state_repository = StateRepositoryImpl::create(axiom_evaluator);
                        }
                        else
                        {
                            static_assert(dependent_false<OptionT>::value, "Missing implementation for option.");
                        }
                    },
                    mode.option);
            }
            else
            {
                static_assert(dependent_false<ModeT>::value, "Missing implementation for mode.");
            }
        },
        search_mode);

    auto event_handler = (verbosity > 1) ? brfs::EventHandler { brfs::DebugEventHandlerImpl::create(problem, false) } :
                                           brfs::EventHandler { brfs::DefaultEventHandlerImpl::create(problem, false) };

    auto search_context = SearchContextImpl::create(problem, applicable_action_generator, state_repository);

    auto brfs_options = brfs::Options();
    brfs_options.event_handler = event_handler;

    auto result = brfs::find_solution(search_context, brfs_options);

    std::cout << "[BrFS] Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time)
              << std::endl;

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
