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

#include "mimir/datasets/generalized_state_space/class_graph.hpp"

namespace mimir::graphs
{
template class Vertex<std::tuple<Index, Index>>;
template class Edge<std::tuple<Index, Index>>;
template class StaticGraph<ClassVertex, ClassEdge>;
template class StaticBidirectionalGraph<StaticClassGraph>;

std::ostream& operator<<(std::ostream& out, const ClassVertex& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const ClassEdge& element) { return mimir::print(out, element); }
}

namespace mimir
{
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
}
