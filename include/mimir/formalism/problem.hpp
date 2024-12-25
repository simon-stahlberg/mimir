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

#ifndef MIMIR_FORMALISM_PROBLEM_HPP_
#define MIMIR_FORMALISM_PROBLEM_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/declarations.hpp"

namespace mimir
{
class ProblemImpl
{
private:
    Index m_index;
    std::optional<fs::path> m_filepath;
    Domain m_domain;
    std::string m_name;
    Requirements m_requirements;
    ObjectList m_objects;
    PredicateList<Derived> m_derived_predicates;
    GroundLiteralList<Static> m_static_initial_literals;
    GroundLiteralList<Fluent> m_fluent_initial_literals;
    NumericFluentList m_numeric_fluents;
    GroundLiteralList<Static> m_static_goal_condition;
    GroundLiteralList<Fluent> m_fluent_goal_condition;
    GroundLiteralList<Derived> m_derived_goal_condition;
    std::optional<OptimizationMetric> m_optimization_metric;
    AxiomList m_axioms;

    // Below: add additional members if needed and initialize them in the constructor

    /* Predicate */
    PredicateList<Derived> m_problem_and_domain_derived_predicates;

    /* Initial state */
    GroundAtomList<Static> m_positive_static_initial_atoms;
    FlatBitset m_positive_static_initial_atoms_bitset;
    FlatIndexList m_positive_static_initial_atoms_indices;
    AssignmentSet<Static> m_positive_static_initial_assignment_set;

    GroundAtomList<Fluent> m_positive_fluent_initial_atoms;

    GroundFunctionMap<ContinuousCost> m_ground_function_to_value;

    /* Goal */
    bool m_static_goal_holds;

    GroundAtomList<Static> m_positive_static_goal_atoms;
    GroundAtomList<Fluent> m_positive_fluent_goal_atoms;
    GroundAtomList<Derived> m_positive_derived_goal_atoms;
    FlatBitset m_positive_static_goal_atoms_bitset;
    FlatBitset m_positive_fluent_goal_atoms_bitset;
    FlatBitset m_positive_derived_goal_atoms_bitset;
    FlatIndexList m_positive_static_goal_atoms_indices;
    FlatIndexList m_positive_fluent_goal_atoms_indices;
    FlatIndexList m_positive_derived_goal_atoms_indices;

    GroundAtomList<Static> m_negative_static_goal_atoms;
    GroundAtomList<Fluent> m_negative_fluent_goal_atoms;
    GroundAtomList<Derived> m_negative_derived_goal_atoms;
    FlatBitset m_negative_static_goal_atoms_bitset;
    FlatBitset m_negative_fluent_goal_atoms_bitset;
    FlatBitset m_negative_derived_goal_atoms_bitset;
    FlatIndexList m_negative_static_goal_atoms_indices;
    FlatIndexList m_negative_fluent_goal_atoms_indices;
    FlatIndexList m_negative_derived_goal_atoms_indices;

    /* Axioms */
    AxiomList m_problem_and_domain_axioms;

    ProblemImpl(Index index,
                std::optional<fs::path> filepath,
                Domain domain,
                std::string name,
                Requirements requirements,
                ObjectList objects,
                PredicateList<Derived> derived_predicates,
                GroundLiteralList<Static> static_initial_literals,
                GroundLiteralList<Fluent> fluent_initial_literals,
                NumericFluentList numeric_fluents,
                GroundLiteralList<Static> static_goal_condition,
                GroundLiteralList<Fluent> fluent_goal_condition,
                GroundLiteralList<Derived> derived_goal_condition,
                std::optional<OptimizationMetric> optimization_metric,
                AxiomList axioms);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ProblemImpl(const ProblemImpl& other) = delete;
    ProblemImpl& operator=(const ProblemImpl& other) = delete;
    ProblemImpl(ProblemImpl&& other) = default;
    ProblemImpl& operator=(ProblemImpl&& other) = default;

    Index get_index() const;
    const std::optional<fs::path>& get_filepath() const;
    const Domain& get_domain() const;
    const std::string& get_name() const;
    const Requirements& get_requirements() const;
    const ObjectList& get_objects() const;
    const PredicateList<Derived>& get_derived_predicates() const;
    const GroundLiteralList<Static>& get_static_initial_literals() const;
    const GroundLiteralList<Fluent>& get_fluent_initial_literals() const;
    const NumericFluentList& get_numeric_fluents() const;
    template<PredicateTag P>
    const GroundLiteralList<P>& get_goal_condition() const;
    const std::optional<OptimizationMetric>& get_optimization_metric() const;
    const AxiomList& get_axioms() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const
    {
        return std::forward_as_tuple(std::as_const(m_domain),
                                     std::as_const(m_requirements),
                                     std::as_const(m_objects),
                                     std::as_const(m_derived_predicates),
                                     std::as_const(m_static_initial_literals),
                                     std::as_const(m_fluent_initial_literals),
                                     std::as_const(m_numeric_fluents),
                                     std::as_const(m_static_goal_condition),
                                     std::as_const(m_fluent_goal_condition),
                                     std::as_const(m_derived_goal_condition),
                                     std::as_const(m_optimization_metric),
                                     std::as_const(m_axioms));
    }

    /**
     * Additional members
     */

    /* Predicate */
    const PredicateList<Derived>& get_problem_and_domain_derived_predicates() const;

    /* Initial state */
    const GroundAtomList<Static>& get_static_initial_atoms() const;
    const FlatBitset& get_static_initial_positive_atoms_bitset() const;
    const FlatIndexList& get_static_initial_positive_atoms_indices() const;
    const AssignmentSet<Static>& get_static_assignment_set() const;

    const GroundAtomList<Fluent>& get_fluent_initial_atoms() const;

    const GroundFunctionMap<ContinuousCost>& get_ground_function_to_value() const;
    ContinuousCost get_ground_function_value(GroundFunction function) const;

    /* Goal */
    bool static_literal_holds(const GroundLiteral<Static> literal) const;  // TODO: probably can go in the future

    bool static_goal_holds() const;

    template<PredicateTag P>
    const GroundAtomList<P>& get_positive_goal_atoms() const;
    template<PredicateTag P>
    const FlatBitset& get_positive_goal_atoms_bitset() const;
    template<PredicateTag P>
    const FlatIndexList& get_positive_goal_atoms_indices() const;

    template<PredicateTag P>
    const GroundAtomList<P>& get_negative_goal_atoms() const;
    template<PredicateTag P>
    const FlatBitset& get_negative_goal_atoms_bitset() const;
    template<PredicateTag P>
    const FlatIndexList& get_negative_goal_atoms_indices() const;

    /* Axioms */
    const AxiomList& get_problem_and_domain_axioms() const;
};

extern std::ostream& operator<<(std::ostream& out, const ProblemImpl& element);

extern std::ostream& operator<<(std::ostream& out, Problem element);

}

#endif
