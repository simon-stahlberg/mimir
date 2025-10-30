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

#include "mimir/datasets/formatter.hpp"

#include "mimir/common/formatter.hpp"
#include "mimir/datasets/generalized_state_space/class_graph.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/datasets/state_space/problem_graph.hpp"
#include "mimir/datasets/tuple_graph.hpp"
#include "mimir/datasets/tuple_graph/internal_tuple_graph.hpp"
#include "mimir/formalism/formatter.hpp"
#include "mimir/search/formatter.hpp"
#include "mimir/search/search_context.hpp"

#include <ostream>

namespace mimir
{
namespace graphs
{
std::ostream& operator<<(std::ostream& out, const ProblemVertex& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const ProblemEdge& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const ClassVertex& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const ClassEdge& element) { return mimir::print(out, element); }
}  // end graphs

namespace datasets
{
std::ostream& operator<<(std::ostream& out, const TupleGraphImpl& element) { return mimir::print(out, element); }
}  // end datasets

std::ostream& print(std::ostream& out, const mimir::graphs::ProblemVertex& element)
{
    out << "problem_v_idx=" << element.get_index() << "\n"  //
        << " state=" << mimir::graphs::get_state(element) << "\n"
        << " unit_goal_dist=" << mimir::graphs::get_unit_goal_distance(element) << "\n"      //
        << " action_goal_dist=" << mimir::graphs::get_action_goal_distance(element) << "\n"  //
        << " is_initial=" << mimir::graphs::is_initial(element) << "\n"                      //
        << " is_goal=" << mimir::graphs::is_goal(element) << "\n"                            //
        << " is_unsolvable=" << mimir::graphs::is_unsolvable(element) << "\n"                //
        << " is_alive=" << mimir::graphs::is_alive(element);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::graphs::ProblemEdge& element)
{
    out << "problem_e_idx=" << element.get_index() << "\n"       //
        << " problem_src_idx=" << element.get_source() << "\n"   //
        << " problem_dst_idx=" << element.get_target() << "\n";  //
    out << " action=";
    mimir::print(out,
                 std::make_tuple(std::cref(*mimir::graphs::get_action(element)),
                                 std::cref(*mimir::graphs::get_problem(element)),
                                 mimir::formalism::PlanFormatterTag {}));
    out << "\n"
        << " action_cost=" << mimir::graphs::get_action_cost(element);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::graphs::ClassVertex& element)
{
    out << "class_v_idx=" << element.get_index() << "\n"                         //
        << " problem_idx=" << mimir::graphs::get_problem_index(element) << "\n"  //
        << " problem_v_idx=" << mimir::graphs::get_problem_vertex_index(element);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::graphs::ClassEdge& element)
{
    out << "class_e_idx=" << element.get_index() << "\n"                         //
        << " class_src_idx=" << element.get_source() << "\n"                     //
        << " class_dst_idx=" << element.get_target() << "\n"                     //
        << " problem_idx=" << mimir::graphs::get_problem_index(element) << "\n"  //
        << " problem_e_idx=" << mimir::graphs::get_problem_edge_index(element);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::datasets::TupleGraphImpl& tuple_graph)
{
    const auto& problem_graph = tuple_graph.get_state_space()->get_graph();
    const auto& problem = *tuple_graph.get_state_space()->get_search_context()->get_problem();
    const auto& pddl_repositories = problem.get_repositories();
    auto atom_indices = mimir::search::iw::AtomIndexList {};

    out << "digraph {\n"
        << "rankdir=\"LR\"" << "\n";

    // 3. Tuple nodes.
    for (const auto& v_idx : tuple_graph.get_tuple_vertex_indices_grouped_by_distance().front())
    {
        out << "Dangling" << v_idx << " [ label = \"\", style = invis ]\n";
    }

    for (const auto group : tuple_graph.get_tuple_vertex_indices_grouped_by_distance())
    {
        for (const auto& v_idx : group)
        {
            const auto& vertex = tuple_graph.get_graph().get_vertex(v_idx);

            out << "t" << v_idx << "[";
            out << "label=<";
            out << "index=" << v_idx << "<BR/>";
            const auto fluent_atoms = pddl_repositories.get_ground_atoms_from_indices<mimir::formalism::FluentTag>(mimir::graphs::get_atom_tuple(vertex));
            out << "fluent_atoms=" << fluent_atoms;
            out << "<BR/>";
            out << "states=[";
            for (size_t i = 0; i < mimir::graphs::get_problem_vertices(vertex).size(); ++i)
            {
                const auto state = mimir::graphs::get_state(problem_graph.get_vertex(mimir::graphs::get_problem_vertices(vertex).at(i)));
                if (i != 0)
                {
                    out << "<BR/>";
                }
                mimir::print(out, std::make_tuple(std::cref(state), std::cref(problem)));
            }
            out << "]>]\n";
        }
    }
    // 4. Group states with same distance together
    // 5. Tuple edges
    out << "{\n";
    for (const auto& v_idx : tuple_graph.get_tuple_vertex_indices_grouped_by_distance().front())
    {
        out << "Dangling" << v_idx << "->t" << v_idx << "\n";
    }
    out << "}\n";
    for (const auto group : tuple_graph.get_tuple_vertex_indices_grouped_by_distance())
    {
        out << "{\n";
        for (const auto& v_idx : group)
        {
            for (const auto& adj_v_idx : tuple_graph.get_graph().get_adjacent_vertex_indices<mimir::graphs::ForwardTag>(v_idx))
            {
                out << "t" << v_idx << "->" << "t" << adj_v_idx << "\n";
            }
        }
        out << "}\n";
    }
    out << "}\n";  // end digraph

    return out;
}
}
