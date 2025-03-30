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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_POLICY_GRAPH_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_POLICY_GRAPH_HPP_

#include "mimir/graphs/dynamic_graph.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/graphs/graph_vertices.hpp"
#include "mimir/languages/general_policies/declarations.hpp"

namespace mimir::graphs
{
using PolicyVertex = Vertex<languages::general_policies::ConditionSet>;
using PolicyEdge = Edge<languages::general_policies::EffectSet>;
using PolicyGraph = DynamicGraph<PolicyVertex, PolicyEdge>;

inline const auto& get_conditions(const PolicyVertex& vertex) { return vertex.get_property<0>(); }

inline const auto& get_effects(const PolicyEdge& edge) { return edge.get_property<0>(); }

extern std::ostream& operator<<(std::ostream& out, const PolicyVertex& vertex);

extern std::ostream& operator<<(std::ostream& out, const PolicyEdge& edge);
}

namespace mimir::languages::general_policies
{
extern graphs::PolicyGraph create_policy_graph(GeneralPolicy policy, Repositories& repositories);
}

#endif