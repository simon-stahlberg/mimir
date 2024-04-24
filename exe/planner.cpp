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

#include <iostream>

using namespace mimir;

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        std::cout << "Usage: planner <domain:str> <problem:str> <grounded:bool>" << std::endl;
        return 1;
    }

    const auto domain_file_path = fs::path { argv[1] };
    const auto problem_file_path = fs::path { argv[2] };
    const auto grounded = bool { std::atoi(argv[3]) };

    std::cout << "Parsing PDDL files..." << std::endl;

    auto parser = PDDLParser(domain_file_path, problem_file_path);

    std::cout << "Domain:" << std::endl;
    std::cout << *parser.get_domain() << std::endl;

    std::cout << std::endl;
    std::cout << "Problem:" << std::endl;
    std::cout << *parser.get_problem() << std::endl;

    std::cout << "Initializing planner..." << std::endl;

    auto state_repository = std::make_shared<SSG<SSGDispatcher<DenseStateTag>>>(parser.get_problem());

    auto successor_generator =
        (grounded) ?
            std::shared_ptr<IDynamicAAG> { std::make_shared<AAG<GroundedAAGDispatcher<DenseStateTag>>>(parser.get_problem(), parser.get_factories()) } :
            std::shared_ptr<IDynamicAAG> { std::make_shared<AAG<LiftedAAGDispatcher<DenseStateTag>>>(parser.get_problem(), parser.get_factories()) };
    auto event_handler = std::make_shared<MinimalEventHandler>();
    auto lifted_brfs = std::make_shared<BrFsAlgorithm>(parser.get_problem(),
                                                       parser.get_factories(),
                                                       std::move(state_repository),
                                                       std::move(successor_generator),
                                                       std::move(event_handler));

    auto planner = std::make_shared<SinglePlanner>(parser.get_domain(), parser.get_problem(), parser.get_factories(), std::move(lifted_brfs));

    std::cout << "Finding solution..." << std::endl;

    auto [stats, plan] = planner->find_solution();

    return 0;
}
