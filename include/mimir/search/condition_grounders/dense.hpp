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

#ifndef MIMIR_SEARCH_CONDITION_GROUNDER_DENSE_HPP_
#define MIMIR_SEARCH_CONDITION_GROUNDER_DENSE_HPP_

#include "mimir/algorithms/kpkc.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate_category.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/variable.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/assignment_set.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/consistency_graph.hpp"
#include "mimir/search/applicable_action_generators/grounding_utils.hpp"
#include "mimir/search/condition_grounders/interface.hpp"
#include "mimir/search/states/dense.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <stdexcept>
#include <utility>

namespace mimir
{

template<>
class ConditionGrounder<DenseStateTag> : public IConditionGrounder<ConditionGrounder<DenseStateTag>>
{
private:
    Problem m_problem;
    VariableList m_variables;
    LiteralList<Static> m_static_conditions;
    LiteralList<Fluent> m_fluent_conditions;
    LiteralList<Derived> m_derived_conditions;
    AssignmentSet<Static> m_static_assignment_set;
    PDDLFactories& m_ref_pddl_factories;

    consistency_graph::StaticConsistencyGraph m_static_consistency_graph;

    friend class IConditionGrounder<ConditionGrounder<DenseStateTag>>;

    template<PredicateCategory P>
    bool is_valid_binding(const LiteralList<P>& literals, const DenseState state, const ObjectList& binding)
    {
        for (const auto& literal : literals)
        {
            auto ground_literal = m_ref_pddl_factories.ground_literal(literal, binding);

            if (!state.literal_holds(ground_literal))
            {
                return false;
            }
        }

        return true;
    }

    bool is_valid_binding(const DenseState state, const ObjectList& binding)
    {
        return is_valid_binding(m_static_conditions, state, binding)       // We need to test all
               && is_valid_binding(m_fluent_conditions, state, binding)    // types of conditions
               && is_valid_binding(m_derived_conditions, state, binding);  // due to over-approx.
    }

    /// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
    bool nullary_conditions_hold(const DenseState state)
    {
        return (ground_and_test_nullary_literals(m_fluent_conditions, state, m_ref_pddl_factories))
               && (ground_and_test_nullary_literals(m_derived_conditions, state, m_ref_pddl_factories));
    }

    void nullary_case(const DenseState state, std::vector<ObjectList>& ref_bindings)
    {
        // There are no parameters, meaning that the preconditions are already fully ground. Simply check if the single ground action is applicable.
        auto binding = ObjectList {};

        if (is_valid_binding(state, binding))
        {
            ref_bindings.emplace_back(std::move(binding));
        }
        else
        {
            // m_event_handler->on_ground_inapplicable_action(grounded_action, m_ref_pddl_factories);
        }
    }

    void unary_case(const AssignmentSet<Fluent>& fluent_assignment_sets,
                    const AssignmentSet<Derived>& derived_assignment_sets,
                    const DenseState state,
                    std::vector<ObjectList>& ref_bindings)
    {
        for (const auto& vertex : m_static_consistency_graph.get_vertices())
        {
            if (fluent_assignment_sets.literal_all_consistent(m_fluent_conditions, vertex)
                && derived_assignment_sets.literal_all_consistent(m_derived_conditions, vertex))
            {
                auto binding = ObjectList { m_ref_pddl_factories.get_object(vertex.get_object_index()) };

                if (is_valid_binding(state, binding))
                {
                    ref_bindings.emplace_back(std::move(binding));
                }
                else
                {
                    // m_event_handler->on_ground_inapplicable_action(grounded_action, m_ref_pddl_factories);
                }
            }
        }
    }

    void general_case(const AssignmentSet<Fluent>& fluent_assignment_sets,
                      const AssignmentSet<Derived>& derived_assignment_sets,
                      const DenseState state,
                      std::vector<ObjectList>& ref_bindings)
    {
        if (m_static_consistency_graph.get_edges().size() == 0)
        {
            return;
        }

        const auto& vertices = m_static_consistency_graph.get_vertices();

        std::vector<boost::dynamic_bitset<>> full_consistency_graph(vertices.size(), boost::dynamic_bitset<>(vertices.size()));

        // D: Restrict statically consistent assignments based on the assignments in the current state
        //    and build the consistency graph as an adjacency matrix
        for (const auto& edge : m_static_consistency_graph.get_edges())
        {
            if (fluent_assignment_sets.literal_all_consistent(m_fluent_conditions, edge)
                && derived_assignment_sets.literal_all_consistent(m_derived_conditions, edge))
            {
                const auto first_id = edge.get_src().get_id();
                const auto second_id = edge.get_dst().get_id();
                auto& first_row = full_consistency_graph[first_id];
                auto& second_row = full_consistency_graph[second_id];
                first_row[second_id] = 1;
                second_row[first_id] = 1;
            }
        }

        // Find all cliques of size num_parameters whose labels denote complete assignments that might yield an applicable precondition. The relatively few
        // atoms in the state (compared to the number of possible atoms) lead to very sparse graphs, so the number of maximal cliques of maximum size (#
        // parameters) tends to be very small.

        const auto& partitions = m_static_consistency_graph.get_vertices_by_parameter_index();
        std::vector<std::vector<std::size_t>> cliques;
        find_all_k_cliques_in_k_partite_graph(full_consistency_graph, partitions, cliques);

        for (const auto& clique : cliques)
        {
            auto binding = ObjectList(clique.size());

            for (std::size_t index = 0; index < clique.size(); ++index)
            {
                const auto& vertex = vertices[clique[index]];
                const auto param_index = vertex.get_param_index();
                const auto object_id = vertex.get_object_index();
                binding[param_index] = m_ref_pddl_factories.get_object(object_id);
            }

            if (is_valid_binding(state, binding))
            {
                ref_bindings.emplace_back(std::move(binding));
            }
            else
            {
                // m_event_handler->on_ground_inapplicable_action(grounded_action, m_ref_pddl_factories);
            }
        }
    }

    void get_bindings_impl(const DenseState state,
                           const AssignmentSet<Fluent>& fluent_assignment_set,
                           const AssignmentSet<Derived>& derived_assignment_set,
                           std::vector<ObjectList>& out_bindings)
    {
        out_bindings.clear();

        if (nullary_conditions_hold(state))
        {
            if (m_variables.size() == 0)
            {
                nullary_case(state, out_bindings);
            }
            else if (m_variables.size() == 1)
            {
                unary_case(fluent_assignment_set, derived_assignment_set, state, out_bindings);
            }
            else
            {
                general_case(fluent_assignment_set, derived_assignment_set, state, out_bindings);
            }
        }
    }

public:
    ConditionGrounder(Problem problem,
                      VariableList variables,
                      LiteralList<Static> static_conditions,
                      LiteralList<Fluent> fluent_conditions,
                      LiteralList<Derived> derived_conditions,
                      AssignmentSet<Static> static_assignment_set,
                      PDDLFactories& ref_pddl_factories) :
        m_problem(std::move(problem)),
        m_variables(std::move(variables)),
        m_static_conditions(std::move(static_conditions)),
        m_fluent_conditions(std::move(fluent_conditions)),
        m_derived_conditions(std::move(derived_conditions)),
        m_static_assignment_set(std::move(static_assignment_set)),
        m_ref_pddl_factories(ref_pddl_factories),
        m_static_consistency_graph(m_problem, 0, m_variables.size(), m_static_conditions, m_static_assignment_set)
    {
        /* Error checking. */

        for (const auto& literal : problem->get_static_initial_literals())
        {
            if (literal->is_negated())
            {
                throw std::runtime_error("Negative literals in the initial state is not supported.");
            }
        }

        for (const auto& literal : problem->get_fluent_initial_literals())
        {
            if (literal->is_negated())
            {
                throw std::runtime_error("Negative literals in the initial state is not supported.");
            }
        }
    }
};

}

#endif
