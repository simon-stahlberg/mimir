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

#ifndef MIMIR_SEARCH_RELAXED_PLANNING_GRAPH_HPP_
#define MIMIR_SEARCH_RELAXED_PLANNING_GRAPH_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"

namespace mimir::search
{
class RelaxedPlanningGraph
{
public:
    RelaxedPlanningGraph(formalism::Problem problem);

private:
    struct UnaryGroundAction
    {
        Index m_index;
        const FlatIndexList* m_fluent_precondition;
        const FlatIndexList* m_derived_precondition;
        Index m_fluent_effect;
        ContinuousCost m_cost;
    };
    using UnaryGroundActionList = std::vector<UnaryGroundAction>;

    struct UnaryGroundAxiom
    {
        Index m_index;
        const FlatIndexList* m_fluent_precondition;
        const FlatIndexList* m_derived_precondition;
        Index m_derived_effect;
        // Axioms have 0 cost.
    };

    UnaryGroundActionList m_unary_actions;

    std::vector<FlatBitset> m_fact_layers;
};
}

#endif
