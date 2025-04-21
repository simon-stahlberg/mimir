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

using namespace mimir::datasets;
using namespace mimir::search;

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: pcg <domain_filepath:str> <problems_directory:str>" << std::endl;
        return 1;
    }

    const auto domain_file_path = fs::path { argv[1] };
    const auto problems_directory = fs::path { argv[2] };

    auto kb_options = KnowledgeBaseImpl::Options();
    auto& state_space_options = kb_options.state_space_options;
    state_space_options.symmetry_pruning = true;
    auto& generalized_state_space_options = kb_options.generalized_state_space_options;
    generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
    auto& tuple_graph_options = kb_options.tuple_graph_options;
    tuple_graph_options = TupleGraphImpl::Options();
    tuple_graph_options->width = 1;
    auto kb = KnowledgeBaseImpl::create(GeneralizedSearchContextImpl::create(domain_file_path, problems_directory), kb_options);

    if (kb->get_generalized_state_space().has_value())
    {
        const auto& generalized_state_space = kb->get_generalized_state_space().value();

        for (size_t i = 0; i < generalized_state_space->get_state_spaces().size(); ++i)
        {
            const auto& pss_i = generalized_state_space->get_state_spaces()[i];
            std::cout << i << ". has num vertices: " << pss_i->get_graph().get_num_vertices() << " and num edges: " << pss_i->get_graph().get_num_edges()
                      << std::endl;
            // std::cout << pss_i->get_graph() << std::endl;
        }

        std::cout << "Class graph has num vertices: " << generalized_state_space->get_graph().get_num_vertices()
                  << " and num edges: " << generalized_state_space->get_graph().get_num_edges() << std::endl;
        // std::cout << generalized_state_space.get_graph() << std::endl;
    }

    if (kb->get_tuple_graphs().has_value())
    {
        auto class_v_idx = size_t(0);
        for (const auto& tuple_graphs : kb->get_tuple_graphs().value())
        {
            for (const auto& tuple_graph : tuple_graphs)
            {
                // std::cout << "Class vertex index: " << class_v_idx++ << std::endl;
                // std::cout << *tuple_graph << std::endl;
            }
        }
    }

    return 0;
}
