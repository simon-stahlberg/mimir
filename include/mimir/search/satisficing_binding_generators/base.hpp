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
#include "mimir/search/satisficing_binding_generators/event_handlers/interface.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace mimir::search
{

/**
 * SatisficingBindingGenerator
 */

template<typename Derived_>
class SatisficingBindingGenerator
{
protected:
    formalism::ConjunctiveCondition m_conjunctive_condition;
    formalism::Problem m_problem;
    SatisficingBindingGeneratorEventHandler m_event_handler;

    formalism::consistency_graph::StaticConsistencyGraph m_static_consistency_graph;

    /* Memory for reuse */
    DenseState m_dense_state;
    formalism::GroundAtomList<formalism::Fluent> m_fluent_atoms;
    formalism::GroundAtomList<formalism::Derived> m_derived_atoms;
    formalism::GroundFunctionList<formalism::Fluent> m_fluent_functions;
    formalism::AssignmentSet<formalism::Fluent> m_fluent_assignment_set;
    formalism::AssignmentSet<formalism::Derived> m_derived_assignment_set;
    formalism::NumericAssignmentSet<formalism::Fluent> m_numeric_assignment_set;
    std::vector<boost::dynamic_bitset<>> m_full_consistency_graph;
    boost::dynamic_bitset<> m_consistent_vertices;
    KPKCWorkspace m_kpkc_workspace;

    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

    template<formalism::FluentOrDerived P>
    bool is_valid_dynamic_binding(const formalism::LiteralList<P>& literals, const FlatBitset& atom_indices, const formalism::ObjectList& binding);

    bool is_valid_static_binding(const formalism::LiteralList<formalism::Static>& literals, const formalism::ObjectList& binding);

    bool
    is_valid_binding(const formalism::NumericConstraintList& constraints, const FlatDoubleList& fluent_numeric_variables, const formalism::ObjectList& binding);

    bool is_valid_binding(formalism::ConjunctiveCondition condition, const DenseState& dense_state, const formalism::ObjectList& binding);

    bool is_valid_binding(const DenseState& dense_state, const formalism::ObjectList& binding);

    mimir::generator<formalism::ObjectList> nullary_case(const DenseState& dense_state);

    mimir::generator<formalism::ObjectList> unary_case(const DenseState& dense_state,
                                                       const formalism::AssignmentSet<formalism::Fluent>& fluent_assignment_sets,
                                                       const formalism::AssignmentSet<formalism::Derived>& derived_assignment_sets,
                                                       const formalism::NumericAssignmentSet<formalism::Static>& static_numeric_assignment_set,
                                                       const formalism::NumericAssignmentSet<formalism::Fluent>& fluent_numeric_assignment_set);

    mimir::generator<formalism::ObjectList> general_case(const DenseState& dense_state,
                                                         const formalism::AssignmentSet<formalism::Fluent>& fluent_assignment_sets,
                                                         const formalism::AssignmentSet<formalism::Derived>& derived_assignment_sets,
                                                         const formalism::NumericAssignmentSet<formalism::Static>& static_numeric_assignment_set,
                                                         const formalism::NumericAssignmentSet<formalism::Fluent>& fluent_numeric_assignment_set);

public:
    SatisficingBindingGenerator(formalism::ConjunctiveCondition conjunctive_condition,
                                formalism::Problem problem,
                                std::optional<SatisficingBindingGeneratorEventHandler> event_handler = std::nullopt);

    mimir::generator<formalism::ObjectList> create_binding_generator(State state,
                                                                     const formalism::AssignmentSet<formalism::Fluent>& fluent_assignment_set,
                                                                     const formalism::AssignmentSet<formalism::Derived>& derived_assignment_set,
                                                                     const formalism::NumericAssignmentSet<formalism::Static>& static_numeric_assignment_set,
                                                                     const formalism::NumericAssignmentSet<formalism::Fluent>& fluent_numeric_assignment_set);

    mimir::generator<formalism::ObjectList> create_binding_generator(const DenseState& dense_state,
                                                                     const formalism::AssignmentSet<formalism::Fluent>& fluent_assignment_set,
                                                                     const formalism::AssignmentSet<formalism::Derived>& derived_assignment_set,
                                                                     const formalism::NumericAssignmentSet<formalism::Static>& static_numeric_assignment_set,
                                                                     const formalism::NumericAssignmentSet<formalism::Fluent>& fluent_numeric_assignment_set);

    mimir::generator<std::pair<formalism::ObjectList,
                               std::tuple<formalism::GroundLiteralList<formalism::Static>,
                                          formalism::GroundLiteralList<formalism::Fluent>,
                                          formalism::GroundLiteralList<formalism::Derived>>>>
    create_ground_conjunction_generator(State state);

    mimir::generator<std::pair<formalism::ObjectList,
                               std::tuple<formalism::GroundLiteralList<formalism::Static>,
                                          formalism::GroundLiteralList<formalism::Fluent>,
                                          formalism::GroundLiteralList<formalism::Derived>>>>
    create_ground_conjunction_generator(const DenseState& dense_state);

    /**
     * Getters
     */

    const formalism::ConjunctiveCondition& get_conjunctive_condition() const;
    const formalism::Problem& get_problem() const;
    const SatisficingBindingGeneratorEventHandler& get_event_handler() const;
    const formalism::consistency_graph::StaticConsistencyGraph& get_static_consistency_graph() const;
};

}

#endif
