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
        std::cout << "Usage: gfa <domain:str> <problems:str> <max_num_state:int> <pruning_strategy:int>" << std::endl;
        return 1;
    }

    const auto domain_file_path = fs::path { argv[1] };
    const auto problems_directory = fs::path { argv[2] };
    const auto max_num_states = std::atoi(argv[3]);
    const auto pruning_strategy = static_cast<ObjectGraphPruningStrategyEnum>(std::atoi(argv[4]));

    auto problem_filepaths = std::vector<fs::path> {};
    for (const auto& problem_filepath : fs::directory_iterator(problems_directory))
    {
        std::cout << problem_filepath.path() << std::endl;
        problem_filepaths.push_back(problem_filepath.path());
    }

    /* State spaces */
    auto state_spaces_options = StateSpacesOptions();
    state_spaces_options.state_space_options.max_num_states = max_num_states;
    auto state_spaces = StateSpace::create(domain_file_path, problem_filepaths, state_spaces_options);

    size_t num_ss_states = 0;
    size_t num_ss_transitions = 0;
    size_t num_ss_ground_actions = 0;
    size_t num_ss_ground_axioms = 0;
    for (const auto& state_space : state_spaces)
    {
        num_ss_states += state_space.get_num_vertices();
        num_ss_transitions += state_space.get_num_edges();
        num_ss_ground_actions += state_space.get_applicable_action_generator()->get_action_grounder().get_num_ground_actions();
        num_ss_ground_axioms += state_space.get_state_repository()->get_axiom_evaluator()->get_axiom_grounder().get_num_ground_axioms();
    }
    std::cout << "Num sss: " << state_spaces.size() << std::endl;
    std::cout << "Num ss states: " << num_ss_states << std::endl;
    std::cout << "Num ss transitions: " << num_ss_transitions << std::endl;
    std::cout << "Num ss ground actions: " << num_ss_ground_actions << std::endl;
    std::cout << "Num ss ground axioms: " << num_ss_ground_axioms << std::endl;

    auto memories =
        std::vector<std::tuple<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>> {};
    for (const auto& state_space : state_spaces)
    {
        memories.emplace_back(state_space.get_problem(),
                              state_space.get_pddl_repositories(),
                              state_space.get_applicable_action_generator(),
                              state_space.get_state_repository());
    }

    /* Gfas */
    auto options = FaithfulAbstractionsOptions();
    options.fa_options.pruning_strategy = pruning_strategy;
    auto gfas = GlobalFaithfulAbstraction::create(memories, options);

    size_t num_gfa_states = 0;
    size_t num_gfa_transitions = 0;
    size_t num_gfa_ground_actions = 0;
    size_t num_gfa_ground_axioms = 0;
    size_t num_isomorphic_states = 0;
    size_t num_non_isomorphic_states = 0;
    for (const auto& gfa : gfas)
    {
        std::cout << gfa.get_problem()->get_filepath().value() << " " << gfa.get_num_vertices() << " " << gfa.get_num_isomorphic_states() << " "
                  << gfa.get_num_non_isomorphic_states() << std::endl;
        num_gfa_states += gfa.get_num_vertices();
        num_gfa_transitions += gfa.get_num_edges();
        num_gfa_ground_actions += gfa.get_applicable_action_generator()->get_action_grounder().get_num_ground_actions();
        num_gfa_ground_axioms += gfa.get_state_repository()->get_axiom_evaluator()->get_axiom_grounder().get_num_ground_axioms();
        num_isomorphic_states += gfa.get_num_isomorphic_states();
        num_non_isomorphic_states += gfa.get_num_non_isomorphic_states();

        /*
        for (const auto& gfa_state : gfa.get_states())
        {
            std::cout << std::make_tuple(gfa.get_problem(),
                                            gfa.get_abstractions()
                                                .at(gfa_state.get_faithful_abstraction_index())
                                                .get_states()
                                                .at(gfa_state.get_faithful_abstraction_vertex_index())
                                                .get_representative_state(),
                                            std::cref(*gfa.get_pddl_repositories()))
                        << std::endl;
        }
        */
    }
    std::cout << "Num gfas: " << gfas.size() << std::endl;
    std::cout << "Num gfa states: " << num_gfa_states << std::endl;
    std::cout << "Num gfa transitions: " << num_gfa_transitions << std::endl;
    std::cout << "Num gfa ground actions: " << num_gfa_ground_actions << std::endl;
    std::cout << "Num gfa ground axioms: " << num_gfa_ground_axioms << std::endl;
    std::cout << "Num isomorphic states: " << num_isomorphic_states << std::endl;
    std::cout << "Num non-isomorphic states: " << num_non_isomorphic_states << std::endl;

    return 0;
}
