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

namespace mimir
{
ProblemColorFunction::ProblemColorFunction(const Problem problem) :
    m_problem(problem),
    m_num_objects(problem->get_objects().size()),
    m_num_predicates(m_problem->get_domain()->get_static_predicates().size() + m_problem->get_domain()->get_fluent_predicates().size()
                     + m_problem->get_domain()->get_derived_predicates().size()),
    m_max_predicate_arity(0)
{
    auto lexicographically_sorted_predicate_names = std::vector<std::string> {};
    for (const auto& predicate : m_problem->get_domain()->get_static_predicates())
    {
        m_max_predicate_arity = std::max(m_max_predicate_arity, static_cast<int>(predicate->get_arity()));
        lexicographically_sorted_predicate_names.push_back(predicate->get_name());
    }
    for (const auto& predicate : m_problem->get_domain()->get_fluent_predicates())
    {
        m_max_predicate_arity = std::max(m_max_predicate_arity, static_cast<int>(predicate->get_arity()));
        lexicographically_sorted_predicate_names.push_back(predicate->get_name());
    }
    for (const auto& predicate : m_problem->get_domain()->get_derived_predicates())
    {
        m_max_predicate_arity = std::max(m_max_predicate_arity, static_cast<int>(predicate->get_arity()));
        lexicographically_sorted_predicate_names.push_back(predicate->get_name());
    }
    std::sort(lexicographically_sorted_predicate_names.begin(), lexicographically_sorted_predicate_names.end());
    for (int i = 0; i < m_num_predicates; ++i)
    {
        m_lexicographically_sorted_predicate_names_to_index.emplace(lexicographically_sorted_predicate_names[i], i);
    }
}

int ProblemColorFunction::get_color(const Object object) { return 0; }

ObjectGraph::ObjectGraph(std::shared_ptr<ProblemColorFunction> coloring_function, State state) :
    m_coloring_function(std::move(coloring_function)),
    m_state(state),
    m_digraph()
{
}

const std::shared_ptr<ProblemColorFunction>& ObjectGraph::get_coloring_function() const { return m_coloring_function; }

const Digraph& ObjectGraph::get_digraph() const { return m_digraph; }

const ColorList& ObjectGraph::get_vertex_colors() const { return m_vertex_colors; }

const ColorList& ObjectGraph::get_edge_colors() const { return m_edge_colors; }

}
