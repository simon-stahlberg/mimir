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

#include <argparse/argparse.hpp>
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
    program.add_argument("-A", "--arity").default_value(size_t(1)).scan<'u', size_t>().help("The arity used in novelty search.");
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
    auto arity = program.get<size_t>("--arity");
    auto grounded = static_cast<bool>(program.get<size_t>("--enable-grounding"));
    auto verbosity = program.get<size_t>("--verbosity");

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

    auto brfs_event_handler = (verbosity > 1) ? brfs::EventHandler { brfs::DebugEventHandlerImpl::create(problem) } :
                                                brfs::EventHandler { brfs::DefaultEventHandlerImpl::create(problem) };

    auto iw_event_handler = iw::DefaultEventHandlerImpl::create(problem);

    auto siw_event_handler = siw::DefaultEventHandlerImpl::create(problem, false);

    auto search_context = SearchContextImpl::create(problem, applicable_action_generator, state_repository);

    auto siw_options = siw::Options();
    siw_options.max_arity = arity;
    siw_options.iw_event_handler = iw_event_handler;
    siw_options.brfs_event_handler = brfs_event_handler;
    siw_options.siw_event_handler = siw_event_handler;

    auto result = siw::find_solution(search_context, siw_options);

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
