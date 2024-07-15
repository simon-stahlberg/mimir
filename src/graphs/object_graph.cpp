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

#include "mimir/graphs/object_graph.hpp"

#include "mimir/formalism/factories.hpp"

namespace mimir
{

/**
 * ObjectGraph
 */

static std::unordered_map<Object, VertexIndex> add_objects_graph_structures(const ProblemColorFunction& color_function,
                                                                            Problem problem,
                                                                            StateIndex state_index,
                                                                            const ObjectGraphPruningStrategy& pruning_strategy,
                                                                            VertexColoredDigraph& out_digraph)
{
    std::unordered_map<Object, VertexIndex> object_to_vertex_index;

    for (const auto& object : problem->get_objects())
    {
        if (!pruning_strategy.prune(state_index, object))
        {
            const auto vertex_color = color_function.get_color(object);
            const auto vertex_index = out_digraph.add_vertex(vertex_color);
            object_to_vertex_index.emplace(object, vertex_index);
        }
    }

    return object_to_vertex_index;
}

template<PredicateCategory P>
static void add_ground_atom_graph_structures(const ProblemColorFunction& color_function,
                                             const std::unordered_map<Object, VertexIndex>& object_to_vertex_index,
                                             GroundAtom<P> atom,
                                             VertexColoredDigraph& out_digraph)
{
    for (size_t pos = 0; pos < atom->get_arity(); ++pos)
    {
        const auto vertex_color = color_function.get_color(atom, pos);
        const auto vertex_index = out_digraph.add_vertex(vertex_color);
        out_digraph.add_undirected_edge(vertex_index, object_to_vertex_index.at(atom->get_objects().at(pos)));
        if (pos > 0)
        {
            out_digraph.add_undirected_edge(vertex_index - 1, vertex_index);
        }
    }
}

static void add_ground_atoms_graph_structures(const ProblemColorFunction& color_function,
                                              const PDDLFactories& pddl_factories,
                                              Problem problem,
                                              State state,
                                              StateIndex state_index,
                                              const ObjectGraphPruningStrategy& pruning_strategy,
                                              const std::unordered_map<Object, VertexIndex>& object_to_vertex_index,
                                              VertexColoredDigraph& out_digraph)
{
    for (const auto& atom : pddl_factories.get_ground_atoms_from_ids<Static>(problem->get_static_initial_positive_atoms_bitset()))
    {
        if (!pruning_strategy.prune(state_index, atom))
        {
            add_ground_atom_graph_structures(color_function, object_to_vertex_index, atom, out_digraph);
        }
    }
    for (const auto& atom : pddl_factories.get_ground_atoms_from_ids<Fluent>(state.get_atoms<Fluent>()))
    {
        if (!pruning_strategy.prune(state_index, atom))
        {
            add_ground_atom_graph_structures(color_function, object_to_vertex_index, atom, out_digraph);
        }
    }
    for (const auto& atom : pddl_factories.get_ground_atoms_from_ids<Derived>(state.get_atoms<Derived>()))
    {
        if (!pruning_strategy.prune(state_index, atom))
        {
            add_ground_atom_graph_structures(color_function, object_to_vertex_index, atom, out_digraph);
        }
    }
}

template<PredicateCategory P>
static void add_ground_literal_graph_structures(const ProblemColorFunction& color_function,
                                                const std::unordered_map<Object, VertexIndex>& object_to_vertex_index,
                                                bool mark_true_goal_literals,
                                                State state,
                                                GroundLiteral<P> literal,
                                                VertexColoredDigraph& out_digraph)
{
    for (size_t pos = 0; pos < literal->get_atom()->get_arity(); ++pos)
    {
        const auto vertex_color = color_function.get_color(state, literal, pos, mark_true_goal_literals);
        const auto vertex_index = out_digraph.add_vertex(vertex_color);
        out_digraph.add_undirected_edge(vertex_index, object_to_vertex_index.at(literal->get_atom()->get_objects().at(pos)));
        if (pos > 0)
        {
            out_digraph.add_undirected_edge(vertex_index - 1, vertex_index);
        }
    }
}

static void add_ground_goal_literals_graph_structures(const ProblemColorFunction& color_function,
                                                      const PDDLFactories& pddl_factories,
                                                      bool mark_true_goal_literals,
                                                      Problem problem,
                                                      State state,
                                                      StateIndex state_index,
                                                      const ObjectGraphPruningStrategy& pruning_strategy,
                                                      const std::unordered_map<Object, VertexIndex>& object_to_vertex_index,
                                                      VertexColoredDigraph& out_digraph)
{
    for (const auto& literal : problem->get_goal_condition<Static>())
    {
        if (!pruning_strategy.prune(state_index, literal))
        {
            add_ground_literal_graph_structures(color_function, object_to_vertex_index, mark_true_goal_literals, state, literal, out_digraph);
        }
    }
    for (const auto& literal : problem->get_goal_condition<Fluent>())
    {
        if (!pruning_strategy.prune(state_index, literal))
        {
            add_ground_literal_graph_structures(color_function, object_to_vertex_index, mark_true_goal_literals, state, literal, out_digraph);
        }
    }
    for (const auto& literal : problem->get_goal_condition<Derived>())
    {
        if (!pruning_strategy.prune(state_index, literal))
        {
            add_ground_literal_graph_structures(color_function, object_to_vertex_index, mark_true_goal_literals, state, literal, out_digraph);
        }
    }
}

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
    m_pruning_components.at(m_component_map.at(state)).m_pruned_static_ground_atoms.get(atom->get_identifier());
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

VertexColoredDigraph create_object_graph(const ProblemColorFunction& color_function,
                                         const PDDLFactories& pddl_factories,
                                         Problem problem,
                                         State state,
                                         StateIndex state_index,
                                         bool mark_true_goal_literals,
                                         const ObjectGraphPruningStrategy& pruning_strategy)
{
    auto vertex_colored_digraph = VertexColoredDigraph();

    const auto object_to_vertex_index = add_objects_graph_structures(color_function, problem, state_index, pruning_strategy, vertex_colored_digraph);

    add_ground_atoms_graph_structures(color_function,
                                      pddl_factories,
                                      problem,
                                      state,
                                      state_index,
                                      pruning_strategy,
                                      object_to_vertex_index,
                                      vertex_colored_digraph);

    add_ground_goal_literals_graph_structures(color_function,
                                              pddl_factories,
                                              mark_true_goal_literals,
                                              problem,
                                              state,
                                              state_index,
                                              pruning_strategy,
                                              object_to_vertex_index,
                                              vertex_colored_digraph);

    return vertex_colored_digraph;
}
}
