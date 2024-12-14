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

#include "mimir/search/satisficing_binding_generator.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate_tag.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/variable.hpp"
#include "mimir/search/grounders/literal_grounder.hpp"
#include "mimir/search/satisficing_binding_generator/event_handlers/default.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{

/**
 * SatisficingBinderGeneratorWorkspace
 */

SatisficingBinderGeneratorWorkspace::SatisficingBinderGeneratorWorkspace(const consistency_graph::StaticConsistencyGraph& static_consistency_graph) :
    full_consistency_graph(static_consistency_graph.get_vertices().size(), boost::dynamic_bitset<>(static_consistency_graph.get_vertices().size())),
    kpkc_memory(static_consistency_graph.get_vertices_by_parameter_index())
{
}

void SatisficingBinderGeneratorWorkspace::initialize_full_consistency_graph()
{
    for (auto& row : full_consistency_graph)
    {
        row.reset();
    }
}

/**
 * SatisficingBindingGenerator
 */

template<DynamicPredicateTag P>
bool SatisficingBindingGenerator::is_valid_dynamic_binding(const LiteralList<P>& literals, State state, const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        auto ground_literal = m_literal_grounder->ground_literal(literal, binding);

        if (!state->literal_holds(ground_literal))
        {
            return false;
        }
    }

    return true;
}

template bool SatisficingBindingGenerator::is_valid_dynamic_binding(const LiteralList<Fluent>& literals, State state, const ObjectList& binding);
template bool SatisficingBindingGenerator::is_valid_dynamic_binding(const LiteralList<Derived>& literals, State state, const ObjectList& binding);

bool SatisficingBindingGenerator::is_valid_static_binding(const LiteralList<Static>& literals, const ObjectList& binding)
{
    const auto problem = m_literal_grounder->get_problem();

    for (const auto& literal : literals)
    {
        auto ground_literal = m_literal_grounder->ground_literal(literal, binding);

        if (ground_literal->is_negated() == problem->get_static_initial_positive_atoms_bitset().get(ground_literal->get_atom()->get_index()))
        {
            return false;
        }
    }

    return true;
}

bool SatisficingBindingGenerator::is_valid_binding(State state, const ObjectList& binding)
{
    return is_valid_static_binding(m_precondition->get_literals<Static>(), binding)               // We need to test all
           && is_valid_dynamic_binding(m_precondition->get_literals<Fluent>(), state, binding)    // types of conditions
           && is_valid_dynamic_binding(m_precondition->get_literals<Derived>(), state, binding);  // due to over-approx.
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
bool SatisficingBindingGenerator::nullary_conditions_hold(State state) const
{
    return nullary_literals_hold(m_precondition->get_nullary_ground_literals<Fluent>(), state)
           && nullary_literals_hold(m_precondition->get_nullary_ground_literals<Derived>(), state);
}

mimir::generator<ObjectList> SatisficingBindingGenerator::nullary_case(State state)
{
    // There are no parameters, meaning that the preconditions are already fully ground. Simply check if the single ground action is applicable.
    auto binding = ObjectList {};

    if (is_valid_binding(state, binding))
    {
        co_yield std::move(binding);
    }
    else
    {
        m_event_handler->on_invalid_binding(binding, *m_literal_grounder->get_pddl_repositories());
    }
}

mimir::generator<ObjectList>
SatisficingBindingGenerator::unary_case(const AssignmentSet<Fluent>& fluent_assignment_sets, const AssignmentSet<Derived>& derived_assignment_sets, State state)
{
    for (const auto& vertex : m_static_consistency_graph.get_vertices())
    {
        if (vertex.consistent_literals(m_precondition->get_literals<Fluent>(), fluent_assignment_sets)
            && vertex.consistent_literals(m_precondition->get_literals<Derived>(), derived_assignment_sets))
        {
            auto binding = ObjectList { m_literal_grounder->get_pddl_repositories()->get_object(vertex.get_object_index()) };

            if (is_valid_binding(state, binding))
            {
                co_yield std::move(binding);
            }
            else
            {
                m_event_handler->on_invalid_binding(binding, *m_literal_grounder->get_pddl_repositories());
            }
        }
    }
}

mimir::generator<ObjectList> SatisficingBindingGenerator::general_case(const AssignmentSet<Fluent>& fluent_assignment_sets,
                                                                       const AssignmentSet<Derived>& derived_assignment_sets,
                                                                       State state)
{
    if (m_static_consistency_graph.get_edges().size() == 0)
    {
        co_return;
    }

    m_workspace.initialize_full_consistency_graph();

    /* Build the full consistency graph.
       Restricts statically consistent assignments based on the assignments in the current state and builds the consistency graph as an adjacency matrix
    */
    for (const auto& edge : m_static_consistency_graph.get_edges())
    {
        if (edge.consistent_literals(m_precondition->get_literals<Fluent>(), fluent_assignment_sets)
            && edge.consistent_literals(m_precondition->get_literals<Derived>(), derived_assignment_sets))
        {
            const auto first_index = edge.get_src().get_index();
            const auto second_index = edge.get_dst().get_index();
            auto& first_row = m_workspace.full_consistency_graph[first_index];
            auto& second_row = m_workspace.full_consistency_graph[second_index];
            first_row[second_index] = 1;
            second_row[first_index] = 1;
        }
    }

    // Find all cliques of size num_parameters whose labels denote complete assignments that might yield an applicable precondition. The relatively few
    // atoms in the state (compared to the number of possible atoms) lead to very sparse graphs, so the number of maximal cliques of maximum size (#
    // parameters) tends to be very small.

    const auto& vertices = m_static_consistency_graph.get_vertices();
    const auto& partitions = m_static_consistency_graph.get_vertices_by_parameter_index();
    for (const auto& clique : create_k_clique_in_k_partite_graph_generator(m_workspace.full_consistency_graph, partitions, &m_workspace.kpkc_memory))
    {
        auto binding = ObjectList(clique.size());

        for (std::size_t index = 0; index < clique.size(); ++index)
        {
            const auto& vertex = vertices[clique[index]];
            const auto parameter_index = vertex.get_parameter_index();
            const auto object_index = vertex.get_object_index();
            binding[parameter_index] = m_literal_grounder->get_pddl_repositories()->get_object(object_index);
        }

        if (this->is_valid_binding(state, binding))
        {
            co_yield std::move(binding);
        }
        else
        {
            m_event_handler->on_invalid_binding(binding, *m_literal_grounder->get_pddl_repositories());
        }
    };
}

SatisficingBindingGenerator::SatisficingBindingGenerator(std::shared_ptr<LiteralGrounder> literal_grounder,
                                                         ExistentiallyQuantifiedConjunctiveCondition precondition) :
    SatisficingBindingGenerator(std::move(literal_grounder), precondition, std::make_shared<DefaultSatisficingBindingGeneratorEventHandler>())
{
}

SatisficingBindingGenerator::SatisficingBindingGenerator(std::shared_ptr<LiteralGrounder> literal_grounder,
                                                         ExistentiallyQuantifiedConjunctiveCondition precondition,
                                                         std::shared_ptr<ISatisficingBindingGeneratorEventHandler> event_handler) :
    m_literal_grounder(std::move(literal_grounder)),
    m_precondition(precondition),
    m_event_handler(std::move(event_handler)),
    m_static_consistency_graph(m_literal_grounder->get_problem(), 0, m_precondition->get_parameters().size(), m_precondition->get_literals<Static>()),
    m_workspace(m_static_consistency_graph)
{
}

mimir::generator<ObjectList> SatisficingBindingGenerator::create_binding_generator(State state,
                                                                                   const AssignmentSet<Fluent>& fluent_assignment_set,
                                                                                   const AssignmentSet<Derived>& derived_assignment_set)
{
    if (nullary_conditions_hold(state))
    {
        if (m_precondition->get_arity() == 0)
        {
            co_yield mimir::ranges::elements_of(nullary_case(state));
        }
        else if (m_precondition->get_arity() == 1)
        {
            co_yield mimir::ranges::elements_of(unary_case(fluent_assignment_set, derived_assignment_set, state));
        }
        else
        {
            co_yield mimir::ranges::elements_of(general_case(fluent_assignment_set, derived_assignment_set, state));
        }
    }
}

mimir::generator<std::pair<ObjectList, std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>>>>
SatisficingBindingGenerator::create_ground_conjunction_generator(State state)
{
    const auto problem = m_literal_grounder->get_problem();
    const auto pddl_repositories = m_literal_grounder->get_pddl_repositories();

    auto& fluent_predicates = problem->get_domain()->get_predicates<Fluent>();
    auto fluent_atoms = pddl_repositories->get_ground_atoms_from_indices<Fluent>(state->get_atoms<Fluent>());
    auto fluent_assignment_set = AssignmentSet<Fluent>(problem->get_objects().size(), fluent_predicates);
    fluent_assignment_set.insert_ground_atoms(fluent_atoms);

    auto& derived_predicates = problem->get_problem_and_domain_derived_predicates();
    auto derived_atoms = pddl_repositories->get_ground_atoms_from_indices<Derived>(state->get_atoms<Derived>());
    auto derived_assignment_set = AssignmentSet<Derived>(problem->get_objects().size(), derived_predicates);
    derived_assignment_set.insert_ground_atoms(derived_atoms);

    for (const auto& binding : create_binding_generator(state, fluent_assignment_set, derived_assignment_set))
    {
        GroundLiteralList<Static> static_grounded_literals;
        for (const auto& static_literal : m_precondition->get_literals<Static>())
        {
            static_grounded_literals.emplace_back(m_literal_grounder->ground_literal(static_literal, binding));
        }

        GroundLiteralList<Fluent> fluent_grounded_literals;
        for (const auto& fluent_literal : m_precondition->get_literals<Fluent>())
        {
            fluent_grounded_literals.emplace_back(m_literal_grounder->ground_literal(fluent_literal, binding));
        }

        GroundLiteralList<Derived> derived_grounded_literals;
        for (const auto& derived_literal : m_precondition->get_literals<Derived>())
        {
            derived_grounded_literals.emplace_back(m_literal_grounder->ground_literal(derived_literal, binding));
        }

        co_yield std::make_pair(binding, std::make_tuple(static_grounded_literals, fluent_grounded_literals, derived_grounded_literals));
    }
}

const std::shared_ptr<LiteralGrounder>& SatisficingBindingGenerator::get_literal_grounder() const { return m_literal_grounder; }

const ExistentiallyQuantifiedConjunctiveCondition& SatisficingBindingGenerator::get_precondition() const { return m_precondition; }

const std::shared_ptr<ISatisficingBindingGeneratorEventHandler>& SatisficingBindingGenerator::get_event_handler() const { return m_event_handler; }

const consistency_graph::StaticConsistencyGraph& SatisficingBindingGenerator::get_static_consistency_graph() const { return m_static_consistency_graph; }

std::ostream& operator<<(std::ostream& out, const SatisficingBindingGenerator& condition_grounder)
{
    out << "Condition Grounder:" << std::endl;
    out << " - Precondition: " << condition_grounder.get_precondition() << std::endl;
    // TODO: There are some issues with the printers, perhaps move them in a single compilation unit.
    // out << " - Static Consistency Graph: " << std::tie(condition_grounder.get_static_consistency_graph(), *condition_grounder.get_pddl_repositories())
    //     << std::endl;
    return out;
}
}
