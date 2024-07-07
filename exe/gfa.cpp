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
    if (argc != 4)
    {
        std::cout << "Usage: planner_brfs <domain:str> <problems:str> <max_num_state:int>" << std::endl;
        return 1;
    }

    const auto domain_file_path = fs::path { argv[1] };
    const auto problems_directory = fs::path { argv[2] };
    const auto max_num_states = std::atoi(argv[3]);

    auto problem_filepaths = std::vector<fs::path> {};
    for (const auto& problem_filepath : fs::directory_iterator(problems_directory))
    {
        std::cout << problem_filepath.path() << std::endl;
        problem_filepaths.push_back(problem_filepath.path());
    }

    auto gfas = FaithfulAbstraction::create(domain_file_path, problem_filepaths, true, true, true, false, max_num_states);

    size_t num_states = 0;
    size_t num_transitions = 0;
    size_t num_ground_actions = 0;
    size_t num_ground_axioms = 0;
    for (const auto& gfa : gfas)
    {
        num_states += gfa.get_num_states();
        num_transitions += gfa.get_num_transitions();
        num_ground_actions += gfa.get_aag()->get_num_ground_actions();
        num_ground_axioms += gfa.get_aag()->get_num_ground_axioms();
    }

    std::cout << "Num gfas: " << gfas.size() << std::endl;
    std::cout << "Num states: " << num_states << std::endl;
    std::cout << "Num transitions: " << num_transitions << std::endl;
    std::cout << "Num ground actions: " << num_ground_actions << std::endl;
    std::cout << "Num ground axioms: " << num_ground_axioms << std::endl;

    return 0;
}
