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

#include "mimir/common/concepts.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/utils.hpp"
#include "mimir/graphs/object_graph.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/graphs/static_graph_boost_adapter.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators/grounded.hpp"

#include <optional>
#include <stack>

namespace mimir
{

/* ObjectGraphStaticPruningStrategy */

ObjectGraphStaticSccPruningStrategy::ObjectGraphStaticSccPruningStrategy(size_t num_components,
                                                                         std::vector<SccPruningComponent> pruning_components,
                                                                         std::vector<size_t> component_map) :
    m_num_components(num_components),
    m_pruning_components(std::move(pruning_components)),
    m_component_map(std::move(component_map))
{
}

bool ObjectGraphStaticSccPruningStrategy::prune(Index state, Object object) const
{
    const auto& pruned_objects = m_pruning_components.at(m_component_map.at(state)).m_pruned_objects;
    return pruned_objects.get(object->get_index());
}

template<PredicateTag P>
static bool prune(const std::vector<ObjectGraphStaticSccPruningStrategy::SccPruningComponent>& pruning_components,
                  const std::vector<size_t>& component_map,
                  Index state,
                  GroundAtom<P> atom)
{
    // Prune atom if at least one object was pruned.
    const auto& pruned_objects = pruning_components.at(component_map.at(state)).m_pruned_objects;
    for (const auto& object : atom->get_objects())
    {
        if (pruned_objects.get(object->get_index()))
        {
            return true;
        }
    }
    return false;
}

bool ObjectGraphStaticSccPruningStrategy::prune(Index state, GroundAtom<Static> atom) const
{
    return mimir::prune(m_pruning_components, m_component_map, state, atom);
}

bool ObjectGraphStaticSccPruningStrategy::prune(Index state, GroundAtom<Fluent> atom) const
{
    return mimir::prune(m_pruning_components, m_component_map, state, atom);
}

bool ObjectGraphStaticSccPruningStrategy::prune(Index state, GroundAtom<Derived> atom) const
{
    return mimir::prune(m_pruning_components, m_component_map, state, atom);
}

template<PredicateTag P>
static bool prune(const std::vector<ObjectGraphStaticSccPruningStrategy::SccPruningComponent>& pruning_components,
                  const std::vector<size_t>& component_map,
                  Index state,
                  GroundLiteral<P> literal)
{
    return pruning_components.at(component_map.at(state)).get_pruned_goal_literals<P>().get(literal->get_index());
}

bool ObjectGraphStaticSccPruningStrategy::prune(Index state, GroundLiteral<Static> literal) const
{
    return mimir::prune(m_pruning_components, m_component_map, state, literal);
}

bool ObjectGraphStaticSccPruningStrategy::prune(Index state, GroundLiteral<Fluent> literal) const
{
    return mimir::prune(m_pruning_components, m_component_map, state, literal);
}

bool ObjectGraphStaticSccPruningStrategy::prune(Index state, GroundLiteral<Derived> literal) const
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

template<PredicateTag P>
const FlatBitset& ObjectGraphStaticSccPruningStrategy::SccPruningComponent::get_pruned_goal_literals() const
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
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatBitset& ObjectGraphStaticSccPruningStrategy::SccPruningComponent::get_pruned_goal_literals<Static>() const;
template const FlatBitset& ObjectGraphStaticSccPruningStrategy::SccPruningComponent::get_pruned_goal_literals<Fluent>() const;
template const FlatBitset& ObjectGraphStaticSccPruningStrategy::SccPruningComponent::get_pruned_goal_literals<Derived>() const;

static StaticForwardGraph<StaticDigraph> create_scc_digraph(size_t num_components, const std::vector<size_t>& component_map, const StateSpace& state_space)
{
    auto g = StaticDigraph();
    for (size_t i = 0; i < num_components; ++i)
    {
        g.add_vertex();
    }
    using StatePair = std::pair<size_t, size_t>;
    std::unordered_set<StatePair, Hash<StatePair>> edges;
    for (const auto& t : state_space.get_graph().get_edges())
    {
        const auto source = t.get_source();
        const auto target = t.get_target();
        if (component_map.at(source) != component_map.at(target) && !edges.contains({ source, target }))
        {
            g.add_directed_edge(component_map.at(source), component_map.at(target));
        }
    }
    return StaticForwardGraph<StaticDigraph>(std::move(g));
}

template<PredicateTag P>
void mark_objects_as_not_prunable(const GroundLiteralList<P>& goal_condition,
                                  const FlatBitset& always_true_state_atoms,
                                  const FlatBitset& always_false_state_atoms,
                                  FlatBitset& ref_pruned_goal_literals,
                                  FlatBitset& ref_pruned_objects)
{
    for (const auto& literal : goal_condition)
    {
        if ((!literal->is_negated() && !always_true_state_atoms.get(literal->get_atom()->get_index()))
            || (literal->is_negated() && !always_false_state_atoms.get(literal->get_atom()->get_index())))
        {
            // literal not always satisfied or unsatisfied
            for (const auto& object : literal->get_atom()->get_objects())
            {
                ref_pruned_objects.unset(object->get_index());
            }
        }
        if (!literal->is_negated() && always_true_state_atoms.get(literal->get_atom()->get_index()))
        {
            // literal always satisfied
            ref_pruned_goal_literals.set(literal->get_index());
        }
    }
}

template<PredicateTag P>
void mark_objects_as_not_prunable(const GroundAtomList<P>& atoms, FlatBitset& ref_pruned_objects)
{
    for (const auto& atom : atoms)
    {
        for (const auto& object : atom->get_objects())
        {
            ref_pruned_objects.unset(object->get_index());
        }
    }
}

std::optional<ObjectGraphStaticSccPruningStrategy>
ObjectGraphStaticSccPruningStrategy::create(Problem problem,
                                            std::shared_ptr<PDDLRepositories> factories,
                                            std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                                            std::shared_ptr<StateRepository> state_repository,
                                            const StateSpaceOptions& options)
{
    // TODO: check assumptions
    /* Check assumptions:
         1. No negative conditions
         2. No unsatisfiable goal, e.g., x and not x
    */

    auto state_space = StateSpace::create(problem, factories, applicable_action_generator, state_repository, options);
    if (!state_space.has_value())
    {
        return std::nullopt;
    }

    auto graph = TraversalDirectionTaggedType(state_space.value().get_graph(), ForwardTraversal());
    const auto [num_components, component_map] = strong_components(graph);
    const auto scc_digraph = create_scc_digraph(num_components, component_map, state_space.value());
    const auto partitioning =
        get_partitioning<StaticBidirectionalGraph<StaticGraph<StateVertex, GroundActionEdge>>, ForwardTraversal>(num_components, component_map);
    auto pruning_components = std::vector<ObjectGraphStaticSccPruningStrategy::SccPruningComponent>();
    pruning_components.reserve(partitioning.size());

    /* 1. Compute atoms that are always true or false in the SCC
       Use default bit value 1 for always false part.
    */
    auto always_true_static_atoms = FlatBitset(state_space->get_problem()->get_static_initial_positive_atoms_bitset());
    auto always_true_fluent_atoms = FlatBitset();
    auto always_true_derived_atoms = FlatBitset();
    auto always_false_static_atoms = FlatBitset(0, 1);
    auto always_false_fluent_atoms = FlatBitset(0, 1);
    auto always_false_derived_atoms = FlatBitset(0, 1);
    always_false_static_atoms -= always_true_static_atoms;

    auto pruned_static_goal_literals = FlatBitset();
    auto pruned_fluent_goal_literals = FlatBitset();
    auto pruned_derived_goal_literals = FlatBitset();

    for (size_t group_index = 0; group_index < partitioning.size(); ++group_index)
    {
        auto group = partitioning.at(group_index);

        // Reuse memory.
        always_true_fluent_atoms = get_state(state_space->get_graph().get_vertices().at(group.front().second))->get_atoms<Fluent>();
        always_true_derived_atoms = get_state(state_space->get_graph().get_vertices().at(group.front().second))->get_atoms<Derived>();
        always_false_fluent_atoms.unset_all();
        always_false_derived_atoms.unset_all();

        pruned_static_goal_literals.unset_all();
        pruned_fluent_goal_literals.unset_all();
        pruned_derived_goal_literals.unset_all();

        for (const auto& [group_index, state_index] : group)
        {
            const auto& state = get_state(state_space->get_graph().get_vertices().at(state_index));
            always_true_fluent_atoms &= state->get_atoms<Fluent>();
            always_true_derived_atoms &= state->get_atoms<Derived>();
            always_false_fluent_atoms -= state->get_atoms<Fluent>();
            always_false_derived_atoms -= state->get_atoms<Derived>();
        }

        /* 2. Initialize prunable objects to all objects.
         */
        auto pruned_objects = FlatBitset();
        for (const auto& object : state_space->get_problem()->get_objects())
        {
            pruned_objects.set(object->get_index());
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
            for (const auto& transition : state_space->get_graph().template get_adjacent_edges<ForwardTraversal>(state_index))
            {
                const auto& precondition = get_creating_action(transition)->get_strips_precondition();
                mark_objects_as_not_prunable(
                    state_space->get_pddl_repositories()->get_ground_atoms_from_indices<Static>(precondition.get_negative_precondition<Static>()),
                    pruned_objects);
                mark_objects_as_not_prunable(
                    state_space->get_pddl_repositories()->get_ground_atoms_from_indices<Static>(precondition.get_positive_precondition<Static>()),
                    pruned_objects);
                mark_objects_as_not_prunable(
                    state_space->get_pddl_repositories()->get_ground_atoms_from_indices<Fluent>(precondition.get_negative_precondition<Fluent>()),
                    pruned_objects);
                mark_objects_as_not_prunable(
                    state_space->get_pddl_repositories()->get_ground_atoms_from_indices<Fluent>(precondition.get_positive_precondition<Fluent>()),
                    pruned_objects);
                mark_objects_as_not_prunable(
                    state_space->get_pddl_repositories()->get_ground_atoms_from_indices<Derived>(precondition.get_negative_precondition<Derived>()),
                    pruned_objects);
                mark_objects_as_not_prunable(
                    state_space->get_pddl_repositories()->get_ground_atoms_from_indices<Derived>(precondition.get_positive_precondition<Derived>()),
                    pruned_objects);

                for (const auto& conditional_effect : get_creating_action(transition)->get_conditional_effects())
                {
                    mark_objects_as_not_prunable(
                        state_space->get_pddl_repositories()->get_ground_atoms_from_indices<Static>(conditional_effect.get_negative_precondition<Static>()),
                        pruned_objects);
                    mark_objects_as_not_prunable(
                        state_space->get_pddl_repositories()->get_ground_atoms_from_indices<Static>(conditional_effect.get_positive_precondition<Static>()),
                        pruned_objects);
                    mark_objects_as_not_prunable(
                        state_space->get_pddl_repositories()->get_ground_atoms_from_indices<Fluent>(conditional_effect.get_negative_precondition<Fluent>()),
                        pruned_objects);
                    mark_objects_as_not_prunable(
                        state_space->get_pddl_repositories()->get_ground_atoms_from_indices<Fluent>(conditional_effect.get_positive_precondition<Fluent>()),
                        pruned_objects);
                    mark_objects_as_not_prunable(
                        state_space->get_pddl_repositories()->get_ground_atoms_from_indices<Derived>(conditional_effect.get_negative_precondition<Derived>()),
                        pruned_objects);
                    mark_objects_as_not_prunable(
                        state_space->get_pddl_repositories()->get_ground_atoms_from_indices<Derived>(conditional_effect.get_positive_precondition<Derived>()),
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
    scc_stack.push(component_map.at(state_space->get_initial_vertex_index()));

    while (!scc_stack.empty())
    {
        const auto& scc = scc_stack.top();
        if (visited_components.at(scc))
        {
            scc_stack.pop();
            for (const auto& succ_scc : scc_digraph.template get_adjacent_vertices<ForwardTraversal>(scc))
            {
                pruning_components.at(scc) &= pruning_components.at(succ_scc.get_index());
            }
            continue;
        }
        visited_components.at(scc) = true;

        for (const auto& succ_scc : scc_digraph.template get_adjacent_vertices<ForwardTraversal>(scc))
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
            [[maybe_unused]] const auto& state = get_state(state_space->get_graph().get_vertices().at(state_index));

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

const std::vector<size_t>& ObjectGraphStaticSccPruningStrategy::get_component_map() const { return m_component_map; }

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out,
                         const std::tuple<const ObjectGraphStaticSccPruningStrategy::SccPruningComponent&, Problem, const PDDLRepositories&>& data)
{
    const auto& [pruning_component, problem, factories] = data;

    out << "pruned_objects: " << factories.get_objects_from_indices(pruning_component.m_pruned_objects) << std::endl;

    return out;
}

}
