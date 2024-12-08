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

#include "mimir/search/grounding/condition_grounder.hpp"

#include "mimir/algorithms/kpkc.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate_tag.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/variable.hpp"
#include "mimir/search/grounding/condition_grounder/event_handlers/default.hpp"
#include "mimir/search/state.hpp"

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

bool ConditionGrounder::is_valid_static_binding(const LiteralList<Static>& literals, const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        auto ground_literal = m_pddl_repositories->ground_literal(literal, binding);

        if (ground_literal->is_negated() == m_problem->get_static_initial_positive_atoms_bitset().get(ground_literal->get_atom()->get_index()))
        {
            return false;
        }
    }

    return true;
}

bool ConditionGrounder::is_valid_binding(State state, const ObjectList& binding)
{
    return is_valid_static_binding(m_static_conditions, binding)               // We need to test all
           && is_valid_dynamic_binding(m_fluent_conditions, state, binding)    // types of conditions
           && is_valid_dynamic_binding(m_derived_conditions, state, binding);  // due to over-approx.
}

template<PredicateTag P>
bool nullary_literals_hold(const GroundLiteralList<P>& literals, State state)
{
    for (const auto& literal : literals)
    {
        assert(literal->get_atom()->get_arity() == 0);

        if (!state->literal_holds(literal))
        {
            return false;
        }
    }

    return true;
}

/// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
bool ConditionGrounder::nullary_conditions_hold(State state)
{
    return nullary_literals_hold(m_nullary_fluent_conditions, state) && nullary_literals_hold(m_nullary_derived_conditions, state);
}

std::generator<ObjectList> ConditionGrounder::nullary_case(State state)
{
    // There are no parameters, meaning that the preconditions are already fully ground. Simply check if the single ground action is applicable.
    auto binding = ObjectList {};

    if (is_valid_binding(state, binding))
    {
        co_yield std::move(binding);
    }
    else
    {
        m_event_handler->on_invalid_binding(binding, *m_pddl_repositories);
    }
}

std::generator<ObjectList>
ConditionGrounder::unary_case(const AssignmentSet<Fluent>& fluent_assignment_sets, const AssignmentSet<Derived>& derived_assignment_sets, State state)
{
    for (const auto& vertex : m_static_consistency_graph.get_vertices())
    {
        if (fluent_assignment_sets.consistent_literals(m_fluent_conditions, vertex)
            && derived_assignment_sets.consistent_literals(m_derived_conditions, vertex))
        {
            auto binding = ObjectList { m_pddl_repositories->get_object(vertex.get_object_index()) };

            if (is_valid_binding(state, binding))
            {
                co_yield std::move(binding);
            }
            else
            {
                m_event_handler->on_invalid_binding(binding, *m_pddl_repositories);
            }
        }
    }
}

std::generator<ObjectList>
ConditionGrounder::general_case(const AssignmentSet<Fluent>& fluent_assignment_sets, const AssignmentSet<Derived>& derived_assignment_sets, State state)
{
    if (m_static_consistency_graph.get_edges().size() == 0)
    {
        co_return;
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

    for (const auto& clique : create_k_clique_in_k_partite_graph_generator(full_consistency_graph, partitions))
    {
        auto binding = ObjectList(clique.size());

        for (std::size_t index = 0; index < clique.size(); ++index)
        {
            const auto& vertex = vertices[clique[index]];
            const auto parameter_index = vertex.get_parameter_index();
            const auto object_index = vertex.get_object_index();
            binding[parameter_index] = m_pddl_repositories->get_object(object_index);
        }

        if (this->is_valid_binding(state, binding))
        {
            co_yield std::move(binding);
        }
        else
        {
            m_event_handler->on_invalid_binding(binding, *m_pddl_repositories);
        }
    };
}

template<PredicateTag P>
static GroundLiteralList<P> ground_nullary_literals(const LiteralList<P>& literals, PDDLRepositories& pddl_repositories)
{
    auto ground_literals = GroundLiteralList<P> {};
    for (const auto& literal : literals)
    {
        if (literal->get_atom()->get_arity() != 0)
            continue;

        ground_literals.push_back(pddl_repositories.ground_literal(literal, {}));
    }

    return ground_literals;
}

ConditionGrounder::ConditionGrounder(Problem problem,
                                     std::shared_ptr<PDDLRepositories> pddl_repositories,
                                     VariableList variables,
                                     LiteralList<Static> static_conditions,
                                     LiteralList<Fluent> fluent_conditions,
                                     LiteralList<Derived> derived_conditions,
                                     AssignmentSet<Static> static_assignment_set) :
    ConditionGrounder(std::move(problem),
                      std::move(pddl_repositories),
                      std::move(variables),
                      std::move(static_conditions),
                      std::move(fluent_conditions),
                      std::move(derived_conditions),
                      std::move(static_assignment_set),
                      std::make_shared<DefaultConditionGrounderEventHandler>())
{
}

ConditionGrounder::ConditionGrounder(Problem problem,
                                     std::shared_ptr<PDDLRepositories> pddl_repositories,
                                     VariableList variables,
                                     LiteralList<Static> static_conditions,
                                     LiteralList<Fluent> fluent_conditions,
                                     LiteralList<Derived> derived_conditions,
                                     AssignmentSet<Static> static_assignment_set,
                                     std::shared_ptr<IConditionGrounderEventHandler> event_handler) :
    m_problem(std::move(problem)),
    m_pddl_repositories(std::move(pddl_repositories)),
    m_variables(std::move(variables)),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions)),
    m_derived_conditions(std::move(derived_conditions)),
    m_static_assignment_set(std::move(static_assignment_set)),
    m_event_handler(std::move(event_handler)),
    m_nullary_static_conditions(ground_nullary_literals(m_static_conditions, *m_pddl_repositories)),
    m_nullary_fluent_conditions(ground_nullary_literals(m_fluent_conditions, *m_pddl_repositories)),
    m_nullary_derived_conditions(ground_nullary_literals(m_derived_conditions, *m_pddl_repositories)),
    m_static_consistency_graph(m_problem, 0, m_variables.size(), m_static_conditions, m_static_assignment_set)
{
}

std::generator<ObjectList> ConditionGrounder::create_binding_generator(State state,
                                                                       const AssignmentSet<Fluent>& fluent_assignment_set,
                                                                       const AssignmentSet<Derived>& derived_assignment_set)
{
    if (nullary_conditions_hold(state))
    {
        if (m_variables.size() == 0)
        {
            co_yield std::ranges::elements_of(nullary_case(state));
        }
        else if (m_variables.size() == 1)
        {
            co_yield std::ranges::elements_of(unary_case(fluent_assignment_set, derived_assignment_set, state));
        }
        else
        {
            co_yield std::ranges::elements_of(general_case(fluent_assignment_set, derived_assignment_set, state));
        }
    }
}

std::generator<std::pair<ObjectList, std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>>>>
ConditionGrounder::create_ground_conjunction_generator(State state)
{
    auto problem = m_problem;

    auto& fluent_predicates = problem->get_domain()->get_predicates<Fluent>();
    auto fluent_atoms = m_pddl_repositories->get_ground_atoms_from_indices<Fluent>(state->get_atoms<Fluent>());
    auto fluent_assignment_set = AssignmentSet<Fluent>(problem, fluent_predicates, fluent_atoms);

    auto& derived_predicates = problem->get_problem_and_domain_derived_predicates();
    auto derived_atoms = m_pddl_repositories->get_ground_atoms_from_indices<Derived>(state->get_atoms<Derived>());
    auto derived_assignment_set = AssignmentSet<Derived>(problem, derived_predicates, derived_atoms);

    for (const auto& binding : create_binding_generator(state, fluent_assignment_set, derived_assignment_set))
    {
        GroundLiteralList<Static> static_grounded_literals;
        for (const auto& static_literal : get_conditions<Static>())
        {
            static_grounded_literals.emplace_back(m_pddl_repositories->ground_literal(static_literal, binding));
        }

        GroundLiteralList<Fluent> fluent_grounded_literals;
        for (const auto& fluent_literal : get_conditions<Fluent>())
        {
            fluent_grounded_literals.emplace_back(m_pddl_repositories->ground_literal(fluent_literal, binding));
        }

        GroundLiteralList<Derived> derived_grounded_literals;
        for (const auto& derived_literal : get_conditions<Derived>())
        {
            derived_grounded_literals.emplace_back(m_pddl_repositories->ground_literal(derived_literal, binding));
        }

        co_yield std::make_pair(binding, std::make_tuple(static_grounded_literals, fluent_grounded_literals, derived_grounded_literals));
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
