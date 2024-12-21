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

#include "mimir/search/satisficing_binding_generator/satisficing_binding_generator.hpp"

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
 * Utils
 */

template<PredicateTag P>
static bool nullary_literals_hold(const GroundLiteralList<P>& literals, const FlatBitset& atom_indices)
{
    for (const auto& literal : literals)
    {
        assert(literal->get_atom()->get_arity() == 0);

        if (literal->is_negated() == atom_indices.get(literal->get_atom()->get_index()))
        {
            return false;
        }
    }

    return true;
}

bool nullary_conditions_hold(ExistentiallyQuantifiedConjunctiveCondition precondition, const DenseState& dense_state)
{
    return nullary_literals_hold(precondition->get_nullary_ground_literals<Fluent>(), dense_state.get_atoms<Fluent>())
           && nullary_literals_hold(precondition->get_nullary_ground_literals<Derived>(), dense_state.get_atoms<Derived>());
}

/**
 * SatisficingBindingGenerator
 */

static void clear_full_consistency_graph(std::vector<boost::dynamic_bitset<>>& full_consistency_graph)
{
    for (auto& row : full_consistency_graph)
    {
        row.reset();
    }
}

template<DynamicPredicateTag P>
bool SatisficingBindingGenerator::is_valid_dynamic_binding(const LiteralList<P>& literals, const FlatBitset& atom_indices, const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        auto ground_literal = m_literal_grounder->ground_literal(literal, binding);

        if (ground_literal->is_negated() == atom_indices.get(ground_literal->get_atom()->get_index()))
        {
            return false;
        }
    }

    return true;
}

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

bool SatisficingBindingGenerator::is_valid_binding(const DenseState& dense_state, const ObjectList& binding)
{
    return is_valid_static_binding(m_precondition->get_literals<Static>(), binding)                                          // We need to test all
           && is_valid_dynamic_binding(m_precondition->get_literals<Fluent>(), dense_state.get_atoms<Fluent>(), binding)     // types of conditions
           && is_valid_dynamic_binding(m_precondition->get_literals<Derived>(), dense_state.get_atoms<Derived>(), binding);  // due to over-approx.
}

mimir::generator<ObjectList> SatisficingBindingGenerator::nullary_case(const DenseState& dense_state)
{
    // There are no parameters, meaning that the preconditions are already fully ground. Simply check if the single ground action is applicable.
    auto binding = ObjectList {};

    if (is_valid_binding(dense_state, binding))
    {
        co_yield std::move(binding);
    }
    else
    {
        m_event_handler->on_invalid_binding(binding, *m_literal_grounder->get_pddl_repositories());
    }
}

mimir::generator<ObjectList> SatisficingBindingGenerator::unary_case(const DenseState& dense_state,
                                                                     const AssignmentSet<Fluent>& fluent_assignment_sets,
                                                                     const AssignmentSet<Derived>& derived_assignment_sets)
{
    for (const auto& vertex : m_static_consistency_graph.get_vertices())
    {
        if (vertex.consistent_literals(m_precondition->get_literals<Fluent>(), fluent_assignment_sets)
            && vertex.consistent_literals(m_precondition->get_literals<Derived>(), derived_assignment_sets))
        {
            auto binding = ObjectList { m_literal_grounder->get_pddl_repositories()->get_object(vertex.get_object_index()) };

            if (is_valid_binding(dense_state, binding))
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

mimir::generator<ObjectList> SatisficingBindingGenerator::general_case(const DenseState& dense_state,
                                                                       const AssignmentSet<Fluent>& fluent_assignment_sets,
                                                                       const AssignmentSet<Derived>& derived_assignment_sets,
                                                                       SatisficingBindingGeneratorWorkspace& workspace)
{
    if (m_static_consistency_graph.get_edges().size() == 0)
    {
        co_return;
    }

    auto& full_consistency_graph = workspace.get_or_create_full_consistency_graph(m_static_consistency_graph);
    clear_full_consistency_graph(full_consistency_graph);

    auto& kpkc_workspace = workspace.get_or_create_kpkc_workspace(m_static_consistency_graph);

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
            auto& first_row = full_consistency_graph[first_index];
            auto& second_row = full_consistency_graph[second_index];
            first_row[second_index] = 1;
            second_row[first_index] = 1;
        }
    }

    // Find all cliques of size num_parameters whose labels denote complete assignments that might yield an applicable precondition. The relatively few
    // atoms in the state (compared to the number of possible atoms) lead to very sparse graphs, so the number of maximal cliques of maximum size (#
    // parameters) tends to be very small.

    const auto& vertices = m_static_consistency_graph.get_vertices();
    const auto& partitions = m_static_consistency_graph.get_vertices_by_parameter_index();
    for (const auto& clique : create_k_clique_in_k_partite_graph_generator(full_consistency_graph, partitions, &kpkc_workspace))
    {
        auto binding = ObjectList(clique.size());

        for (std::size_t index = 0; index < clique.size(); ++index)
        {
            const auto& vertex = vertices[clique[index]];
            const auto parameter_index = vertex.get_parameter_index();
            const auto object_index = vertex.get_object_index();
            binding[parameter_index] = m_literal_grounder->get_pddl_repositories()->get_object(object_index);
        }

        if (is_valid_binding(dense_state, binding))
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
    m_static_consistency_graph(m_literal_grounder->get_problem(), 0, m_precondition->get_parameters().size(), m_precondition->get_literals<Static>())
{
}

mimir::generator<ObjectList> SatisficingBindingGenerator::create_binding_generator(State state,
                                                                                   const AssignmentSet<Fluent>& fluent_assignment_set,
                                                                                   const AssignmentSet<Derived>& derived_assignment_set,
                                                                                   SatisficingBindingGeneratorWorkspace& workspace)
{
    auto& dense_state = workspace.get_or_create_dense_state();
    DenseState::translate(state, dense_state);

    return create_binding_generator(dense_state, fluent_assignment_set, derived_assignment_set, workspace);
}

mimir::generator<ObjectList> SatisficingBindingGenerator::create_binding_generator(const DenseState& dense_state,
                                                                                   const AssignmentSet<Fluent>& fluent_assignment_set,
                                                                                   const AssignmentSet<Derived>& derived_assignment_set,
                                                                                   SatisficingBindingGeneratorWorkspace& workspace)
{
    /* Important optimization:
       Moving the nullary_conditions_check out of this function had a large impact on memory allocations/deallocations.
       To avoid accidental errors, we ensure that we checked whether all nullary conditions are satisfied. */
    assert(nullary_conditions_hold(m_precondition, dense_state));

    if (m_precondition->get_arity() == 0)
    {
        return nullary_case(dense_state);
    }
    else if (m_precondition->get_arity() == 1)
    {
        return unary_case(dense_state, fluent_assignment_set, derived_assignment_set);
    }
    else
    {
        return general_case(dense_state, fluent_assignment_set, derived_assignment_set, workspace);
    }
}

mimir::generator<std::pair<ObjectList, std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>>>>
SatisficingBindingGenerator::create_ground_conjunction_generator(State state, SatisficingBindingGeneratorWorkspace& workspace)
{
    auto& dense_state = workspace.get_or_create_dense_state();
    DenseState::translate(state, dense_state);

    return create_ground_conjunction_generator(dense_state, workspace);
}

mimir::generator<std::pair<ObjectList, std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>>>>
SatisficingBindingGenerator::create_ground_conjunction_generator(const DenseState& dense_state, SatisficingBindingGeneratorWorkspace& workspace)
{
    auto& dense_fluent_atoms = dense_state.get_atoms<Fluent>();
    auto& dense_derived_atoms = dense_state.get_atoms<Derived>();

    // We have to check here to avoid unnecessary creations of mimir::generator.
    if (!nullary_conditions_hold(m_precondition, dense_state))
    {
        co_return;
    }

    auto& assignment_set_workspace = workspace.get_or_create_assignment_set_workspace();
    auto& fluent_atoms = assignment_set_workspace.get_or_create_fluent_atoms(dense_fluent_atoms, *m_literal_grounder->get_pddl_repositories());
    auto& fluent_assignment_set = assignment_set_workspace.get_or_create_fluent_assignment_set(m_literal_grounder->get_problem());
    fluent_assignment_set.clear();
    fluent_assignment_set.insert_ground_atoms(fluent_atoms);

    auto& derived_fluents = assignment_set_workspace.get_or_create_derived_atoms(dense_derived_atoms, *m_literal_grounder->get_pddl_repositories());
    auto& derived_assignment_set = assignment_set_workspace.get_or_create_derived_assignment_set(m_literal_grounder->get_problem());
    derived_assignment_set.clear();
    derived_assignment_set.insert_ground_atoms(derived_fluents);

    for (const auto& binding : create_binding_generator(dense_state, fluent_assignment_set, derived_assignment_set, workspace))
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
