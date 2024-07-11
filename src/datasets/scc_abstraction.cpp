/*
 * Copyright (C) 2024 Dominik Drexler and Till Hofmann
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

/*
 * 1. PNF
 * 2. state space
 * 3. SCCs
 * 4. prune static atoms in each SCC (and all successors), starting with leafs
 * 5. compute certificate for each state (with pruned atoms) -> faithful abstraction
 */

#include "mimir/graphs/object_graph.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/flat_types.hpp"

#include <mimir/datasets/boost_adapter.hpp>
#include <mimir/datasets/scc_abstraction.hpp>
#include <mimir/formalism/factories.hpp>
#include <mimir/formalism/transformers/encode_parameter_index_in_variables.hpp>
#include <mimir/formalism/transformers/to_positive_normal_form.hpp>
#include <mimir/search/applicable_action_generators/grounded.hpp>
#include <optional>

namespace mimir
{

Digraph create_scc_digraph(size_t num_components, const std::map<StateIndex, size_t>& component_map, const StateSpace& state_space)
{
    auto g = Digraph();
    for (size_t i = 0; i < num_components; ++i)
    {
        g.add_vertex();
    }
    std::set<std::pair<size_t, size_t>> edges;
    for (const auto t : state_space.get_transitions())
    {
        const auto source = t.get_source_state();
        const auto target = t.get_target_state();
        if (component_map.at(source) != component_map.at(target) && !edges.contains({ source, target }))
        {
            g.add_directed_edge(component_map.at(source), component_map.at(target));
        }
    }
    return g;
}

std::optional<SccAbstraction> SccAbstraction::create(Problem problem, bool remove_if_unsolvable, uint32_t max_num_states, uint32_t timeout_ms)
{
    auto pnf_factories = std::make_shared<PDDLFactories>();
    auto pnf_problem = ToPositiveNormalFormTransformer(*pnf_factories).run(*problem);
    pnf_problem = EncodeParameterIndexInVariables(*pnf_factories).run(*pnf_problem);
    std::cout << "original domain:\n" << *problem->get_domain() << std::endl;
    std::cout << "PNF domain:\n" << *pnf_problem->get_domain() << std::endl;
    std::cout << "original problem:\n" << *problem << std::endl;
    std::cout << "PNF problem:\n" << *pnf_problem << std::endl;
    auto pnf_aag = std::make_shared<GroundedAAG>(pnf_problem, pnf_factories);
    auto pnf_ssg = std::make_shared<SuccessorStateGenerator>(pnf_aag);
    auto pnf_state_space = StateSpace::create(pnf_problem, pnf_factories, pnf_aag, pnf_ssg, true, remove_if_unsolvable, max_num_states, timeout_ms);
    if (!pnf_state_space)
    {
        return std::nullopt;
    }
    const auto [num_components, component_map] = strong_components(*pnf_state_space);
    const auto scc_digraph = create_scc_digraph(num_components, component_map, *pnf_state_space);
    const auto partitioning = get_partitioning<StateSpace>(num_components, component_map);
    for (size_t group_index = 0; group_index < partitioning.get_num_groups(); ++group_index)
    {
        auto group = partitioning.get_group(group_index);
        auto group_pruning_strategy = ObjectGraphStaticPruningStrategy();
        auto pruned_fluent_ground_atoms = FlatBitsetBuilder<Fluent>(pnf_state_space->get_states()[group.begin()->second].get_atoms<Fluent>());
        for (const auto& [group_index, state_index] : group)
        {
            pruned_fluent_ground_atoms &= pnf_state_space->get_states()[state_index].get_atoms<Fluent>();
            for (const auto& transition : pnf_state_space->get_forward_transitions(state_index))
            {
                auto precondition = StripsActionPrecondition(transition.get_creating_action().get_strips_precondition());
                pruned_fluent_ground_atoms &= precondition.get_negative_precondition<Fluent>();
                pruned_fluent_ground_atoms &= precondition.get_positive_precondition<Fluent>();
            }
        }
    }

    return std::nullopt;
}
}
