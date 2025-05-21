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

#ifndef MIMIR_SEARCH_PARTIALLY_ORDERED_PLAN_HPP_
#define MIMIR_SEARCH_PARTIALLY_ORDERED_PLAN_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/concrete/digraph.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/plan.hpp"

namespace mimir::search
{

/// @brief `PartiallyOrderedPlan` (POP) represents a total-order plan (TOP) augmented with a strict partial order `<`, where `a_i < a_j` indicates that
/// action `a_i` must strictly precede action `a_j` in any valid execution order. The relation `<` is irreflexive.
///
/// More precisely, `<(a_i, a_j)` holds if `a_i` adds a precondition of `a_j`, or if `a_i` deletes a precondition of `a_j`. This causal relation is defined
/// independently of whether any conditional effects are actually triggered in preceding states.
///
/// Notes: the current implementation only supports fluent ground atoms and throws an exception for plans that use derived atoms or numeric constraints.
class PartiallyOrderedPlan
{
private:
    Plan m_t_o_plan;

    graphs::DynamicDigraph m_graph;

public:
    explicit PartiallyOrderedPlan(Plan t_o_plan);

    std::pair<Plan, IndexList> compute_t_o_plan_with_maximal_makespan() const;

    const Plan& get_t_o_plan() const;
    const graphs::DynamicDigraph& get_graph() const;
};

extern std::ostream& operator<<(std::ostream& out, const PartiallyOrderedPlan& p_o_plan);

}

#endif
