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

#include "mimir/search/match_tree/node_score_functions/min_depth.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/placeholder.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
double MinDepthNodeScoreFunction<Element>::compute_score(const PlaceholderNode<Element>& node)
{
    return node->get_root_distance();
}

template class MinDepthNodeScoreFunction<GroundActionImpl>;
template class MinDepthNodeScoreFunction<GroundAxiomImpl>;

}
