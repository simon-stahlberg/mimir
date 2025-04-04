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

#ifndef MIMIR_SEARCH_SATISFICING_BINDING_GENERATOR_BASE_HPP_
#define MIMIR_SEARCH_SATISFICING_BINDING_GENERATOR_BASE_HPP_

#include "mimir/algorithms/generator.hpp"
#include "mimir/algorithms/kpkc.hpp"
#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/consistency_graph.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/dense_state.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace mimir::search
{

/**
 * SatisficingBindingGenerator
 */

template<typename Derived_>
class SatisficingBindingGenerator
{
public:
    using IEventHandler = satisficing_binding_generator::IEventHandler;
    using EventHandler = satisficing_binding_generator::EventHandler;

    using DefaultEventHandlerImpl = satisficing_binding_generator::DefaultEventHandlerImpl;
    using DefaultEventHandler = satisficing_binding_generator::DefaultEventHandler;

    SatisficingBindingGenerator(formalism::ConjunctiveCondition conjunctive_condition, formalism::Problem problem, EventHandler event_handler = nullptr);

    mimir::generator<formalism::ObjectList>
    create_binding_generator(State state,
                             const formalism::AssignmentSet<formalism::FluentTag>& fluent_assignment_set,
                             const formalism::AssignmentSet<formalism::DerivedTag>& derived_assignment_set,
                             const formalism::NumericAssignmentSet<formalism::StaticTag>& static_numeric_assignment_set,
                             const formalism::NumericAssignmentSet<formalism::FluentTag>& fluent_numeric_assignment_set);

    mimir::generator<formalism::ObjectList>
    create_binding_generator(const DenseState& dense_state,
                             const formalism::AssignmentSet<formalism::FluentTag>& fluent_assignment_set,
                             const formalism::AssignmentSet<formalism::DerivedTag>& derived_assignment_set,
                             const formalism::NumericAssignmentSet<formalism::StaticTag>& static_numeric_assignment_set,
                             const formalism::NumericAssignmentSet<formalism::FluentTag>& fluent_numeric_assignment_set);

    mimir::generator<std::pair<formalism::ObjectList,
                               std::tuple<formalism::GroundLiteralList<formalism::StaticTag>,
                                          formalism::GroundLiteralList<formalism::FluentTag>,
                                          formalism::GroundLiteralList<formalism::DerivedTag>>>>
    create_ground_conjunction_generator(State state);

    mimir::generator<std::pair<formalism::ObjectList,
                               std::tuple<formalism::GroundLiteralList<formalism::StaticTag>,
                                          formalism::GroundLiteralList<formalism::FluentTag>,
                                          formalism::GroundLiteralList<formalism::DerivedTag>>>>
    create_ground_conjunction_generator(const DenseState& dense_state);

    /**
     * Getters
     */

    const formalism::ConjunctiveCondition& get_conjunctive_condition() const;
    const formalism::Problem& get_problem() const;
    const EventHandler& get_event_handler() const;
    const formalism::StaticConsistencyGraph& get_static_consistency_graph() const;

protected:
    formalism::ConjunctiveCondition m_conjunctive_condition;
    formalism::Problem m_problem;
    EventHandler m_event_handler;

    formalism::StaticConsistencyGraph m_static_consistency_graph;

    /* Memory for reuse */
    DenseState m_dense_state;
    formalism::GroundAtomList<formalism::FluentTag> m_fluent_atoms;
    formalism::GroundAtomList<formalism::DerivedTag> m_derived_atoms;
    formalism::GroundFunctionList<formalism::FluentTag> m_fluent_functions;
    formalism::AssignmentSet<formalism::FluentTag> m_fluent_assignment_set;
    formalism::AssignmentSet<formalism::DerivedTag> m_derived_assignment_set;
    formalism::NumericAssignmentSet<formalism::FluentTag> m_numeric_assignment_set;
    std::vector<boost::dynamic_bitset<>> m_full_consistency_graph;
    boost::dynamic_bitset<> m_consistent_vertices;
    KPKCWorkspace m_kpkc_workspace;

    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

    template<formalism::IsFluentOrDerivedTag P>
    bool is_valid_dynamic_binding(const formalism::LiteralList<P>& literals, const FlatBitset& atom_indices, const formalism::ObjectList& binding);

    bool is_valid_static_binding(const formalism::LiteralList<formalism::StaticTag>& literals, const formalism::ObjectList& binding);

    bool
    is_valid_binding(const formalism::NumericConstraintList& constraints, const FlatDoubleList& fluent_numeric_variables, const formalism::ObjectList& binding);

    bool is_valid_binding(formalism::ConjunctiveCondition condition, const DenseState& dense_state, const formalism::ObjectList& binding);

    bool is_valid_binding(const DenseState& dense_state, const formalism::ObjectList& binding);

    mimir::generator<formalism::ObjectList> nullary_case(const DenseState& dense_state);

    mimir::generator<formalism::ObjectList> unary_case(const DenseState& dense_state,
                                                       const formalism::AssignmentSet<formalism::FluentTag>& fluent_assignment_sets,
                                                       const formalism::AssignmentSet<formalism::DerivedTag>& derived_assignment_sets,
                                                       const formalism::NumericAssignmentSet<formalism::StaticTag>& static_numeric_assignment_set,
                                                       const formalism::NumericAssignmentSet<formalism::FluentTag>& fluent_numeric_assignment_set);

    mimir::generator<formalism::ObjectList> general_case(const DenseState& dense_state,
                                                         const formalism::AssignmentSet<formalism::FluentTag>& fluent_assignment_sets,
                                                         const formalism::AssignmentSet<formalism::DerivedTag>& derived_assignment_sets,
                                                         const formalism::NumericAssignmentSet<formalism::StaticTag>& static_numeric_assignment_set,
                                                         const formalism::NumericAssignmentSet<formalism::FluentTag>& fluent_numeric_assignment_set);
};

}

#endif
