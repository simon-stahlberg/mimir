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
    if (argc != 3)
    {
        std::cout << "Usage: pcg <domain_filepath:str> <problems_directory:str>" << std::endl;
        return 1;
    }

    const auto domain_file_path = fs::path { argv[1] };
    const auto problems_directory = fs::path { argv[2] };

    auto state_space_options = GeneralizedStateSpace::Options();
    state_space_options.problem_options.symmetry_pruning = false;

    auto tuple_graph_options = TupleGraphCollection::Options();
    tuple_graph_options.width = 2;

    auto kb =
        KnowledgeBase::create(GeneralizedSearchContext(domain_file_path, problems_directory), KnowledgeBase::Options(state_space_options, tuple_graph_options));
    const auto& pcss = kb->get_generalized_state_space();

    for (size_t i = 0; i < pcss.get_problem_graphs().size(); ++i)
    {
        const auto& pss_i = pcss.get_problem_graphs()[i];
        std::cout << i << ". has num vertices: " << pss_i.get_graph().get_num_vertices() << " and num edges: " << pss_i.get_graph().get_num_edges()
                  << std::endl;
        // std::cout << pss_i.get_graph() << std::endl;
    }

    const auto& css = pcss.get_class_state_space();

    std::cout << "Class graph has num vertices: " << css.get_graph().get_num_vertices() << " and num edges: " << css.get_graph().get_num_edges() << std::endl;
    std::cout << css.get_graph() << std::endl;

    if (kb->get_tuple_graphs().has_value())
    {
        auto class_v_idx = size_t(0);
        for (const auto& tuple_graph : kb->get_tuple_graphs().value().get_per_class_vertex_tuple_graph())
        {
            // std::cout << "Class vertex index: " << class_v_idx++ << std::endl;
            // std::cout << tuple_graph << std::endl;
        }
    }

    return 0;
}
