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
    if (argc != 5)
    {
        std::cout << "Usage: pcg <domain:str> <problems:str> <max_num_state:int> <pruning_strategy:int>" << std::endl;
        return 1;
    }

    const auto domain_file_path = fs::path { argv[1] };
    const auto problems_directory = fs::path { argv[2] };

    auto contexts = mimir::datasets::ProblemContextList();
    for (const auto& problem_filepath : fs::directory_iterator(problems_directory))
    {
        auto parser = PDDLParser(domain_file_path, problem_filepath.path());
        auto delete_relaxed_problem_exploration = DeleteRelaxedProblemExplorator(parser.get_problem(), parser.get_pddl_repositories());
        auto applicable_action_generator = delete_relaxed_problem_exploration.create_grounded_applicable_action_generator();
        auto axiom_evaluator = delete_relaxed_problem_exploration.create_grounded_axiom_evaluator();
        auto state_repository = std::make_shared<StateRepository>(axiom_evaluator);
        contexts.emplace_back(parser.get_problem(), state_repository, applicable_action_generator);
    }

    auto pcg = mimir::datasets::ProblemClassGraph(contexts);

    for (size_t i = 0; i < pcg.get_problem_graphs().size(); ++i)
    {
        const auto& pg_i = pcg.get_problem_graphs()[i];
        std::cout << i << ". has num states: " << pg_i.get_num_vertices() << " and num edges: " << pg_i.get_num_edges() << std::endl;
    }

    const auto& cg = pcg.get_class_graph();

    std::cout << "Class graph has num states: " << cg.get_num_vertices() << " and num edges: " << cg.get_num_edges() << std::endl;

    return 0;
}
