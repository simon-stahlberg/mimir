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

#ifndef MIMIR_SEARCH_CONDITION_GROUNDERS_HPP_
#define MIMIR_SEARCH_CONDITION_GROUNDERS_HPP_

#include "mimir/algorithms/kpkc.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate_category.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/variable.hpp"
#include "mimir/search/applicable_action_generators/lifted/assignment_set.hpp"
#include "mimir/search/applicable_action_generators/lifted/consistency_graph.hpp"
#include "mimir/search/condition_grounders/event_handlers/default.hpp"
#include "mimir/search/condition_grounders/event_handlers/interface.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace mimir
{

template<typename S>
concept HasLiteralHolds = requires(S state, GroundLiteral<Fluent> fluent_literal, GroundLiteral<Derived> derived_literal) {
    {
        state.literal_holds(fluent_literal)
    } -> std::convertible_to<bool>;
    {
        state.literal_holds(derived_literal)
    } -> std::convertible_to<bool>;
};

template<typename State>
    requires HasLiteralHolds<State>
class ConditionGrounder
{
private:
    Problem m_problem;
    VariableList m_variables;
    LiteralList<Static> m_static_conditions;
    LiteralList<Fluent> m_fluent_conditions;
    LiteralList<Derived> m_derived_conditions;
    AssignmentSet<Static> m_static_assignment_set;
    std::shared_ptr<PDDLFactories> m_pddl_factories;
    std::shared_ptr<IConditionGrounderEventHandler> m_event_handler;

    consistency_graph::StaticConsistencyGraph m_static_consistency_graph;

    template<DynamicPredicateCategory P>
    bool is_valid_dynamic_binding(const LiteralList<P>& literals, const State state, const ObjectList& binding)
    {
        for (const auto& literal : literals)
        {
            auto ground_literal = m_pddl_factories->ground_literal(literal, binding);

            if (!state.literal_holds(ground_literal))
            {
                return false;
            }
        }

        return true;
    }

    bool is_valid_static_binding(const Problem problem, const LiteralList<Static>& literals, const ObjectList& binding)
    {
        for (const auto& literal : literals)
        {
            auto ground_literal = m_pddl_factories->ground_literal(literal, binding);

            if (ground_literal->is_negated() == problem->get_static_initial_positive_atoms_bitset().get(ground_literal->get_atom()->get_identifier()))
            {
                return false;
            }
        }

        return true;
    }

    bool is_valid_binding(const Problem problem, const State state, const ObjectList& binding)
    {
        return is_valid_static_binding(problem, m_static_conditions, binding)      // We need to test all
               && is_valid_dynamic_binding(m_fluent_conditions, state, binding)    // types of conditions
               && is_valid_dynamic_binding(m_derived_conditions, state, binding);  // due to over-approx.
    }

    template<PredicateCategory P>
    bool nullary_literals_hold(const LiteralList<P>& literals, const Problem problem, const State state, PDDLFactories& pddl_factories)
    {
        for (const auto& literal : literals)
        {
            if (literal->get_atom()->get_predicate()->get_arity() == 0)
            {
                if (!state.literal_holds(pddl_factories.ground_literal(literal, {})))
                {
                    return false;
                }
            }
        }

        return true;
    }

    /// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
    bool nullary_conditions_hold(const Problem problem, const State state)
    {
        return nullary_literals_hold(m_fluent_conditions, problem, state, *m_pddl_factories)
               && nullary_literals_hold(m_derived_conditions, problem, state, *m_pddl_factories);
    }

    void nullary_case(const State state, std::vector<ObjectList>& ref_bindings)
    {
        // There are no parameters, meaning that the preconditions are already fully ground. Simply check if the single ground action is applicable.
        auto binding = ObjectList {};

        if (is_valid_binding(m_problem, state, binding))
        {
            ref_bindings.emplace_back(std::move(binding));
        }
        else
        {
            m_event_handler->on_invalid_binding(binding, *m_pddl_factories);
        }
    }

    void unary_case(const AssignmentSet<Fluent>& fluent_assignment_sets,
                    const AssignmentSet<Derived>& derived_assignment_sets,
                    const State state,
                    std::vector<ObjectList>& ref_bindings)
    {
        for (const auto& vertex : m_static_consistency_graph.get_vertices())
        {
            if (fluent_assignment_sets.consistent_literals(m_fluent_conditions, vertex)
                && derived_assignment_sets.consistent_literals(m_derived_conditions, vertex))
            {
                auto binding = ObjectList { m_pddl_factories->get_object(vertex.get_object_id()) };

                if (is_valid_binding(m_problem, state, binding))
                {
                    ref_bindings.emplace_back(std::move(binding));
                }
                else
                {
                    m_event_handler->on_invalid_binding(binding, *m_pddl_factories);
                }
            }
        }
    }

    void general_case(const AssignmentSet<Fluent>& fluent_assignment_sets,
                      const AssignmentSet<Derived>& derived_assignment_sets,
                      const State state,
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
            if (fluent_assignment_sets.consistent_literals(m_fluent_conditions, edge)
                && derived_assignment_sets.consistent_literals(m_derived_conditions, edge))
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
                const auto param_index = vertex.get_parameter_index();
                const auto object_id = vertex.get_object_id();
                binding[param_index] = m_pddl_factories->get_object(object_id);
            }

            if (is_valid_binding(m_problem, state, binding))
            {
                ref_bindings.emplace_back(std::move(binding));
            }
            else
            {
                m_event_handler->on_invalid_binding(binding, *m_pddl_factories);
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
                      std::shared_ptr<PDDLFactories> pddl_factories) :
        ConditionGrounder(std::move(problem),
                          std::move(variables),
                          std::move(static_conditions),
                          std::move(fluent_conditions),
                          std::move(derived_conditions),
                          std::move(static_assignment_set),
                          std::move(pddl_factories),
                          std::make_shared<DefaultConditionGrounderEventHandler>())
    {
    }

    ConditionGrounder(Problem problem,
                      VariableList variables,
                      LiteralList<Static> static_conditions,
                      LiteralList<Fluent> fluent_conditions,
                      LiteralList<Derived> derived_conditions,
                      AssignmentSet<Static> static_assignment_set,
                      std::shared_ptr<PDDLFactories> pddl_factories,
                      std::shared_ptr<IConditionGrounderEventHandler> event_handler) :
        m_problem(std::move(problem)),
        m_variables(std::move(variables)),
        m_static_conditions(std::move(static_conditions)),
        m_fluent_conditions(std::move(fluent_conditions)),
        m_derived_conditions(std::move(derived_conditions)),
        m_static_assignment_set(std::move(static_assignment_set)),
        m_pddl_factories(std::move(pddl_factories)),
        m_event_handler(std::move(event_handler)),
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

    void compute_bindings(const State state,
                          const AssignmentSet<Fluent>& fluent_assignment_set,
                          const AssignmentSet<Derived>& derived_assignment_set,
                          std::vector<ObjectList>& out_bindings)
    {
        out_bindings.clear();

        if (nullary_conditions_hold(m_problem, state))
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

    friend std::ostream& operator<<(std::ostream& out, const ConditionGrounder<State>& condition_grounder)
    {
        out << "Condition Grounder:" << std::endl;
        out << " - Variables: " << condition_grounder.m_variables << std::endl;
        out << " - Static Conditions: " << condition_grounder.m_static_conditions << std::endl;
        out << " - Fluent Conditions: " << condition_grounder.m_fluent_conditions << std::endl;
        out << " - Derived Conditions: " << condition_grounder.m_derived_conditions << std::endl;
        out << " - Static Consistency Graph: " << std::tie(condition_grounder.m_static_consistency_graph, *condition_grounder.m_pddl_factories) << std::endl;
        return out;
    }
};

}

#endif  // MIMIR_SEARCH_CONDITION_GROUNDERS_HPP_
