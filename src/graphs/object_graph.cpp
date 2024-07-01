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

int ProblemColorFunction::get_color(const Object& object) { return m_name_to_color.at(""); }

Color ProblemColorFunction::get_color(const State& state, const GroundLiteral<Static>& literal, int pos, bool mark_true_goal_literal)
{
    bool is_satisfied_in_goal = m_problem->static_literal_holds(literal);
    return m_name_to_color.at(literal->get_atom()->get_predicate()->get_name() + ":g"
                              + (mark_true_goal_literal ? (is_satisfied_in_goal ? ":true" : ":false") : "") + ":" + std::to_string(pos));
}

const std::string& ProblemColorFunction::get_color_name(Color color) const { return m_color_to_name.at(color); }

Problem ProblemColorFunction::get_problem() const { return m_problem; }

/**
 * ObjectGraph
 */

ObjectGraph::ObjectGraph(std::shared_ptr<ProblemColorFunction> coloring_function) : m_coloring_function(std::move(coloring_function)), m_digraph(false) {}

const std::shared_ptr<ProblemColorFunction>& ObjectGraph::get_coloring_function() const { return m_coloring_function; }

const Digraph& ObjectGraph::get_digraph() const { return m_digraph; }

const ColorList& ObjectGraph::get_vertex_colors() const { return m_vertex_colors; }

const ColorList& ObjectGraph::get_sorted_vertex_colors() const { return m_sorted_vertex_colors; }

const Partitioning& ObjectGraph::get_partitioning() const { return m_partitioning; }

/**
 * ObjectGraphFactory
 */

ObjectGraphFactory::ObjectGraphFactory(Problem problem, std::shared_ptr<PDDLFactories> pddl_factories, bool mark_true_goal_literals) :
    m_problem(problem),
    m_pddl_factories(std::move(pddl_factories)),
    m_mark_true_goal_literals(mark_true_goal_literals),
    m_coloring_function(std::make_shared<ProblemColorFunction>(m_problem)),
    m_object_graph(m_coloring_function)
{
}

int ObjectGraphFactory::add_object_graph_structures(Object object, int num_vertices)
{
    const auto vertex_color = m_coloring_function->get_color(object);
    m_object_graph.m_vertex_colors.push_back(vertex_color);
    m_object_graph.m_sorted_vertex_colors.push_back(vertex_color);
    m_object_to_vertex_index.emplace(object, num_vertices);
    return ++num_vertices;
}

static int compute_num_vertices(const Problem problem, const PDDLFactories& pddl_factories, const State state)
{
    auto num_vertices = static_cast<int>(problem->get_objects().size());
    for (const auto& atom : pddl_factories.get_ground_atoms_from_ids<Static>(problem->get_static_initial_positive_atoms_bitset()))
    {
        num_vertices += atom->get_arity();
    }
    for (const auto& atom : pddl_factories.get_ground_atoms_from_ids<Fluent>(state.get_atoms<Fluent>()))
    {
        num_vertices += atom->get_arity();
    }
    for (const auto& atom : pddl_factories.get_ground_atoms_from_ids<Derived>(state.get_atoms<Derived>()))
    {
        num_vertices += atom->get_arity();
    }
    for (const auto& literal : problem->get_goal_condition<Static>())
    {
        num_vertices += literal->get_atom()->get_arity();
    }
    for (const auto& literal : problem->get_goal_condition<Fluent>())
    {
        num_vertices += literal->get_atom()->get_arity();
    }
    for (const auto& literal : problem->get_goal_condition<Derived>())
    {
        num_vertices += literal->get_atom()->get_arity();
    }
    return num_vertices;
}

const ObjectGraph& ObjectGraphFactory::create(State state)
{
    // Reset data structures
    auto num_vertices = compute_num_vertices(m_problem, *m_pddl_factories, state);
    m_object_graph.m_digraph.reset(num_vertices, false);
    m_object_graph.m_vertex_colors.clear();
    m_object_graph.m_sorted_vertex_colors.clear();
    m_object_to_vertex_index.clear();

    // Initialize object vertices
    auto vertex_index = 0;
    for (const auto& object : m_problem->get_objects())
    {
        vertex_index = add_object_graph_structures(object, vertex_index);
    }

    // Initialize atom vertices and edges
    for (const auto& atom : m_pddl_factories->get_ground_atoms_from_ids<Static>(m_problem->get_static_initial_positive_atoms_bitset()))
    {
        vertex_index = add_ground_atom_graph_structures(atom, vertex_index);
    }
    for (const auto& atom : m_pddl_factories->get_ground_atoms_from_ids<Fluent>(state.get_atoms<Fluent>()))
    {
        vertex_index = add_ground_atom_graph_structures(atom, vertex_index);
    }
    for (const auto& atom : m_pddl_factories->get_ground_atoms_from_ids<Derived>(state.get_atoms<Derived>()))
    {
        vertex_index = add_ground_atom_graph_structures(atom, vertex_index);
    }

    // Initialize goal literal vertices and edges
    for (const auto& literal : m_problem->get_goal_condition<Static>())
    {
        vertex_index = add_ground_literal_graph_structures(state, literal, vertex_index);
    }
    for (const auto& literal : m_problem->get_goal_condition<Fluent>())
    {
        vertex_index = add_ground_literal_graph_structures(state, literal, vertex_index);
    }
    for (const auto& literal : m_problem->get_goal_condition<Derived>())
    {
        vertex_index = add_ground_literal_graph_structures(state, literal, vertex_index);
    }

    // Initialize histogram
    std::sort(m_object_graph.m_sorted_vertex_colors.begin(), m_object_graph.m_sorted_vertex_colors.end());

    // Initialize partitioning
    m_vertex_index_and_color.clear();
    for (int vertex_index = 0; vertex_index < num_vertices; ++vertex_index)
    {
        m_vertex_index_and_color.emplace_back(vertex_index, m_object_graph.m_vertex_colors.at(vertex_index));
    }
    std::sort(m_vertex_index_and_color.begin(), m_vertex_index_and_color.end(), [](const auto& l, const auto& r) { return l.second < r.second; });

    m_object_graph.m_partitioning.m_vertex_index_permutation.clear();
    m_object_graph.m_partitioning.m_partitioning.clear();
    m_object_graph.m_partitioning.m_partition_begin.clear();
    m_object_graph.m_partitioning.m_vertex_index_permutation.resize(num_vertices);
    m_object_graph.m_partitioning.m_partitioning.resize(num_vertices);
    m_object_graph.m_partitioning.m_partition_begin.push_back(0);
    for (int i = 0; i < num_vertices; ++i)
    {
        const auto& [vertex_index, color] = m_vertex_index_and_color.at(i);
        const auto next_has_same_color = ((i < num_vertices - 1) && (color == m_vertex_index_and_color[i + 1].second));
        m_object_graph.m_partitioning.m_vertex_index_permutation.at(i) = vertex_index;
        m_object_graph.m_partitioning.m_partitioning.at(i) = (next_has_same_color) ? 1 : 0;
        if (!next_has_same_color)
        {
            m_object_graph.m_partitioning.m_partition_begin.push_back(i + 1);
        }
    }

    return m_object_graph;
}

std::ostream& operator<<(std::ostream& out, const ObjectGraph& object_graph)
{
    out << "digraph {\n";

    for (int vertex_index = 0; vertex_index < object_graph.get_digraph().get_num_vertices(); ++vertex_index)
    {
        const auto color = object_graph.get_vertex_colors().at(vertex_index);
        const auto& color_name = object_graph.get_coloring_function()->get_color_name(color);
        out << "t" << vertex_index << "[";
        out << "label=\"" << color_name << " (" << color << ")"
            << "\"]\n";
    }

    for (int vertex_index = 0; vertex_index < object_graph.get_digraph().get_num_vertices(); ++vertex_index)
    {
        for (const auto& succ_vertex_index : object_graph.get_digraph().get_forward_successors()[vertex_index])
        {
            out << "t" << vertex_index << "->"
                << "t" << succ_vertex_index << "\n";
        }
    }

    out << "}";  // end graph

    return out;
}
}
