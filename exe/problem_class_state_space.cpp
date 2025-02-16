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

    auto contexts = mimir::ProblemContextList();
    for (const auto& problem_filepath : fs::directory_iterator(problems_directory))
    {
        auto parser = PDDLParser(domain_file_path, problem_filepath.path());
        auto delete_relaxed_problem_exploration = DeleteRelaxedProblemExplorator(parser.get_problem(), parser.get_pddl_repositories());
        auto applicable_action_generator = delete_relaxed_problem_exploration.create_grounded_applicable_action_generator();
        auto axiom_evaluator = delete_relaxed_problem_exploration.create_grounded_axiom_evaluator();
        auto state_repository = std::make_shared<StateRepository>(axiom_evaluator);
        contexts.emplace_back(parser.get_problem(), state_repository, applicable_action_generator);
    }

    auto pcss = mimir::ProblemClassStateSpace(contexts);

    for (size_t i = 0; i < pcss.get_problem_state_spaces().size(); ++i)
    {
        const auto& pss_i = pcss.get_problem_state_spaces()[i];
        std::cout << i << ". has num vertices: " << pss_i.get_graph().get_num_vertices() << " and num edges: " << pss_i.get_graph().get_num_edges()
                  << std::endl;
        std::cout << pss_i.get_graph() << std::endl;
    }

    const auto& css = pcss.get_class_state_space();

    std::cout << "Class graph has num vertices: " << css.get_graph().get_num_vertices() << " and num edges: " << css.get_graph().get_num_edges() << std::endl;
    std::cout << css.get_graph() << std::endl;

    return 0;
}
