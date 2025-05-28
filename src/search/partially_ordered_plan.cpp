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

#include "mimir/search/partially_ordered_plan.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_conjunctive_condition.hpp"
#include "mimir/formalism/ground_effects.hpp"
#include "mimir/graphs/bgl/dynamic_graph_algorithms.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

static std::pair<FlatBitset, FlatBitset> get_conditions(const ProblemImpl& problem, State state, GroundAction action)
{
    auto positive = FlatBitset();
    auto negative = FlatBitset();
    insert_into_bitset(action->get_conjunctive_condition()->get_precondition<PositiveTag, FluentTag>(), positive);
    insert_into_bitset(action->get_conjunctive_condition()->get_precondition<NegativeTag, FluentTag>(), negative);
    for (const auto& cond_effect : action->get_conditional_effects())
    {
        if (is_applicable(cond_effect, problem, state))
        {
            insert_into_bitset(cond_effect->get_conjunctive_condition()->get_precondition<PositiveTag, FluentTag>(), positive);
            insert_into_bitset(cond_effect->get_conjunctive_condition()->get_precondition<NegativeTag, FluentTag>(), negative);
        }
    }
    return std::make_pair(std::move(positive), std::move(negative));
}

static std::pair<FlatBitset, FlatBitset> get_effects(const ProblemImpl& problem, State state, GroundAction action)
{
    auto positive = FlatBitset();
    auto negative = FlatBitset();
    for (const auto& cond_effect : action->get_conditional_effects())
    {
        if (is_applicable(cond_effect, problem, state))
        {
            insert_into_bitset(cond_effect->get_conjunctive_effect()->get_propositional_effects<PositiveTag>(), positive);
            insert_into_bitset(cond_effect->get_conjunctive_effect()->get_propositional_effects<NegativeTag>(), negative);
        }
    }
    return std::make_pair(std::move(positive), std::move(negative));
}

static bool must_precede(const ProblemImpl& problem, State lhs_state, GroundAction lhs_action, State rhs_state, GroundAction rhs_action)
{
    if (!lhs_state->get_atoms<DerivedTag>().empty() || !rhs_state->get_atoms<DerivedTag>().empty())
    {
        throw std::runtime_error("must_precede(problem, lhs_state, lhs_action, rhs_state, rhs_action): Derived atoms not supported yet.");
    }
    if (!lhs_state->get_numeric_variables().empty() || !rhs_state->get_numeric_variables().empty())
    {
        throw std::runtime_error("must_precede(problem, lhs_state, lhs_action, rhs_state, rhs_action): Numerical variables not supported yet.");
    }

    auto [lhs_positive_conditions, lhs_negative_conditions] = get_conditions(problem, lhs_state, lhs_action);
    auto [lhs_positive_effects, lhs_negative_effects] = get_effects(problem, lhs_state, lhs_action);

    auto [rhs_positive_conditions, rhs_negative_conditions] = get_conditions(problem, rhs_state, rhs_action);
    auto [rhs_positive_effects, rhs_negative_effects] = get_effects(problem, rhs_state, rhs_action);

    // Check lhs → rhs
    const bool lhs_to_rhs = !are_disjoint(lhs_positive_effects, rhs_positive_conditions) ||  ///< causal support
                            !are_disjoint(lhs_positive_effects, rhs_negative_conditions) ||  ///< interference: lhs adds something rhs wants false
                            !are_disjoint(lhs_negative_effects, rhs_positive_conditions);    ///< interference: lhs deletes something rhs wants true

    // Check rhs → lhs
    const bool rhs_to_lhs = !are_disjoint(rhs_positive_effects, lhs_negative_effects) ||   ///< interference: rhs adds something lhs wants false
                            !are_disjoint(rhs_negative_effects, lhs_positive_conditions);  ///< interference: rhs deletes something lhs wants true

    return lhs_to_rhs || rhs_to_lhs;
}

PartiallyOrderedPlan::PartiallyOrderedPlan(Plan t_o_plan) : m_t_o_plan(std::move(t_o_plan)), m_graph()
{
    for (size_t i = 0; i < m_t_o_plan.get_length(); ++i)
    {
        m_graph.add_vertex();
    }
    for (size_t i = 0; i < m_t_o_plan.get_length(); ++i)
    {
        const auto& lhs_state = m_t_o_plan.get_states().at(i);
        const auto& lhs_action = m_t_o_plan.get_actions().at(i);

        for (size_t j = i + 1; j < m_t_o_plan.get_length(); ++j)
        {
            if (i == j)
                continue;

            const auto& rhs_state = m_t_o_plan.get_states().at(j);
            const auto& rhs_action = m_t_o_plan.get_actions().at(j);

            if (search::must_precede(*m_t_o_plan.get_search_context()->get_problem(), lhs_state, lhs_action, rhs_state, rhs_action))
            {
                m_graph.add_directed_edge(i, j);
            }
        }
    }

    auto transitive_edges = graphs::EdgeIndexSet {};
    for (const auto& [edge_i_idx, edge_i] : m_graph.get_edges())
    {
        for (const auto& edge_j : m_graph.get_adjacent_edges<graphs::ForwardTag>(edge_i.get_target()))
        {
            for (const auto& edge_k : m_graph.get_adjacent_edges<graphs::ForwardTag>(edge_i.get_source()))
            {
                if (edge_k.get_target() == edge_j.get_target())
                {
                    transitive_edges.insert(edge_k.get_index());
                }
            }
        }
    }
    for (const auto& e_idx : transitive_edges)
    {
        m_graph.remove_edge(e_idx);
    }
}

std::pair<Plan, IndexList> PartiallyOrderedPlan::compute_t_o_plan_with_maximal_makespan() const
{
    auto top_sort = graphs::bgl::topological_sort(graphs::DirectionTaggedType(m_graph, graphs::BackwardTag {}));

    auto states = StateList { m_t_o_plan.get_states().front() };
    auto actions = GroundActionList {};

    auto cur_state = m_t_o_plan.get_states().front();
    auto cur_state_metric_value = compute_state_metric_value(cur_state, *m_t_o_plan.get_search_context()->get_problem());
    for (const auto& i : top_sort)
    {
        const auto action = m_t_o_plan.get_actions().at(i);
        const auto [succ_state, succ_state_metric_value] =
            m_t_o_plan.get_search_context()->get_state_repository()->get_or_create_successor_state(cur_state, action, cur_state_metric_value);

        actions.push_back(action);
        states.push_back(succ_state);

        cur_state = succ_state;
        cur_state_metric_value = succ_state_metric_value;
    }

    // Ensure that reordering preserves costs.
    assert(cur_state_metric_value == m_t_o_plan.get_cost());

    return std::make_pair(Plan(m_t_o_plan.get_search_context(), std::move(states), std::move(actions), cur_state_metric_value), top_sort);
}

const Plan& PartiallyOrderedPlan::get_t_o_plan() const { return m_t_o_plan; }

const graphs::DynamicDigraph& PartiallyOrderedPlan::get_graph() const { return m_graph; }

std::ostream& operator<<(std::ostream& out, const PartiallyOrderedPlan& p_o_plan)
{
    out << "digraph Tree {\n"
           "rankdir=TB;\n\n";

    /* Node definitions */
    for (const auto& [v_idx, v] : p_o_plan.get_graph().get_vertices())
    {
        out << "n" << v.get_index() << " [label=\"index=" << v_idx << ", action=";
        mimir::operator<<(out,
                          std::make_tuple(p_o_plan.get_t_o_plan().get_actions().at(v_idx),
                                          std::cref(*p_o_plan.get_t_o_plan().get_search_context()->get_problem()),
                                          GroundActionImpl::PlanFormatterTag {}));
        out << "\"];\n";
    }
    out << "\n";

    /* Edge definitions */
    for (const auto& [e_idx, e] : p_o_plan.get_graph().get_edges())
    {
        out << "n" << e.get_source() << " -> " << "n" << e.get_target() << " [label=\"index=" << e_idx << "\"];\n";
    }
    out << "\n";

    out << "}\n";  // end graph

    return out;
}

}
