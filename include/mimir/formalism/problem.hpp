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
#include "mimir/formalism/axiom_stratification.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/grounding_table.hpp"
#include "mimir/formalism/repositories.hpp"

namespace mimir
{
// A table for each pair (is_negated,predicate_index) since those are context independent.
template<typename T>
using LiteralGroundingTableList = std::array<std::vector<GroundingTable<T>>, 2>;

using PDDLTypeToGroundingTable =
    boost::hana::map<boost::hana::pair<boost::hana::type<GroundLiteral<Static>>, LiteralGroundingTableList<GroundLiteral<Static>>>,
                     boost::hana::pair<boost::hana::type<GroundLiteral<Fluent>>, LiteralGroundingTableList<GroundLiteral<Fluent>>>,
                     boost::hana::pair<boost::hana::type<GroundLiteral<Derived>>, LiteralGroundingTableList<GroundLiteral<Derived>>>,
                     boost::hana::pair<boost::hana::type<GroundFunction<Static>>, GroundingTableList<GroundFunction<Static>>>,
                     boost::hana::pair<boost::hana::type<GroundFunction<Fluent>>, GroundingTableList<GroundFunction<Fluent>>>,
                     boost::hana::pair<boost::hana::type<GroundFunction<Auxiliary>>, GroundingTableList<GroundFunction<Auxiliary>>>,
                     boost::hana::pair<boost::hana::type<GroundFunctionExpression>, GroundingTableList<GroundFunctionExpression>>>;

class ProblemImpl
{
private:
    Index m_index;
    PDDLRepositories m_repositories;
    std::optional<fs::path> m_filepath;
    Domain m_domain;
    std::string m_name;
    Requirements m_requirements;
    ObjectList m_objects;
    PredicateList<Derived> m_derived_predicates;
    GroundLiteralList<Static> m_static_initial_literals;
    GroundLiteralList<Fluent> m_fluent_initial_literals;
    GroundFunctionValueList<Static> m_static_function_values;
    GroundFunctionValueList<Fluent> m_fluent_function_values;
    std::optional<GroundFunctionValue<Auxiliary>> m_auxiliary_function_value;
    GroundLiteralList<Static> m_static_goal_condition;
    GroundLiteralList<Fluent> m_fluent_goal_condition;
    GroundLiteralList<Derived> m_derived_goal_condition;
    GroundNumericConstraintList m_numeric_goal_condition;
    OptimizationMetric m_optimization_metric;
    AxiomList m_axioms;

    /* Grounding */

    PDDLTypeToGroundingTable m_grounding_tables;

    /* For ground actions and axioms we also create a reusable builder. */

    GroundActionImplSet m_ground_actions;
    GroundActionList m_ground_actions_by_index;

    using PerActionData = std::tuple<GroundActionImpl,               ///< Builder
                                     GroundingTable<GroundAction>>;  ///< Cache

    std::unordered_map<Action, PerActionData> m_per_action_datas;

    GroundAxiomImplSet m_ground_axioms;
    GroundAxiomList m_ground_axioms_by_index;

    using PerAxiomData = std::tuple<GroundAxiomImpl,               ///< Builder
                                    GroundingTable<GroundAxiom>>;  ///< Cache

    std::unordered_map<Axiom, PerAxiomData> m_per_axiom_data;

    // Below: add additional members if needed and initialize them in the constructor

    /* Predicate */
    PredicateList<Derived> m_problem_and_domain_derived_predicates;

    /* Initial state */
    GroundAtomList<Static> m_positive_static_initial_atoms;
    FlatBitset m_positive_static_initial_atoms_bitset;
    FlatIndexList m_positive_static_initial_atoms_indices;
    AssignmentSet<Static> m_positive_static_initial_assignment_set;
    NumericAssignmentSet<Static> m_static_initial_numeric_assignment_set;

    GroundAtomList<Fluent> m_positive_fluent_initial_atoms;

    FlatDoubleList m_static_function_to_value;
    FlatDoubleList m_fluent_function_to_value;

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
    std::vector<AxiomPartition> m_problem_and_domain_axiom_partitioning;  ///< Obtained from stratification

    ProblemImpl(Index index,
                PDDLRepositories repositories,
                std::optional<fs::path> filepath,
                Domain domain,
                std::string name,
                Requirements requirements,
                ObjectList objects,
                PredicateList<Derived> derived_predicates,
                GroundLiteralList<Static> static_initial_literals,
                GroundLiteralList<Fluent> fluent_initial_literals,
                GroundFunctionValueList<Static> static_function_values,
                GroundFunctionValueList<Fluent> fluent_function_values,
                std::optional<GroundFunctionValue<Auxiliary>> auxiliary_function_value,
                GroundLiteralList<Static> static_goal_condition,
                GroundLiteralList<Fluent> fluent_goal_condition,
                GroundLiteralList<Derived> derived_goal_condition,
                GroundNumericConstraintList numeric_goal_condition,
                OptimizationMetric optimization_metric,
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
    const PDDLRepositories& repositories() const;
    const std::optional<fs::path>& get_filepath() const;
    const Domain& get_domain() const;
    const std::string& get_name() const;
    const Requirements& get_requirements() const;
    const ObjectList& get_objects() const;
    const PredicateList<Derived>& get_derived_predicates() const;
    const GroundLiteralList<Static>& get_static_initial_literals() const;
    const GroundLiteralList<Fluent>& get_fluent_initial_literals() const;
    template<StaticOrFluentTag F>
    const GroundFunctionValueList<F>& get_function_values() const;
    const std::optional<GroundFunctionValue<Auxiliary>>& get_auxiliary_function_value() const;
    template<StaticOrFluentOrDerived P>
    const GroundLiteralList<P>& get_goal_condition() const;
    const GroundNumericConstraintList& get_numeric_goal_condition() const;
    const OptimizationMetric& get_optimization_metric() const;
    const AxiomList& get_axioms() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const
    {
        return std::forward_as_tuple(std::as_const(m_domain),
                                     std::as_const(m_requirements),
                                     std::as_const(m_objects),
                                     std::as_const(m_derived_predicates),
                                     std::as_const(m_static_initial_literals),
                                     std::as_const(m_fluent_initial_literals),
                                     std::as_const(m_static_function_values),
                                     std::as_const(m_fluent_function_values),
                                     std::as_const(m_auxiliary_function_value),
                                     std::as_const(m_static_goal_condition),
                                     std::as_const(m_fluent_goal_condition),
                                     std::as_const(m_derived_goal_condition),
                                     std::as_const(m_numeric_goal_condition),
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
    const NumericAssignmentSet<Static>& get_static_initial_numeric_assignment_set() const;

    const GroundAtomList<Fluent>& get_fluent_initial_atoms() const;

    template<StaticOrFluentTag F>
    const FlatDoubleList& get_function_to_value() const;
    template<StaticOrFluentTag F>
    ContinuousCost get_function_value(GroundFunction<F> function) const;

    /* Goal */
    bool static_literal_holds(const GroundLiteral<Static> literal) const;  // TODO: probably can go in the future

    bool static_goal_holds() const;

    template<StaticOrFluentOrDerived P>
    const GroundAtomList<P>& get_positive_goal_atoms() const;
    template<StaticOrFluentOrDerived P>
    const FlatBitset& get_positive_goal_atoms_bitset() const;
    template<StaticOrFluentOrDerived P>
    const FlatIndexList& get_positive_goal_atoms_indices() const;

    template<StaticOrFluentOrDerived P>
    const GroundAtomList<P>& get_negative_goal_atoms() const;
    template<StaticOrFluentOrDerived P>
    const FlatBitset& get_negative_goal_atoms_bitset() const;
    template<StaticOrFluentOrDerived P>
    const FlatIndexList& get_negative_goal_atoms_indices() const;

    /* Axioms */
    const AxiomList& get_problem_and_domain_axioms() const;
    const std::vector<AxiomPartition>& get_problem_and_domain_axiom_partitioning() const;

    /* Grounding */

    template<StaticOrFluentOrDerived P>
    GroundLiteral<P> ground(Literal<P> literal, const ObjectList& binding);

    template<StaticOrFluentOrDerived P>
    void ground_and_fill_bitset(const std::vector<Literal<P>>& literals,
                                FlatBitset& ref_positive_bitset,
                                FlatBitset& ref_negative_bitset,
                                const ObjectList& binding);

    template<StaticOrFluentOrDerived P>
    void ground_and_fill_vector(const std::vector<Literal<P>>& literals,
                                FlatIndexList& ref_positive_indices,
                                FlatIndexList& ref_negative_indices,
                                const ObjectList& binding);

    GroundFunctionExpression ground(FunctionExpression fexpr, Problem problem, const ObjectList& binding);

    GroundNumericConstraint ground(NumericConstraint numeric_constraint, Problem problem, const ObjectList& binding);

    template<FluentOrAuxiliary F>
    GroundNumericEffect<F> ground(NumericEffect<F> numeric_effect, Problem problem, const ObjectList& binding);

    template<StaticOrFluentOrAuxiliary F>
    GroundFunction<F> ground(Function<F> function, const ObjectList& binding);

    void ground_and_fill_vector(const NumericConstraintList& numeric_constraints,
                                Problem problem,
                                const ObjectList& binding,
                                FlatExternalPtrList<const GroundNumericConstraintImpl>& ref_numeric_constraints);

    void ground_and_fill_vector(const NumericEffectList<Fluent>& numeric_effects,
                                Problem problem,
                                const ObjectList& binding,
                                GroundNumericEffectList<Fluent>& ref_numeric_effects);

    void ground_and_fill_optional(const std::optional<NumericEffect<Auxiliary>>& numeric_effect,
                                  Problem problem,
                                  const ObjectList& binding,
                                  cista::optional<FlatExternalPtr<const GroundNumericEffectImpl<Auxiliary>>>& ref_numeric_effect);

    GroundAction ground(Action action,
                        Problem problem,
                        ObjectList binding,
                        const std::vector<std::vector<IndexList>>& candidate_conditional_effect_objects_by_parameter_index);

    const GroundActionList& get_ground_actions() const;
    GroundAction get_ground_action(Index action_index) const;
    size_t get_num_ground_actions() const;
    size_t get_estimated_memory_usage_in_bytes_for_actions() const;

    GroundAxiom ground(Axiom axiom, Problem problem, ObjectList binding);

    const GroundAxiomList& get_ground_axioms() const;
    GroundAxiom get_ground_axiom(Index axiom_index) const;
    size_t get_num_ground_axioms() const;
    size_t get_estimated_memory_usage_in_bytes_for_axioms() const;
};

extern std::ostream& operator<<(std::ostream& out, const ProblemImpl& element);

extern std::ostream& operator<<(std::ostream& out, Problem element);

}

#endif
