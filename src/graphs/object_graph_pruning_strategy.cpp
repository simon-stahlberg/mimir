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
#include <stack>

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
    const auto& pruned_objects = m_pruning_components.at(m_component_map.at(state)).m_pruned_objects;
    return pruned_objects.get(object->get_identifier());
}

template<PredicateCategory P>
static bool prune(const std::vector<ObjectGraphStaticSccPruningStrategy::SccPruningComponent>& pruning_components,
                  const std::map<StateIndex, size_t>& component_map,
                  StateIndex state,
                  GroundAtom<P> atom)
{
    // Prune atom if at least one object was pruned.
    const auto& pruned_objects = pruning_components.at(component_map.at(state)).m_pruned_objects;
    for (const auto& object : atom->get_objects())
    {
        if (pruned_objects.get(object->get_identifier()))
        {
            return true;
        }
    }
    return false;
}

bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, GroundAtom<Static> atom) const
{
    return mimir::prune(m_pruning_components, m_component_map, state, atom);
}

bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, GroundAtom<Fluent> atom) const
{
    return mimir::prune(m_pruning_components, m_component_map, state, atom);
}

bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, GroundAtom<Derived> atom) const
{
    return mimir::prune(m_pruning_components, m_component_map, state, atom);
}

template<PredicateCategory P>
static bool prune(const std::vector<ObjectGraphStaticSccPruningStrategy::SccPruningComponent>& pruning_components,
                  const std::map<StateIndex, size_t>& component_map,
                  StateIndex state,
                  GroundLiteral<P> literal)
{
    return pruning_components.at(component_map.at(state)).get_pruned_goal_literals<P>().get(literal->get_identifier());
}

bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, GroundLiteral<Static> literal) const
{
    return mimir::prune(m_pruning_components, m_component_map, state, literal);
}

bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, GroundLiteral<Fluent> literal) const
{
    return mimir::prune(m_pruning_components, m_component_map, state, literal);
}

bool ObjectGraphStaticSccPruningStrategy::prune(StateIndex state, GroundLiteral<Derived> literal) const
{
    return mimir::prune(m_pruning_components, m_component_map, state, literal);
}

ObjectGraphStaticSccPruningStrategy::SccPruningComponent&
ObjectGraphStaticSccPruningStrategy::SccPruningComponent::operator&=(const ObjectGraphStaticSccPruningStrategy::SccPruningComponent& other)
{
    m_pruned_objects &= other.m_pruned_objects;
    m_pruned_static_goal_literal &= other.m_pruned_static_goal_literal;
    m_pruned_fluent_goal_literal &= other.m_pruned_fluent_goal_literal;
    m_pruned_derived_goal_literal &= other.m_pruned_derived_goal_literal;
    return *this;
}

template<PredicateCategory P>
const FlatBitsetBuilder<P>& ObjectGraphStaticSccPruningStrategy::SccPruningComponent::get_pruned_goal_literals() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_pruned_static_goal_literal;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_pruned_fluent_goal_literal;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_pruned_derived_goal_literal;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template const FlatBitsetBuilder<Static>& ObjectGraphStaticSccPruningStrategy::SccPruningComponent::get_pruned_goal_literals<Static>() const;
template const FlatBitsetBuilder<Fluent>& ObjectGraphStaticSccPruningStrategy::SccPruningComponent::get_pruned_goal_literals<Fluent>() const;
template const FlatBitsetBuilder<Derived>& ObjectGraphStaticSccPruningStrategy::SccPruningComponent::get_pruned_goal_literals<Derived>() const;

static ForwardGraph<Digraph> create_scc_digraph(size_t num_components, const std::map<StateIndex, size_t>& component_map, const StateSpace& state_space)
{
    auto g = Digraph();
    for (size_t i = 0; i < num_components; ++i)
    {
        g.add_vertex();
    }
    using StatePair = std::pair<size_t, size_t>;
    const auto state_pair_hash = [](const auto& pair) { return loki::hash_combine(pair.first, pair.second); };
    std::unordered_set<StatePair, decltype(state_pair_hash)> edges;
    for (const auto t : state_space.get_transitions())
    {
        const auto source = t.get_source_state();
        const auto target = t.get_target_state();
        if (component_map.at(source) != component_map.at(target) && !edges.contains({ source, target }))
        {
            g.add_directed_edge(component_map.at(source), component_map.at(target));
        }
    }
    return ForwardGraph<Digraph>(std::move(g));
}

template<PredicateCategory P>
void mark_objects_as_not_prunable(const GroundLiteralList<P>& goal_condition,
                                  const FlatBitsetBuilder<P>& always_true_state_atoms,
                                  const FlatBitsetBuilder<P>& always_false_state_atoms,
                                  FlatBitsetBuilder<P>& ref_pruned_goal_literals,
                                  FlatBitsetBuilder<>& ref_pruned_objects)
{
    for (const auto& literal : goal_condition)
    {
        if ((!literal->is_negated() && !always_true_state_atoms.get(literal->get_atom()->get_identifier()))
            || (literal->is_negated() && !always_false_state_atoms.get(literal->get_atom()->get_identifier())))
        {
            // literal not always satisfied or unsatisfied
            for (const auto& object : literal->get_atom()->get_objects())
            {
                ref_pruned_objects.unset(object->get_identifier());
            }
        }
        if (!literal->is_negated() && always_true_state_atoms.get(literal->get_atom()->get_identifier()))
        {
            // literal always satisfied
            ref_pruned_goal_literals.set(literal->get_identifier());
        }
    }
}

template<PredicateCategory P>
void mark_objects_as_not_prunable(const GroundAtomList<P>& atoms, FlatBitsetBuilder<>& ref_pruned_objects)
{
    for (const auto& atom : atoms)
    {
        for (const auto& object : atom->get_objects())
        {
            ref_pruned_objects.unset(object->get_identifier());
        }
    }
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
    if (!state_space.has_value())
    {
        return std::nullopt;
    }

    const auto [num_components, component_map] = strong_components(state_space.value());
    const auto scc_digraph = create_scc_digraph(num_components, component_map, state_space.value());
    const auto partitioning = get_partitioning<StateSpace>(num_components, component_map);
    auto pruning_components = std::vector<ObjectGraphStaticSccPruningStrategy::SccPruningComponent>();
    pruning_components.reserve(partitioning.size());

    /* 1. Compute atoms that are always true or false in the SCC
       Use default bit value 1 for always false part.
    */
    auto always_true_static_atoms = FlatBitsetBuilder<Static>(state_space->get_problem()->get_static_initial_positive_atoms_bitset());
    auto always_true_fluent_atoms = FlatBitsetBuilder<Fluent>();
    auto always_true_derived_atoms = FlatBitsetBuilder<Derived>();
    auto always_false_static_atoms = FlatBitsetBuilder<Static>(0, 1);
    auto always_false_fluent_atoms = FlatBitsetBuilder<Fluent>(0, 1);
    auto always_false_derived_atoms = FlatBitsetBuilder<Derived>(0, 1);
    always_false_static_atoms -= always_true_static_atoms;

    auto pruned_static_goal_literals = FlatBitsetBuilder<Static>();
    auto pruned_fluent_goal_literals = FlatBitsetBuilder<Fluent>();
    auto pruned_derived_goal_literals = FlatBitsetBuilder<Derived>();

    for (size_t group_index = 0; group_index < partitioning.size(); ++group_index)
    {
        auto group = partitioning.at(group_index);

        // Reuse memory.
        always_true_fluent_atoms = state_space->get_states().at(group.front().second).get_atoms<Fluent>();
        always_true_derived_atoms = state_space->get_states().at(group.front().second).get_atoms<Derived>();
        always_false_fluent_atoms.unset_all();
        always_false_derived_atoms.unset_all();

        pruned_static_goal_literals.unset_all();
        pruned_fluent_goal_literals.unset_all();
        pruned_derived_goal_literals.unset_all();

        for (const auto& [group_index, state_index] : group)
        {
            const auto& state = state_space->get_states().at(state_index);
            always_true_fluent_atoms &= state.get_atoms<Fluent>();
            always_true_derived_atoms &= state.get_atoms<Derived>();
            always_false_fluent_atoms -= state.get_atoms<Fluent>();
            always_false_derived_atoms -= state.get_atoms<Derived>();
        }

        /* 2. Initialize prunable objects to all objects.
         */
        auto pruned_objects = FlatBitsetBuilder<>();
        for (const auto& object : state_space->get_problem()->get_objects())
        {
            pruned_objects.set(object->get_identifier());
        }

        /* 3. Do not prune objects that appear in unsatisfied goal literals.
         */

        mark_objects_as_not_prunable(problem->get_goal_condition<Static>(),
                                     always_true_static_atoms,
                                     always_false_static_atoms,
                                     pruned_static_goal_literals,
                                     pruned_objects);
        mark_objects_as_not_prunable(problem->get_goal_condition<Fluent>(),
                                     always_true_fluent_atoms,
                                     always_false_fluent_atoms,
                                     pruned_fluent_goal_literals,
                                     pruned_objects);
        mark_objects_as_not_prunable(problem->get_goal_condition<Derived>(),
                                     always_true_derived_atoms,
                                     always_false_derived_atoms,
                                     pruned_derived_goal_literals,
                                     pruned_objects);

        /* 4. Do not prune objects that appear in literals of action conditions.
         */

        for (const auto& [group_index, state_index] : group)
        {
            for (const auto& transition : state_space->get_forward_transitions(state_index))
            {
                const auto& precondition = StripsActionPrecondition(transition.get_creating_action().get_strips_precondition());
                mark_objects_as_not_prunable(
                    state_space->get_pddl_factories()->get_ground_atoms_from_ids<Static>(precondition.get_negative_precondition<Static>()),
                    pruned_objects);
                mark_objects_as_not_prunable(
                    state_space->get_pddl_factories()->get_ground_atoms_from_ids<Static>(precondition.get_positive_precondition<Static>()),
                    pruned_objects);
                mark_objects_as_not_prunable(
                    state_space->get_pddl_factories()->get_ground_atoms_from_ids<Fluent>(precondition.get_negative_precondition<Fluent>()),
                    pruned_objects);
                mark_objects_as_not_prunable(
                    state_space->get_pddl_factories()->get_ground_atoms_from_ids<Fluent>(precondition.get_positive_precondition<Fluent>()),
                    pruned_objects);
                mark_objects_as_not_prunable(
                    state_space->get_pddl_factories()->get_ground_atoms_from_ids<Derived>(precondition.get_negative_precondition<Derived>()),
                    pruned_objects);
                mark_objects_as_not_prunable(
                    state_space->get_pddl_factories()->get_ground_atoms_from_ids<Derived>(precondition.get_positive_precondition<Derived>()),
                    pruned_objects);

                for (const auto& flat_conditional_effect : transition.get_creating_action().get_conditional_effects())
                {
                    const auto conditional_effect = ConditionalEffect(flat_conditional_effect);
                    mark_objects_as_not_prunable(
                        state_space->get_pddl_factories()->get_ground_atoms_from_ids<Static>(conditional_effect.get_negative_precondition<Static>()),
                        pruned_objects);
                    mark_objects_as_not_prunable(
                        state_space->get_pddl_factories()->get_ground_atoms_from_ids<Static>(conditional_effect.get_positive_precondition<Static>()),
                        pruned_objects);
                    mark_objects_as_not_prunable(
                        state_space->get_pddl_factories()->get_ground_atoms_from_ids<Fluent>(conditional_effect.get_negative_precondition<Fluent>()),
                        pruned_objects);
                    mark_objects_as_not_prunable(
                        state_space->get_pddl_factories()->get_ground_atoms_from_ids<Fluent>(conditional_effect.get_positive_precondition<Fluent>()),
                        pruned_objects);
                    mark_objects_as_not_prunable(
                        state_space->get_pddl_factories()->get_ground_atoms_from_ids<Derived>(conditional_effect.get_negative_precondition<Derived>()),
                        pruned_objects);
                    mark_objects_as_not_prunable(
                        state_space->get_pddl_factories()->get_ground_atoms_from_ids<Derived>(conditional_effect.get_positive_precondition<Derived>()),
                        pruned_objects);
                }
            }
        }

        pruning_components.push_back(ObjectGraphStaticSccPruningStrategy::SccPruningComponent { pruned_objects,
                                                                                                pruned_static_goal_literals,
                                                                                                pruned_fluent_goal_literals,
                                                                                                pruned_derived_goal_literals });
    }

    /* 5. Propagate info along SCCs using post-order DFS.
     */

    auto visited_components = std::vector<bool>(num_components, false);
    auto scc_stack = std::stack<size_t>();
    scc_stack.push(component_map.at(state_space->get_initial_state()));

    while (!scc_stack.empty())
    {
        const auto& scc = scc_stack.top();
        if (visited_components.at(scc))
        {
            scc_stack.pop();
            for (const auto& succ_scc : scc_digraph.get_targets(scc))
            {
                pruning_components.at(scc) &= pruning_components.at(succ_scc.get_index());
            }
            continue;
        }
        visited_components.at(scc) = true;

        for (const auto& succ_scc : scc_digraph.get_targets(scc))
        {
            if (!visited_components.at(succ_scc.get_index()))
            {
                scc_stack.push(succ_scc.get_index());
            }
        }
    }

    for (size_t group_index = 0; group_index < num_components; ++group_index)
    {
        auto group = partitioning.at(group_index);
        for (const auto& [group_index, state_index] : group)
        {
            const auto& state = state_space->get_states().at(state_index);

            // std::cout << std::make_tuple(problem, state, std::cref(*factories)) << std::endl;
        }

        // std::cout << group_index << " " << std::make_tuple(std::cref(pruning_components.at(group_index)), problem, std::cref(*factories)) << std::endl;
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

    return out;
}

}
