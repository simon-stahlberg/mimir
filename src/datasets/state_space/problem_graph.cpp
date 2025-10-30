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

#include "mimir/datasets/state_space/problem_graph.hpp"

namespace mimir::graphs
{
template class Vertex<std::tuple<search::State, formalism::Problem, std::shared_ptr<nauty::SparseGraph>, DiscreteCost, ContinuousCost, bool, bool, bool, bool>>;
template class Edge<std::tuple<formalism::GroundAction, formalism::Problem, ContinuousCost>>;
template class StaticGraph<ProblemVertex, ProblemEdge>;
template class StaticBidirectionalGraph<StaticProblemGraph>;

std::ostream& operator<<(std::ostream& out, const ProblemVertex& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const ProblemEdge& element) { return mimir::print(out, element); }
}

namespace mimir
{
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
                 std::make_tuple(mimir::graphs::get_action(element),
                                 std::cref(*mimir::graphs::get_problem(element)),
                                 mimir::formalism::GroundActionImpl::PlanFormatterTag {}));
    out << "\n"
        << " action_cost=" << mimir::graphs::get_action_cost(element);
    return out;
}
}
