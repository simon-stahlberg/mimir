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
#include "mimir/formalism/problem_context.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/dense_state.hpp"
#include "mimir/search/satisficing_binding_generators/event_handlers/interface.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace mimir
{

/**
 * SatisficingBindingGenerator
 */

template<typename Derived_>
class SatisficingBindingGenerator
{
protected:
    ConjunctiveCondition m_conjunctive_condition;
    ProblemContext m_problem_context;
    std::shared_ptr<ISatisficingBindingGeneratorEventHandler> m_event_handler;

    consistency_graph::StaticConsistencyGraph m_static_consistency_graph;

    /* Memory for reuse */
    DenseState m_dense_state;
    GroundAtomList<Fluent> m_fluent_atoms;
    GroundAtomList<Derived> m_derived_atoms;
    GroundFunctionList<Fluent> m_fluent_functions;
    AssignmentSet<Fluent> m_fluent_assignment_set;
    AssignmentSet<Derived> m_derived_assignment_set;
    NumericAssignmentSet<Fluent> m_numeric_assignment_set;
    std::vector<boost::dynamic_bitset<>> m_full_consistency_graph;
    boost::dynamic_bitset<> m_consistent_vertices;
    KPKCWorkspace m_kpkc_workspace;

    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

    template<FluentOrDerived P>
    bool is_valid_dynamic_binding(const LiteralList<P>& literals, const FlatBitset& atom_indices, const ObjectList& binding);

    bool is_valid_static_binding(const LiteralList<Static>& literals, const ObjectList& binding);

    bool is_valid_binding(const NumericConstraintList& constraints, const FlatDoubleList& fluent_numeric_variables, const ObjectList& binding);

    bool is_valid_binding(ConjunctiveCondition condition, const DenseState& dense_state, const ObjectList& binding);

    bool is_valid_binding(const DenseState& dense_state, const ObjectList& binding);

    mimir::generator<ObjectList> nullary_case(const DenseState& dense_state);

    mimir::generator<ObjectList> unary_case(const DenseState& dense_state,
                                            const AssignmentSet<Fluent>& fluent_assignment_sets,
                                            const AssignmentSet<Derived>& derived_assignment_sets,
                                            const NumericAssignmentSet<Static>& static_numeric_assignment_set,
                                            const NumericAssignmentSet<Fluent>& fluent_numeric_assignment_set);

    mimir::generator<ObjectList> general_case(const DenseState& dense_state,
                                              const AssignmentSet<Fluent>& fluent_assignment_sets,
                                              const AssignmentSet<Derived>& derived_assignment_sets,
                                              const NumericAssignmentSet<Static>& static_numeric_assignment_set,
                                              const NumericAssignmentSet<Fluent>& fluent_numeric_assignment_set);

public:
    SatisficingBindingGenerator(ConjunctiveCondition conjunctive_condition,
                                ProblemContext problem_context,
                                std::optional<std::shared_ptr<ISatisficingBindingGeneratorEventHandler>> event_handler = std::nullopt);

    mimir::generator<ObjectList> create_binding_generator(State state,
                                                          const AssignmentSet<Fluent>& fluent_assignment_set,
                                                          const AssignmentSet<Derived>& derived_assignment_set,
                                                          const NumericAssignmentSet<Static>& static_numeric_assignment_set,
                                                          const NumericAssignmentSet<Fluent>& fluent_numeric_assignment_set);

    mimir::generator<ObjectList> create_binding_generator(const DenseState& dense_state,
                                                          const AssignmentSet<Fluent>& fluent_assignment_set,
                                                          const AssignmentSet<Derived>& derived_assignment_set,
                                                          const NumericAssignmentSet<Static>& static_numeric_assignment_set,
                                                          const NumericAssignmentSet<Fluent>& fluent_numeric_assignment_set);

    mimir::generator<std::pair<ObjectList, std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>>>>
    create_ground_conjunction_generator(State state);

    mimir::generator<std::pair<ObjectList, std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>>>>
    create_ground_conjunction_generator(const DenseState& dense_state);

    /**
     * Getters
     */

    const ConjunctiveCondition& get_conjunctive_condition() const;
    const ProblemContext& get_problem_context() const;
    const std::shared_ptr<ISatisficingBindingGeneratorEventHandler>& get_event_handler() const;
    const consistency_graph::StaticConsistencyGraph& get_static_consistency_graph() const;
};

}

#endif
