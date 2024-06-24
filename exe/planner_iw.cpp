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
    if (argc != 7)
    {
        std::cout << "Usage: planner_iw <domain:str> <problem:str> <plan:str> <arity:int> <grounded:bool> <debug:bool>" << std::endl;
        return 1;
    }

    const auto domain_file_path = fs::path { argv[1] };
    const auto problem_file_path = fs::path { argv[2] };
    const auto plan_file_name = argv[3];
    const auto arity = static_cast<int>(std::atoi(argv[4]));
    const auto grounded = static_cast<bool>(std::atoi(argv[5]));
    const auto debug = static_cast<bool>(std::atoi(argv[6]));

    std::cout << "Parsing PDDL files..." << std::endl;

    auto parser = PDDLParser(domain_file_path, problem_file_path);

    std::cout << "Domain:" << std::endl;
    std::cout << *parser.get_domain() << std::endl;

    std::cout << std::endl;
    std::cout << "Problem:" << std::endl;
    std::cout << *parser.get_problem() << std::endl;

    auto applicable_action_generator =
        (grounded) ? std::shared_ptr<IAAG> { std::make_shared<GroundedAAG>(parser.get_problem(),
                                                                           parser.get_factories(),
                                                                           std::make_shared<DebugGroundedAAGEventHandler>(false)) } :
                     std::shared_ptr<IAAG> {
                         std::make_shared<LiftedAAG>(parser.get_problem(), parser.get_factories(), std::make_shared<DebugLiftedAAGEventHandler>(false))
                     };

    auto successor_state_generator = std::make_shared<SuccessorStateGenerator>(applicable_action_generator);

    auto brfs_event_handler = (debug) ? std::shared_ptr<IBrFSAlgorithmEventHandler> { std::make_shared<DebugBrFSAlgorithmEventHandler>() } :
                                        std::shared_ptr<IBrFSAlgorithmEventHandler> { std::make_shared<DefaultBrFSAlgorithmEventHandler>() };

    auto iw_event_handler = std::make_shared<DefaultIWAlgorithmEventHandler>(false);

    auto iw = std::make_shared<IWAlgorithm>(applicable_action_generator, arity, successor_state_generator, brfs_event_handler, iw_event_handler);

    auto planner = std::make_shared<SinglePlanner>(std::move(iw));

    auto [stats, plan] = planner->find_solution();

    if (stats == SearchStatus::SOLVED)
    {
        std::ofstream plan_file;
        plan_file.open(plan_file_name);
        if (!plan_file.is_open())
        {
            std::cerr << "Error opening file!" << std::endl;
            return 1;
        }
        plan_file << plan;
        plan_file.close();
    }

    return 0;
}
