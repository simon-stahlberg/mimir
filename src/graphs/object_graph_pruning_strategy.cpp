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

#include "mimir/graphs/object_graph_pruning_strategy.hpp"

#include "mimir/datasets/boost_adapter.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/transformers/encode_parameter_index_in_variables.hpp"
#include "mimir/formalism/transformers/to_positive_normal_form.hpp"
#include "mimir/graphs/object_graph.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators/grounded.hpp"
#include "mimir/search/flat_types.hpp"

#include <optional>

namespace mimir
{

/* ObjectGraphStaticPruningStrategy */

ObjectGraphStaticSccPruningStrategy::ObjectGraphStaticSccPruningStrategy(std::vector<SccPruningComponent> pruning_components,
                                                                         std::map<StateIndex, size_t> component_map) :
    m_pruning_components(std::move(pruning_components)),
    m_component_map(std::move(component_map))
{
}

bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, Object object) const
{
    return m_pruning_components.at(m_component_map.at(state)).m_pruned_objects.get(object->get_identifier());
}
bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, GroundAtom<Static> atom) const
{
    return m_pruning_components.at(m_component_map.at(state)).m_pruned_static_ground_atoms.get(atom->get_identifier());
}
bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, GroundAtom<Fluent> atom) const
{
    return m_pruning_components.at(m_component_map.at(state)).m_pruned_fluent_ground_atoms.get(atom->get_identifier());
}
bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, GroundAtom<Derived> atom) const
{
    return m_pruning_components.at(m_component_map.at(state)).m_pruned_derived_ground_atoms.get(atom->get_identifier());
}
bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, GroundLiteral<Static> literal) const
{
    return m_pruning_components.at(m_component_map.at(state)).m_pruned_static_ground_literals.get(literal->get_atom()->get_identifier());
}
bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, GroundLiteral<Fluent> literal) const
{
    return m_pruning_components.at(m_component_map.at(state)).m_pruned_fluent_ground_literals.get(literal->get_atom()->get_identifier());
}
bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, GroundLiteral<Derived> literal) const
{
    return m_pruning_components.at(m_component_map.at(state)).m_pruned_derived_ground_literals.get(literal->get_atom()->get_identifier());
}

ObjectGraphStaticSccPruningStrategy::SccPruningComponent&
ObjectGraphStaticSccPruningStrategy::SccPruningComponent::operator&=(const ObjectGraphStaticSccPruningStrategy::SccPruningComponent& other)
{
    m_pruned_objects &= other.m_pruned_objects;
    m_pruned_static_ground_atoms &= other.m_pruned_static_ground_atoms;
    m_pruned_fluent_ground_atoms &= other.m_pruned_fluent_ground_atoms;
    m_pruned_derived_ground_atoms &= other.m_pruned_derived_ground_atoms;
    m_pruned_static_ground_literals &= other.m_pruned_static_ground_literals;
    m_pruned_fluent_ground_literals &= other.m_pruned_fluent_ground_literals;
    m_pruned_derived_ground_literals &= other.m_pruned_derived_ground_literals;
    return *this;
}

static Digraph create_scc_digraph(size_t num_components, const std::map<StateIndex, size_t>& component_map, const StateSpace& state_space)
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

std::optional<ObjectGraphStaticSccPruningStrategy> ObjectGraphStaticSccPruningStrategy::create(Problem problem, const StateSpaceOptions& options)
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
    auto pnf_state_space = StateSpace::create(pnf_problem, pnf_factories, pnf_aag, pnf_ssg, options);
    if (!pnf_state_space)
    {
        return std::nullopt;
    }
    const auto [num_components, component_map] = strong_components(*pnf_state_space);
    const auto scc_digraph = create_scc_digraph(num_components, component_map, *pnf_state_space);
    const auto partitioning = get_partitioning<StateSpace>(num_components, component_map);
    auto pruning_components = std::vector<ObjectGraphStaticSccPruningStrategy::SccPruningComponent>();
    pruning_components.reserve(partitioning.size());
    for (size_t group_index = 0; group_index < partitioning.size(); ++group_index)
    {
        auto group = partitioning.at(group_index);
        auto pruned_static_ground_atoms = FlatBitsetBuilder<Static>(pnf_state_space->get_problem()->get_static_initial_positive_atoms_bitset());
        auto pruned_fluent_ground_atoms = FlatBitsetBuilder<Fluent>(pnf_state_space->get_states()[group.begin()->second].get_atoms<Fluent>());
        auto pruned_derived_ground_atoms = FlatBitsetBuilder<Derived>(pnf_state_space->get_states()[group.begin()->second].get_atoms<Derived>());
        auto pruned_static_ground_literals = FlatBitsetBuilder<Static>();
        for (const auto& literal : pnf_state_space->get_problem()->get_goal_condition<Static>())
        {
            pruned_static_ground_literals.set(literal->get_identifier());
        }
        auto pruned_fluent_ground_literals = FlatBitsetBuilder<Fluent>();

        for (const auto& literal : pnf_state_space->get_problem()->get_goal_condition<Fluent>())
        {
            pruned_static_ground_literals.set(literal->get_identifier());
        }
        auto pruned_derived_ground_literals = FlatBitsetBuilder<Derived>();
        for (const auto& literal : pnf_state_space->get_problem()->get_goal_condition<Derived>())
        {
            pruned_static_ground_literals.set(literal->get_identifier());
        }
        for (const auto& [group_index, state_index] : group)
        {
            pruned_fluent_ground_atoms &= pnf_state_space->get_states()[state_index].get_atoms<Fluent>();
            pruned_derived_ground_atoms &= pnf_state_space->get_states()[state_index].get_atoms<Derived>();
            for (const auto& transition : pnf_state_space->get_forward_transitions(state_index))
            {
                const auto& precondition = StripsActionPrecondition(transition.get_creating_action().get_strips_precondition());
                pruned_static_ground_atoms -= precondition.get_negative_precondition<Static>();
                pruned_static_ground_atoms -= precondition.get_positive_precondition<Static>();
                pruned_fluent_ground_atoms -= precondition.get_negative_precondition<Fluent>();
                pruned_fluent_ground_atoms -= precondition.get_positive_precondition<Fluent>();
                pruned_derived_ground_atoms -= precondition.get_negative_precondition<Derived>();
                pruned_derived_ground_atoms -= precondition.get_positive_precondition<Derived>();
                for (const auto& literal : transition.get_creating_action().get_action()->get_conditions<Static>())
                {
                    pruned_static_ground_literals.unset(literal->get_identifier());
                }
                for (const auto& literal : transition.get_creating_action().get_action()->get_conditions<Fluent>())
                {
                    pruned_fluent_ground_literals.unset(literal->get_identifier());
                }
                for (const auto& literal : transition.get_creating_action().get_action()->get_conditions<Derived>())
                {
                    pruned_derived_ground_literals.unset(literal->get_identifier());
                }
            }

            // TODO we may also prune goal literals that occur in a condition but are static in the SCC.
            auto pruned_static_ground_literals = FlatBitsetBuilder<Static>();
            for (const auto& literal : pnf_state_space->get_problem()->get_goal_condition<Static>())
            {
                if (pruned_static_ground_atoms.get(literal->get_atom()->get_identifier()))
                {
                    pruned_static_ground_literals.set(literal->get_identifier());
                }
            }
            auto pruned_fluent_ground_literals = FlatBitsetBuilder<Fluent>();
            for (const auto& literal : pnf_state_space->get_problem()->get_goal_condition<Fluent>())
            {
                if (pruned_fluent_ground_atoms.get(literal->get_atom()->get_identifier()))
                {
                    pruned_fluent_ground_literals.set(literal->get_identifier());
                }
            }
            auto pruned_derived_ground_literals = FlatBitsetBuilder<Derived>();
            for (const auto& literal : pnf_state_space->get_problem()->get_goal_condition<Derived>())
            {
                if (pruned_derived_ground_atoms.get(literal->get_atom()->get_identifier()))
                {
                    pruned_derived_ground_literals.set(literal->get_identifier());
                }
            }
        }
        auto pruned_objects = FlatBitsetBuilder<>();
        for (const auto& object : pnf_state_space->get_problem()->get_objects())
        {
            pruned_objects.set(object->get_identifier());
        }
        auto unpruned_static_ground_atoms = FlatBitsetBuilder<Static>(pnf_state_space->get_problem()->get_static_initial_positive_atoms_bitset());
        unpruned_static_ground_atoms -= pruned_static_ground_atoms;
        auto unpruned_fluent_ground_atoms = FlatBitsetBuilder<Fluent>(pnf_ssg->get_reached_fluent_ground_atoms());
        unpruned_fluent_ground_atoms -= pruned_fluent_ground_atoms;
        auto unpruned_derived_ground_atoms = FlatBitsetBuilder<Derived>(pnf_ssg->get_reached_derived_ground_atoms());
        unpruned_derived_ground_atoms -= pruned_derived_ground_atoms;
        for (const auto& atom : pnf_factories->get_ground_atoms_from_ids<Static>(unpruned_static_ground_atoms))
        {
            for (const auto& object : atom->get_objects())
            {
                pruned_objects.unset(object->get_identifier());
            }
        }
        for (const auto& atom : pnf_factories->get_ground_atoms_from_ids<Fluent>(unpruned_fluent_ground_atoms))
        {
            for (const auto& object : atom->get_objects())
            {
                pruned_objects.unset(object->get_identifier());
            }
        }
        for (const auto& atom : pnf_factories->get_ground_atoms_from_ids<Derived>(unpruned_derived_ground_atoms))
        {
            for (const auto& object : atom->get_objects())
            {
                pruned_objects.unset(object->get_identifier());
            }
        }
        pruning_components.push_back(ObjectGraphStaticSccPruningStrategy::SccPruningComponent { pruned_objects,
                                                                                                pruned_static_ground_atoms,
                                                                                                pruned_fluent_ground_atoms,
                                                                                                pruned_derived_ground_atoms,
                                                                                                pruned_static_ground_literals,
                                                                                                pruned_fluent_ground_literals,
                                                                                                pruned_derived_ground_literals });
    }
    auto computed_components = std::vector<bool>(num_components, false);
    auto scc_deque = std::deque<size_t>();
    scc_deque.push_back(component_map.at(pnf_state_space->get_initial_state()));
    while (!scc_deque.empty())
    {
        const auto& scc = scc_deque.front();
        scc_deque.pop_front();
        if (computed_components.at(scc))
        {
            continue;
        }
        auto can_compute = true;
        for (const auto& succ_scc : scc_digraph.get_targets(scc))
        {
            if (!computed_components.at(succ_scc.get_index()))
            {
                can_compute = false;
                scc_deque.push_back(succ_scc.get_index());
            }
        }
        if (can_compute)
        {
            for (const auto& succ_scc : scc_digraph.get_targets(scc))
            {
                pruning_components.at(scc) &= pruning_components.at(succ_scc.get_index());
            }
            computed_components[scc] = true;
        }
        else
        {
            scc_deque.push_back(scc);
        }
    }

    return ObjectGraphStaticSccPruningStrategy(std::move(pruning_components), std::move(component_map));
}

}
