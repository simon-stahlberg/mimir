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

#ifndef MIMIR_SEARCH_SATISFICING_BINDING_GENERATOR_BASE_IMPL_HPP_
#define MIMIR_SEARCH_SATISFICING_BINDING_GENERATOR_BASE_IMPL_HPP_

#include "mimir/formalism/consistency_graph.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/tags.hpp"
#include "mimir/formalism/variable.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/assignment_set_utils.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/satisficing_binding_generators/base.hpp"
#include "mimir/search/satisficing_binding_generators/event_handlers/default.hpp"
#include "mimir/search/satisficing_binding_generators/event_handlers/interface.hpp"
#include "mimir/search/state.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace mimir::search
{

/**
 * Implementations
 */

/**
 * Helpers
 */

inline void clear_full_consistency_graph(std::vector<boost::dynamic_bitset<>>& full_consistency_graph)
{
    for (auto& row : full_consistency_graph)
    {
        row.reset();
    }
}

/**
 * SatisficingBindingGenerator
 */

template<typename Derived_>
template<formalism::IsFluentOrDerivedTag P>
bool SatisficingBindingGenerator<Derived_>::is_valid_dynamic_binding(const formalism::LiteralList<P>& literals,
                                                                     const FlatBitset& atom_indices,
                                                                     const formalism::ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        auto ground_literal = m_problem->ground(literal, binding);

        if (ground_literal->get_polarity() != atom_indices.get(ground_literal->get_atom()->get_index()))
        {
            return false;
        }
    }

    return true;
}

template<typename Derived_>
bool SatisficingBindingGenerator<Derived_>::is_valid_binding(const formalism::NumericConstraintList& constraints,
                                                             const FlatDoubleList& fluent_numeric_variables,
                                                             const formalism::ObjectList& binding)
{
    for (const auto& constraint : constraints)
    {
        if (!evaluate(m_problem->ground(constraint, binding), m_problem->get_initial_function_to_value<formalism::StaticTag>(), fluent_numeric_variables))
        {
            return false;
        }
    }
    return true;
}

template<typename Derived_>
bool SatisficingBindingGenerator<Derived_>::is_valid_static_binding(const formalism::LiteralList<formalism::StaticTag>& literals,
                                                                    const formalism::ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        auto ground_literal = m_problem->ground(literal, binding);

        if (ground_literal->get_polarity() != m_problem->get_positive_static_initial_atoms_bitset().get(ground_literal->get_atom()->get_index()))
        {
            return false;
        }
    }

    return true;
}

template<typename Derived_>
bool SatisficingBindingGenerator<Derived_>::is_valid_binding(formalism::ConjunctiveCondition condition,
                                                             const UnpackedStateImpl& unpacked_state,
                                                             const formalism::ObjectList& binding)
{
    return is_valid_static_binding(condition->get_literals<formalism::StaticTag>(), binding)
           && is_valid_dynamic_binding(condition->get_literals<formalism::FluentTag>(), unpacked_state.get_atoms<formalism::FluentTag>(), binding)
           && is_valid_dynamic_binding(condition->get_literals<formalism::DerivedTag>(), unpacked_state.get_atoms<formalism::DerivedTag>(), binding)
           && is_valid_binding(condition->get_numeric_constraints(), unpacked_state.get_numeric_variables(), binding);
}

template<typename Derived_>
bool SatisficingBindingGenerator<Derived_>::is_valid_binding(const UnpackedStateImpl& unpacked_state, const formalism::ObjectList& binding)
{
    const auto base_ok = is_valid_binding(m_conjunctive_condition, unpacked_state, binding);
    if (!base_ok)
    {
        m_event_handler->on_invalid_base_binding(binding);
        return false;
    }
    m_event_handler->on_valid_base_binding(binding);

    const auto derived_ok = self().is_valid_binding_impl(unpacked_state, binding);
    if (!derived_ok)
    {
        m_event_handler->on_invalid_derived_binding(binding);
        return false;
    }
    m_event_handler->on_valid_derived_binding(binding);

    return true;
}

template<typename Derived_>
mimir::generator<formalism::ObjectList> SatisficingBindingGenerator<Derived_>::nullary_case(const UnpackedStateImpl& unpacked_state)
{
    // There are no parameters, meaning that the preconditions are already fully ground. Simply check if the single ground action is applicable.
    auto binding = formalism::ObjectList {};

    if (is_valid_binding(unpacked_state, binding))
        co_yield std::move(binding);
}

template<typename Derived_>
mimir::generator<formalism::ObjectList> SatisficingBindingGenerator<Derived_>::unary_case(const UnpackedStateImpl& unpacked_state,
                                                                                          const formalism::DynamicAssignmentSets& dynamic_assignment_sets)
{
    for (const auto& vertex : m_static_consistency_graph.consistent_vertices(m_problem->get_static_assignment_sets(), dynamic_assignment_sets))
    {
        auto binding = formalism::ObjectList { m_problem->get_repositories().get_object(vertex.get_object_index()) };

        if (is_valid_binding(unpacked_state, binding))
            co_yield std::move(binding);
    }
}

template<typename Derived_>
mimir::generator<formalism::ObjectList> SatisficingBindingGenerator<Derived_>::general_case(const UnpackedStateImpl& unpacked_state,
                                                                                            const formalism::DynamicAssignmentSets& dynamic_assignment_sets)
{
    if (m_static_consistency_graph.get_num_edges() == 0)
    {
        co_return;
    }

    clear_full_consistency_graph(m_full_consistency_graph);

    for (const auto& edge : m_static_consistency_graph.consistent_edges(m_problem->get_static_assignment_sets(), dynamic_assignment_sets))
    {
        const auto first_index = edge.get_src().get_index();
        const auto second_index = edge.get_dst().get_index();
        auto& first_row = m_full_consistency_graph[first_index];
        auto& second_row = m_full_consistency_graph[second_index];
        first_row[second_index] = 1;
        second_row[first_index] = 1;
    }

    // Find all cliques of size num_parameters whose labels denote complete assignments that might yield an applicable precondition. The relatively few
    // atoms in the state (compared to the number of possible atoms) lead to very sparse graphs, so the number of maximal cliques of maximum size (#
    // parameters) tends to be very small.

    const auto& problem = *m_problem;

    const auto& vertices = m_static_consistency_graph.get_vertices();
    const auto& partitions = m_static_consistency_graph.get_vertices_by_parameter_index();
    for (const auto& clique : create_k_clique_in_k_partite_graph_generator(m_full_consistency_graph, partitions))
    {
        auto binding = formalism::ObjectList(clique.size());

        for (std::size_t index = 0; index < clique.size(); ++index)
        {
            const auto& vertex = vertices[clique[index]];
            const auto parameter_index = vertex.get_parameter_index();
            const auto object_index = vertex.get_object_index();
            binding[parameter_index] = problem.get_problem_and_domain_objects()[object_index];
        }

        if (is_valid_binding(unpacked_state, binding))
            co_yield std::move(binding);
    }
}

template<typename Derived_>
SatisficingBindingGenerator<Derived_>::SatisficingBindingGenerator(formalism::ConjunctiveCondition conjunctive_condition,
                                                                   formalism::Problem problem,
                                                                   EventHandler event_handler) :
    m_conjunctive_condition(conjunctive_condition),
    m_problem(problem),
    m_event_handler(event_handler ? event_handler : std::make_shared<DefaultEventHandlerImpl>()),
    m_static_consistency_graph(*m_problem, m_conjunctive_condition, 0, m_conjunctive_condition->get_parameters().size()),
    m_full_consistency_graph(m_static_consistency_graph.get_vertices().size(), boost::dynamic_bitset<>(m_static_consistency_graph.get_vertices().size()))
{
}

template<typename Derived_>
mimir::generator<formalism::ObjectList>
SatisficingBindingGenerator<Derived_>::create_binding_generator(const State& state, const formalism::DynamicAssignmentSets& dynamic_assignment_sets)
{
    return create_binding_generator(state.get_unpacked_state(), dynamic_assignment_sets);
}

template<typename Derived_>
mimir::generator<formalism::ObjectList>
SatisficingBindingGenerator<Derived_>::create_binding_generator(const UnpackedStateImpl& unpacked_state,
                                                                const formalism::DynamicAssignmentSets& dynamic_assignment_sets)
{
    /* Important optimization:
       Moving the nullary_conditions_check out of this function had a large impact on memory allocations/deallocations.
       To avoid accidental errors, we ensure that we checked whether all nullary conditions are satisfied. */
    assert(nullary_conditions_hold(m_conjunctive_condition, unpacked_state));

    if (m_conjunctive_condition->get_arity() == 0)
    {
        return nullary_case(unpacked_state);
    }
    else if (m_conjunctive_condition->get_arity() == 1)
    {
        return unary_case(unpacked_state, dynamic_assignment_sets);
    }
    else
    {
        return general_case(unpacked_state, dynamic_assignment_sets);
    }
}

template<typename Derived_>
mimir::generator<std::pair<formalism::ObjectList,
                           std::tuple<formalism::GroundLiteralList<formalism::StaticTag>,
                                      formalism::GroundLiteralList<formalism::FluentTag>,
                                      formalism::GroundLiteralList<formalism::DerivedTag>>>>
SatisficingBindingGenerator<Derived_>::create_ground_conjunction_generator(const State& state)
{
    return create_ground_conjunction_generator(state.get_unpacked_state());
}

template<typename Derived_>
mimir::generator<std::pair<formalism::ObjectList,
                           std::tuple<formalism::GroundLiteralList<formalism::StaticTag>,
                                      formalism::GroundLiteralList<formalism::FluentTag>,
                                      formalism::GroundLiteralList<formalism::DerivedTag>>>>
SatisficingBindingGenerator<Derived_>::create_ground_conjunction_generator(const UnpackedStateImpl& unpacked_state)
{
    formalism::DynamicAssignmentSets dynamic_assignment_sets(*m_problem);

    initialize(unpacked_state, dynamic_assignment_sets);

    // We have to check here to avoid unnecessary creations of mimir::generator.
    if (!nullary_conditions_hold(m_conjunctive_condition, unpacked_state))
    {
        co_return;
    }

    for (const auto& binding : create_binding_generator(unpacked_state, dynamic_assignment_sets))
    {
        formalism::GroundLiteralList<formalism::StaticTag> static_grounded_literals;
        for (const auto& static_literal : m_conjunctive_condition->get_literals<formalism::StaticTag>())
        {
            static_grounded_literals.emplace_back(m_problem->ground(static_literal, binding));
        }

        formalism::GroundLiteralList<formalism::FluentTag> fluent_grounded_literals;
        for (const auto& fluent_literal : m_conjunctive_condition->get_literals<formalism::FluentTag>())
        {
            fluent_grounded_literals.emplace_back(m_problem->ground(fluent_literal, binding));
        }

        formalism::GroundLiteralList<formalism::DerivedTag> derived_grounded_literals;
        for (const auto& derived_literal : m_conjunctive_condition->get_literals<formalism::DerivedTag>())
        {
            derived_grounded_literals.emplace_back(m_problem->ground(derived_literal, binding));
        }

        co_yield std::make_pair(binding, std::make_tuple(static_grounded_literals, fluent_grounded_literals, derived_grounded_literals));
    }
}

template<typename Derived_>
const formalism::ConjunctiveCondition& SatisficingBindingGenerator<Derived_>::get_conjunctive_condition() const
{
    return m_conjunctive_condition;
}

template<typename Derived_>
const formalism::Problem& SatisficingBindingGenerator<Derived_>::get_problem() const
{
    return m_problem;
}

template<typename Derived_>
const typename SatisficingBindingGenerator<Derived_>::EventHandler& SatisficingBindingGenerator<Derived_>::get_event_handler() const
{
    return m_event_handler;
}

template<typename Derived_>
const formalism::StaticConsistencyGraph& SatisficingBindingGenerator<Derived_>::get_static_consistency_graph() const
{
    return m_static_consistency_graph;
}
}

#endif