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

#include "mimir/common/printers.hpp"

namespace mimir
{
/**
 * ProblemColorFunction
 */

ProblemColorFunction::ProblemColorFunction(Problem problem) : m_problem(problem), m_name_to_color(), m_color_to_name() { initialize_predicates(); }

void ProblemColorFunction::initialize_predicates()
{
    std::vector<std::pair<std::string, int>> lexicographically_sorted_predicates;

    auto add_predicates = [&](const auto& predicates)
    {
        for (const auto& predicate : predicates)
        {
            lexicographically_sorted_predicates.push_back(std::make_pair(predicate->get_name(), predicate->get_arity()));
        }
    };

    add_predicates(m_problem->get_domain()->get_predicates<Static>());
    add_predicates(m_problem->get_domain()->get_predicates<Fluent>());
    add_predicates(m_problem->get_domain()->get_predicates<Derived>());

    // Sort the vector lexicographically by the string in the pair
    std::sort(lexicographically_sorted_predicates.begin(),
              lexicographically_sorted_predicates.end(),
              [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) { return a.first < b.first; });

    // Object color
    const auto color = m_color_to_name.size();
    const auto name = "";
    m_color_to_name.emplace(color, name);
    m_name_to_color.emplace(name, color);

    // Predicate colors
    for (size_t predicate_index = 0; predicate_index < lexicographically_sorted_predicates.size(); ++predicate_index)
    {
        const auto& [predicate_name, predicate_arity] = lexicographically_sorted_predicates.at(predicate_index);

        for (int pos = 0; pos < predicate_arity; ++pos)
        {
            // atom
            auto color = m_color_to_name.size();
            auto name = predicate_name + ":" + std::to_string(pos);
            m_color_to_name.emplace(color, name);
            m_name_to_color.emplace(name, color);
            // goal atom
            color = m_color_to_name.size();
            name = predicate_name + ":g:" + std::to_string(pos);
            m_color_to_name.emplace(color, name);
            m_name_to_color.emplace(name, color);
            // marked goal atoms
            color = m_color_to_name.size();
            name = predicate_name + ":g:true:" + std::to_string(pos);
            m_color_to_name.emplace(color, name);
            m_name_to_color.emplace(name, color);
            color = m_color_to_name.size();
            name = predicate_name + ":g:false:" + std::to_string(pos);
            m_color_to_name.emplace(color, name);
            m_name_to_color.emplace(name, color);
        }
    }
}

Color ProblemColorFunction::get_color(Object object) const { return m_name_to_color.at(""); }

Color ProblemColorFunction::get_color(State state, GroundLiteral<Static> literal, size_t pos, bool mark_true_goal_literal) const
{
    bool is_satisfied_in_goal = m_problem->static_literal_holds(literal);
    return m_name_to_color.at(literal->get_atom()->get_predicate()->get_name() + ":g"
                              + (mark_true_goal_literal ? (is_satisfied_in_goal ? ":true" : ":false") : "") + ":" + std::to_string(pos));
}

const std::string& ProblemColorFunction::get_color_name(Color color) const { return m_color_to_name.at(color); }

Problem ProblemColorFunction::get_problem() const { return m_problem; }

const std::unordered_map<std::string, Color>& ProblemColorFunction::get_name_to_color() const { return m_name_to_color; }

const std::unordered_map<Color, std::string>& ProblemColorFunction::get_color_to_name() const { return m_color_to_name; }

/**
 * ObjectGraph
 */

ObjectGraph::ObjectGraph(std::shared_ptr<const ProblemColorFunction> coloring_function) : m_coloring_function(std::move(coloring_function)), m_digraph() {}

const std::shared_ptr<const ProblemColorFunction>& ObjectGraph::get_coloring_function() const { return m_coloring_function; }

const VertexColoredDigraph& ObjectGraph::get_vertex_colored_digraph() const { return m_digraph; }

/**
 * ObjectGraphFactory
 */

ObjectGraphFactory::ObjectGraphFactory(Problem problem, std::shared_ptr<PDDLFactories> pddl_factories, bool mark_true_goal_literals) :
    m_problem(problem),
    m_pddl_factories(std::move(pddl_factories)),
    m_mark_true_goal_literals(mark_true_goal_literals),
    m_coloring_function(std::make_shared<const ProblemColorFunction>(m_problem)),
    m_object_graph(m_coloring_function)
{
}

int ObjectGraphFactory::add_object_graph_structures(Object object, int num_vertices)
{
    const auto vertex_color = m_coloring_function->get_color(object);
    m_object_graph.m_digraph.add_vertex(vertex_color);
    m_object_to_vertex_index.emplace(object, num_vertices);
    return ++num_vertices;
}

const ObjectGraph& ObjectGraphFactory::create(State state, const ObjectGraphPruningStrategy& pruning_strategy)
{
    // Reset data structures
    m_object_graph.m_digraph.reset();
    m_object_to_vertex_index.clear();

    // Initialize object vertices
    auto vertex_index = 0;
    for (const auto& object : m_problem->get_objects())
    {
        if (!pruning_strategy.prune(object))
        {
            vertex_index = add_object_graph_structures(object, vertex_index);
        }
    }

    // Initialize atom vertices and edges
    for (const auto& atom : m_pddl_factories->get_ground_atoms_from_ids<Static>(m_problem->get_static_initial_positive_atoms_bitset()))
    {
        if (!pruning_strategy.prune(atom))
        {
            vertex_index = add_ground_atom_graph_structures(atom, vertex_index);
        }
    }
    for (const auto& atom : m_pddl_factories->get_ground_atoms_from_ids<Fluent>(state.get_atoms<Fluent>()))
    {
        if (!pruning_strategy.prune(atom))
        {
            vertex_index = add_ground_atom_graph_structures(atom, vertex_index);
        }
    }
    for (const auto& atom : m_pddl_factories->get_ground_atoms_from_ids<Derived>(state.get_atoms<Derived>()))
    {
        if (!pruning_strategy.prune(atom))
        {
            vertex_index = add_ground_atom_graph_structures(atom, vertex_index);
        }
    }

    // Initialize goal literal vertices and edges
    for (const auto& literal : m_problem->get_goal_condition<Static>())
    {
        if (!pruning_strategy.prune(literal))
        {
            vertex_index = add_ground_literal_graph_structures(state, literal, vertex_index);
        }
    }
    for (const auto& literal : m_problem->get_goal_condition<Fluent>())
    {
        if (!pruning_strategy.prune(literal))
        {
            vertex_index = add_ground_literal_graph_structures(state, literal, vertex_index);
        }
    }
    for (const auto& literal : m_problem->get_goal_condition<Derived>())
    {
        if (!pruning_strategy.prune(literal))
        {
            vertex_index = add_ground_literal_graph_structures(state, literal, vertex_index);
        }
    }

    return m_object_graph;
}

const std::shared_ptr<const ProblemColorFunction>& ObjectGraphFactory::get_coloring_function() const { return m_coloring_function; }

std::ostream& operator<<(std::ostream& out, const ObjectGraph& object_graph)
{
    out << "digraph {\n";

    for (const auto& vertex : object_graph.get_vertex_colored_digraph().get_vertices())
    {
        const auto& color_name = object_graph.get_coloring_function()->get_color_name(vertex.get_color());
        out << "t" << vertex.get_index() << "[";
        out << "label=\"" << color_name << " (" << vertex.get_color() << ")"
            << "\"]\n";
    }

    for (const auto& vertex : object_graph.get_vertex_colored_digraph().get_vertices())
    {
        for (const auto& succ_vertex : object_graph.get_vertex_colored_digraph().get_targets(vertex.get_index()))
        {
            out << "t" << vertex.get_index() << "->"
                << "t" << succ_vertex.get_index() << "\n";
        }
    }

    out << "}";  // end graph

    return out;
}
}
