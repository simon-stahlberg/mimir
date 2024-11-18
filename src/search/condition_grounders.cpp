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

#include "mimir/search/condition_grounders.hpp"

#include "mimir/algorithms/kpkc.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate_tag.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/variable.hpp"
#include "mimir/search/condition_grounders/event_handlers/default.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace mimir
{

template<DynamicPredicateTag P>
bool ConditionGrounder::is_valid_dynamic_binding(const LiteralList<P>& literals, State state, const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        auto ground_literal = m_pddl_repositories->ground_literal(literal, binding);

        if (!state->literal_holds(ground_literal))
        {
            return false;
        }
    }

    return true;
}

template bool ConditionGrounder::is_valid_dynamic_binding(const LiteralList<Fluent>& literals, State state, const ObjectList& binding);
template bool ConditionGrounder::is_valid_dynamic_binding(const LiteralList<Derived>& literals, State state, const ObjectList& binding);

bool ConditionGrounder::is_valid_static_binding(Problem problem, const LiteralList<Static>& literals, const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        auto ground_literal = m_pddl_repositories->ground_literal(literal, binding);

        if (ground_literal->is_negated() == problem->get_static_initial_positive_atoms_bitset().get(ground_literal->get_atom()->get_index()))
        {
            return false;
        }
    }

    return true;
}

bool ConditionGrounder::is_valid_binding(Problem problem, State state, const ObjectList& binding)
{
    return is_valid_static_binding(problem, m_static_conditions, binding)      // We need to test all
           && is_valid_dynamic_binding(m_fluent_conditions, state, binding)    // types of conditions
           && is_valid_dynamic_binding(m_derived_conditions, state, binding);  // due to over-approx.
}

template<DynamicPredicateTag P>
bool ConditionGrounder::nullary_literals_hold(const LiteralList<P>& literals, Problem problem, State state, PDDLRepositories& pddl_repositories)
{
    for (const auto& literal : literals)
    {
        if (literal->get_atom()->get_predicate()->get_arity() == 0)
        {
            if (!state->literal_holds(pddl_repositories.ground_literal(literal, {})))
            {
                return false;
            }
        }
    }

    return true;
}

template bool ConditionGrounder::nullary_literals_hold(const LiteralList<Fluent>& literals, Problem problem, State state, PDDLRepositories& pddl_repositories);
template bool ConditionGrounder::nullary_literals_hold(const LiteralList<Derived>& literals, Problem problem, State state, PDDLRepositories& pddl_repositories);

/// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
bool ConditionGrounder::nullary_conditions_hold(Problem problem, State state)
{
    return nullary_literals_hold(m_fluent_conditions, problem, state, *m_pddl_repositories)
           && nullary_literals_hold(m_derived_conditions, problem, state, *m_pddl_repositories);
}

void ConditionGrounder::nullary_case(State state, std::vector<ObjectList>& ref_bindings)
{
    // There are no parameters, meaning that the preconditions are already fully ground. Simply check if the single ground action is applicable.
    auto binding = ObjectList {};

    if (is_valid_binding(m_problem, state, binding))
    {
        ref_bindings.emplace_back(std::move(binding));
    }
    else
    {
        m_event_handler->on_invalid_binding(binding, *m_pddl_repositories);
    }
}

void ConditionGrounder::unary_case(const AssignmentSet<Fluent>& fluent_assignment_sets,
                                   const AssignmentSet<Derived>& derived_assignment_sets,
                                   State state,
                                   std::vector<ObjectList>& ref_bindings)
{
    for (const auto& vertex : m_static_consistency_graph.get_vertices())
    {
        if (fluent_assignment_sets.consistent_literals(m_fluent_conditions, vertex)
            && derived_assignment_sets.consistent_literals(m_derived_conditions, vertex))
        {
            auto binding = ObjectList { m_pddl_repositories->get_object(vertex.get_object_index()) };

            if (is_valid_binding(m_problem, state, binding))
            {
                ref_bindings.emplace_back(std::move(binding));
            }
            else
            {
                m_event_handler->on_invalid_binding(binding, *m_pddl_repositories);
            }
        }
    }
}

void ConditionGrounder::general_case(const AssignmentSet<Fluent>& fluent_assignment_sets,
                                     const AssignmentSet<Derived>& derived_assignment_sets,
                                     State state,
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
        if (fluent_assignment_sets.consistent_literals(m_fluent_conditions, edge) && derived_assignment_sets.consistent_literals(m_derived_conditions, edge))
        {
            const auto first_index = edge.get_src().get_index();
            const auto second_index = edge.get_dst().get_index();
            auto& first_row = full_consistency_graph[first_index];
            auto& second_row = full_consistency_graph[second_index];
            first_row[second_index] = 1;
            second_row[first_index] = 1;
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
            const auto parameter_index = vertex.get_parameter_index();
            const auto object_index = vertex.get_object_index();
            binding[parameter_index] = m_pddl_repositories->get_object(object_index);
        }

        if (is_valid_binding(m_problem, state, binding))
        {
            ref_bindings.emplace_back(std::move(binding));
        }
        else
        {
            m_event_handler->on_invalid_binding(binding, *m_pddl_repositories);
        }
    }
}

ConditionGrounder::ConditionGrounder(Problem problem,
                                     VariableList variables,
                                     LiteralList<Static> static_conditions,
                                     LiteralList<Fluent> fluent_conditions,
                                     LiteralList<Derived> derived_conditions,
                                     AssignmentSet<Static> static_assignment_set,
                                     std::shared_ptr<PDDLRepositories> pddl_repositories) :
    ConditionGrounder(std::move(problem),
                      std::move(variables),
                      std::move(static_conditions),
                      std::move(fluent_conditions),
                      std::move(derived_conditions),
                      std::move(static_assignment_set),
                      std::move(pddl_repositories),
                      std::make_shared<DefaultConditionGrounderEventHandler>())
{
}

ConditionGrounder::ConditionGrounder(Problem problem,
                                     VariableList variables,
                                     LiteralList<Static> static_conditions,
                                     LiteralList<Fluent> fluent_conditions,
                                     LiteralList<Derived> derived_conditions,
                                     AssignmentSet<Static> static_assignment_set,
                                     std::shared_ptr<PDDLRepositories> pddl_repositories,
                                     std::shared_ptr<IConditionGrounderEventHandler> event_handler) :
    m_problem(std::move(problem)),
    m_variables(std::move(variables)),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions)),
    m_derived_conditions(std::move(derived_conditions)),
    m_static_assignment_set(std::move(static_assignment_set)),
    m_pddl_repositories(std::move(pddl_repositories)),
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

void ConditionGrounder::compute_bindings(State state,
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

Problem ConditionGrounder::get_problem() const { return m_problem; }

const VariableList& ConditionGrounder::get_variables() const { return m_variables; }

template<PredicateTag P>
const LiteralList<P>& ConditionGrounder::get_conditions() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_static_conditions;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_conditions;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_conditions;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const LiteralList<Static>& ConditionGrounder::get_conditions<Static>() const;
template const LiteralList<Fluent>& ConditionGrounder::get_conditions<Fluent>() const;
template const LiteralList<Derived>& ConditionGrounder::get_conditions<Derived>() const;

const AssignmentSet<Static>& ConditionGrounder::get_static_assignment_set() const { return m_static_assignment_set; }

const std::shared_ptr<PDDLRepositories>& ConditionGrounder::get_pddl_repositories() const { return m_pddl_repositories; }

const std::shared_ptr<IConditionGrounderEventHandler>& ConditionGrounder::get_event_handler() const { return m_event_handler; }

const consistency_graph::StaticConsistencyGraph& ConditionGrounder::get_static_consistency_graph() const { return m_static_consistency_graph; }

std::ostream& operator<<(std::ostream& out, const ConditionGrounder& condition_grounder)
{
    out << "Condition Grounder:" << std::endl;
    out << " - Variables: " << condition_grounder.get_variables() << std::endl;
    out << " - Static Conditions: " << condition_grounder.get_conditions<Static>() << std::endl;
    out << " - Fluent Conditions: " << condition_grounder.get_conditions<Fluent>() << std::endl;
    out << " - Derived Conditions: " << condition_grounder.get_conditions<Derived>() << std::endl;
    // TODO: There are some issues with the printers, perhaps move them in a single compilation unit.
    // out << " - Static Consistency Graph: " << std::tie(condition_grounder.get_static_consistency_graph(), *condition_grounder.get_pddl_repositories())
    //     << std::endl;
    return out;
}
}
