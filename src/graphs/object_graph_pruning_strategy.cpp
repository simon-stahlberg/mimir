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
#include "mimir/graphs/object_graph.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators/grounded.hpp"
#include "mimir/search/flat_types.hpp"
#include "mimir/search/translations.hpp"

#include <optional>

namespace mimir
{

/* ObjectGraphStaticPruningStrategy */

ObjectGraphStaticSccPruningStrategy::ObjectGraphStaticSccPruningStrategy(size_t num_components,
                                                                         std::vector<SccPruningComponent> pruning_components,
                                                                         std::map<StateIndex, size_t> component_map) :
    m_num_components(num_components),
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

std::optional<ObjectGraphStaticSccPruningStrategy> ObjectGraphStaticSccPruningStrategy::create(Problem problem,
                                                                                               std::shared_ptr<PDDLFactories> factories,
                                                                                               std::shared_ptr<IAAG> aag,
                                                                                               std::shared_ptr<SuccessorStateGenerator> ssg,
                                                                                               const StateSpaceOptions& options)
{
    // TODO: check assumptions
    /* Check assumptions:
         1. No negative conditions
         2. No unsatisfiable goal, e.g., x and not x
    */

    auto state_space = StateSpace::create(problem, factories, aag, ssg, options);

    const auto [num_components, component_map] = strong_components(*state_space);
    const auto scc_digraph = create_scc_digraph(num_components, component_map, *state_space);
    const auto partitioning = get_partitioning<StateSpace>(num_components, component_map);
    auto pruning_components = std::vector<ObjectGraphStaticSccPruningStrategy::SccPruningComponent>();
    pruning_components.reserve(partitioning.size());
    for (size_t group_index = 0; group_index < partitioning.size(); ++group_index)
    {
        std::cout << "group_index: " << group_index << std::endl;
        auto group = partitioning.at(group_index);

        /* 0. Compute atoms that are always true/false in the SCC */
        auto always_true_static_atoms = FlatBitsetBuilder<Static>(state_space->get_problem()->get_static_initial_positive_atoms_bitset());
        auto always_true_fluent_atoms = FlatBitsetBuilder<Fluent>(state_space->get_states()[group.begin()->second].get_atoms<Fluent>());
        auto always_true_derived_atoms = FlatBitsetBuilder<Derived>(state_space->get_states()[group.begin()->second].get_atoms<Derived>());
        // Use default bit value 1 for the false part.
        auto always_false_static_atoms = FlatBitsetBuilder<Static>(0, 1);
        auto always_false_fluent_atoms = FlatBitsetBuilder<Fluent>(0, 1);
        auto always_false_derived_atoms = FlatBitsetBuilder<Derived>(0, 1);
        // All atoms in the scc
        auto static_atoms = FlatBitsetBuilder<Static>(state_space->get_problem()->get_static_initial_positive_atoms_bitset());
        auto fluent_atoms = FlatBitsetBuilder<Fluent>();
        auto derived_atoms = FlatBitsetBuilder<Derived>();

        always_false_static_atoms -= always_true_static_atoms;
        for (const auto& [group_index, state_index] : group)
        {
            const auto& state = state_space->get_states().at(state_index);
            std::cout << std::make_tuple(state_space->get_problem(), state, std::cref(*state_space->get_pddl_factories())) << std::endl;
            always_true_fluent_atoms &= state.get_atoms<Fluent>();
            always_true_derived_atoms &= state.get_atoms<Derived>();
            always_false_fluent_atoms -= state.get_atoms<Fluent>();
            always_false_derived_atoms -= state.get_atoms<Derived>();
            fluent_atoms |= state.get_atoms<Fluent>();
            derived_atoms |= state.get_atoms<Derived>();
        }

        std::cout << "always_true_static_atoms: " << always_true_static_atoms.count() << std::endl;
        std::cout << "always_true_fluent_atoms: " << always_true_fluent_atoms.count() << std::endl;

        /* 1. Prune goal literals that are satisfied in all states in the group. */
        auto pruned_static_goal_literals = FlatBitsetBuilder<Static>();
        auto pruned_fluent_goal_literals = FlatBitsetBuilder<Fluent>();
        auto pruned_derived_goal_literals = FlatBitsetBuilder<Derived>();
        // Track underlying prunable goal atoms
        auto pruned_static_goal_atoms = FlatBitsetBuilder<Static>();
        auto pruned_fluent_goal_atoms = FlatBitsetBuilder<Fluent>();
        auto pruned_derived_goal_atoms = FlatBitsetBuilder<Derived>();
        for (const auto& literal : problem->get_goal_condition<Static>())
        {
            if ((!literal->is_negated() && always_true_static_atoms.get(literal->get_atom()->get_identifier()))
                || (literal->is_negated() && always_false_static_atoms.get(literal->get_atom()->get_identifier())))
            {
                // static goal literal is always satisfied => prune it!
                pruned_static_goal_literals.set(literal->get_identifier());
                pruned_static_goal_atoms.set(literal->get_atom()->get_identifier());
            }
        }
        for (const auto& literal : problem->get_goal_condition<Fluent>())
        {
            if ((!literal->is_negated() && always_true_fluent_atoms.get(literal->get_atom()->get_identifier()))
                || (literal->is_negated() && always_false_fluent_atoms.get(literal->get_atom()->get_identifier())))
            {
                // fluent goal literal is always satisfied => prune it!
                pruned_fluent_goal_literals.set(literal->get_identifier());
                pruned_fluent_goal_atoms.set(literal->get_atom()->get_identifier());
            }
        }
        for (const auto& literal : problem->get_goal_condition<Derived>())
        {
            if ((!literal->is_negated() && always_true_derived_atoms.get(literal->get_atom()->get_identifier()))
                || (literal->is_negated() && always_false_derived_atoms.get(literal->get_atom()->get_identifier())))
            {
                // derived goal literal is always satisfied => prune it!
                pruned_derived_goal_literals.set(literal->get_identifier());
                pruned_derived_goal_atoms.set(literal->get_atom()->get_identifier());
            }
        }

        std::cout << "pruned_fluent_goal_literals: " << pruned_fluent_goal_literals.count() << std::endl;
        std::cout << "pruned_derived_goal_atoms: " << pruned_derived_goal_atoms.count() << std::endl;

        /* 2. Prune state atoms that never occur within the group in
           1) a action precondition,
           2) a conditional effect precondition, or
           3) a unsatisfied goal atom.
        */
        auto pruned_static_ground_atoms = static_atoms;
        auto pruned_fluent_ground_atoms = fluent_atoms;
        auto pruned_derived_ground_atoms = derived_atoms;

        for (const auto& [group_index, state_index] : group)
        {
            for (const auto& transition : state_space->get_forward_transitions(state_index))
            {
                const auto& precondition = StripsActionPrecondition(transition.get_creating_action().get_strips_precondition());
                pruned_static_ground_atoms -= precondition.get_negative_precondition<Static>();
                pruned_static_ground_atoms -= precondition.get_positive_precondition<Static>();
                pruned_fluent_ground_atoms -= precondition.get_negative_precondition<Fluent>();
                pruned_fluent_ground_atoms -= precondition.get_positive_precondition<Fluent>();
                pruned_derived_ground_atoms -= precondition.get_negative_precondition<Derived>();
                pruned_derived_ground_atoms -= precondition.get_positive_precondition<Derived>();
                for (const auto& flat_conditional_effect : transition.get_creating_action().get_conditional_effects())
                {
                    const auto conditional_effect = ConditionalEffect(flat_conditional_effect);
                    for (const auto& atom_id : conditional_effect.get_negative_precondition<Static>())
                    {
                        pruned_static_ground_atoms.unset(atom_id);
                    }
                    for (const auto& atom_id : conditional_effect.get_positive_precondition<Static>())
                    {
                        pruned_static_ground_atoms.unset(atom_id);
                    }
                    for (const auto& atom_id : conditional_effect.get_negative_precondition<Fluent>())
                    {
                        pruned_fluent_ground_atoms.unset(atom_id);
                    }
                    for (const auto& atom_id : conditional_effect.get_positive_precondition<Fluent>())
                    {
                        pruned_fluent_ground_atoms.unset(atom_id);
                    }
                    for (const auto& atom_id : conditional_effect.get_negative_precondition<Derived>())
                    {
                        pruned_derived_ground_atoms.unset(atom_id);
                    }
                    for (const auto& atom_id : conditional_effect.get_positive_precondition<Derived>())
                    {
                        pruned_derived_ground_atoms.unset(atom_id);
                    }
                }
            }
        }

        /* 3. Prune objects that remain in no more state atoms or goal literals.
         */
        auto pruned_objects = FlatBitsetBuilder<>();
        for (const auto& object : state_space->get_problem()->get_objects())
        {
            pruned_objects.set(object->get_identifier());
        }

        // atoms with unsatisfied goal atom cannot be pruned
        for (const auto& literal : problem->get_goal_condition<Static>())
        {
            if (!pruned_static_goal_atoms.get(literal->get_atom()->get_identifier()))
            {
                pruned_static_ground_atoms.unset(literal->get_atom()->get_identifier());
                for (const auto& object : literal->get_atom()->get_objects())
                {
                    pruned_objects.unset(object->get_identifier());
                }
            }
        }
        for (const auto& literal : problem->get_goal_condition<Fluent>())
        {
            if (!pruned_fluent_goal_atoms.get(literal->get_atom()->get_identifier()))
            {
                pruned_fluent_ground_atoms.unset(literal->get_atom()->get_identifier());
                for (const auto& object : literal->get_atom()->get_objects())
                {
                    pruned_objects.unset(object->get_identifier());
                }
            }
        }
        for (const auto& literal : problem->get_goal_condition<Derived>())
        {
            if (!pruned_derived_goal_atoms.get(literal->get_atom()->get_identifier()))
            {
                pruned_derived_ground_atoms.unset(literal->get_atom()->get_identifier());
                for (const auto& object : literal->get_atom()->get_objects())
                {
                    pruned_objects.unset(object->get_identifier());
                }
            }
        }

        std::cout << "pruned_static_ground_atoms: " << pruned_static_ground_atoms.count() << std::endl;
        std::cout << "pruned_fluent_ground_atoms: " << pruned_fluent_ground_atoms.count() << std::endl;

        auto unpruned_static_ground_atoms = static_atoms;
        unpruned_static_ground_atoms -= pruned_static_ground_atoms;
        auto unpruned_fluent_ground_atoms = fluent_atoms;
        unpruned_fluent_ground_atoms -= pruned_fluent_ground_atoms;
        auto unpruned_derived_ground_atoms = derived_atoms;
        unpruned_derived_ground_atoms -= pruned_derived_ground_atoms;
        for (const auto& atom : factories->get_ground_atoms_from_ids<Static>(unpruned_static_ground_atoms))
        {
            std::cout << "Unpruned static: " << *atom << std::endl;
            for (const auto& object : atom->get_objects())
            {
                pruned_objects.unset(object->get_identifier());
            }
        }
        for (const auto& atom : factories->get_ground_atoms_from_ids<Fluent>(unpruned_fluent_ground_atoms))
        {
            std::cout << "Unpruned fluent: " << *atom << std::endl;
            for (const auto& object : atom->get_objects())
            {
                pruned_objects.unset(object->get_identifier());
            }
        }
        for (const auto& atom : factories->get_ground_atoms_from_ids<Derived>(unpruned_derived_ground_atoms))
        {
            for (const auto& object : atom->get_objects())
            {
                pruned_objects.unset(object->get_identifier());
            }
        }
        std::cout << "pruned_objects: " << pruned_objects.count() << std::endl;

        pruning_components.push_back(ObjectGraphStaticSccPruningStrategy::SccPruningComponent { pruned_objects,
                                                                                                pruned_static_ground_atoms,
                                                                                                pruned_fluent_ground_atoms,
                                                                                                pruned_derived_ground_atoms,
                                                                                                pruned_static_goal_literals,
                                                                                                pruned_fluent_goal_literals,
                                                                                                pruned_derived_goal_literals });
    }
    auto computed_components = std::vector<bool>(num_components, false);
    auto scc_deque = std::deque<size_t>();
    scc_deque.push_back(component_map.at(state_space->get_initial_state()));
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

    for (size_t group_index = 0; group_index < partitioning.size(); ++group_index)
    {
        std::cout << "group_index: " << group_index << std::endl;
        auto group = partitioning.at(group_index);
        for (const auto& [group_index, state_index] : group)
        {
            const auto& state = state_space->get_states().at(state_index);
            std::cout << std::make_tuple(state_space->get_problem(), state, std::cref(*state_space->get_pddl_factories())) << std::endl;
        }
        const auto& pruning_component = pruning_components.at(group_index);
        std::cout << "pruned_objects: " << pruning_component.m_pruned_objects.count() << std::endl;
        std::cout << std::make_tuple(std::cref(pruning_component), problem, std::cref(*factories)) << std::endl;
    }

    return ObjectGraphStaticSccPruningStrategy(num_components, std::move(pruning_components), std::move(component_map));
}

size_t ObjectGraphStaticSccPruningStrategy::get_num_components() const { return m_num_components; }

const std::vector<ObjectGraphStaticSccPruningStrategy::SccPruningComponent>& ObjectGraphStaticSccPruningStrategy::get_pruning_components() const
{
    return m_pruning_components;
}

const std::map<StateIndex, size_t>& ObjectGraphStaticSccPruningStrategy::get_component_map() const { return m_component_map; }

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out,
                         const std::tuple<const ObjectGraphStaticSccPruningStrategy::SccPruningComponent&, Problem, const PDDLFactories&>& data)
{
    const auto& [pruning_component, problem, factories] = data;

    out << "pruned_objects: " << factories.get_objects_from_ids(pruning_component.m_pruned_objects) << std::endl;
    out << "pruned_static_atoms: " << factories.get_ground_atoms_from_ids<Static>(pruning_component.m_pruned_static_ground_atoms) << std::endl;
    out << "pruned_fluent_atoms: " << factories.get_ground_atoms_from_ids<Fluent>(pruning_component.m_pruned_fluent_ground_atoms) << std::endl;
    out << "pruned_derived_atoms: " << factories.get_ground_atoms_from_ids<Derived>(pruning_component.m_pruned_derived_ground_atoms) << std::endl;

    return out;
}

}
