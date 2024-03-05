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

#include "mimir/search/config.hpp"
#include <iostream>
#include <mimir/search/planners.hpp>

using namespace mimir;

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: planner <domain:str> <problem:str>" << std::endl;
        return 1;
    }

    const auto domain_file_path = fs::path { argv[1] };
    const auto problem_file_path = fs::path { argv[2] };

    std::cout << "Initializing planner..." << std::endl;

    auto planner = Planner<SingleTag<BrFSTag<LiftedTag, BitsetStateTag>>>(domain_file_path, problem_file_path);

    std::cout << "Finding solution..." << std::endl;

    auto [stats, plan] = planner.find_solution();

    return 0;
}
