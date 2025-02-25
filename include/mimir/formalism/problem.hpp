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
    ObjectList m_problem_and_domain_objects;  ///< Includes domain constants
    PredicateList<Derived> m_derived_predicates;
    PredicateList<Derived> m_problem_and_domain_derived_predicates;  ///< Includes domain derived predicates
    GroundLiteralLists<Static, Fluent> m_initial_literals;
    GroundFunctionValueLists<Static, Fluent> m_initial_function_values;
    std::optional<GroundFunctionValue<Auxiliary>> m_auxiliary_function_value;
    GroundLiteralLists<Static, Fluent, Derived> m_goal_condition;
    GroundNumericConstraintList m_numeric_goal_condition;
    std::optional<OptimizationMetric> m_optimization_metric;
    AxiomList m_axioms;
    AxiomList m_problem_and_domain_axioms;  ///< Includes domain axioms

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

    GroundAtomLists<Static, Fluent, Derived> m_positive_goal_atoms;
    FlatBitsets<Static, Fluent, Derived> m_positive_goal_atoms_bitset;
    FlatIndexLists<Static, Fluent, Derived> m_positive_goal_atoms_indices;

    GroundAtomLists<Static, Fluent, Derived> m_negative_goal_atoms;
    FlatBitsets<Static, Fluent, Derived> m_negative_goal_atoms_bitset;
    FlatIndexLists<Static, Fluent, Derived> m_negative_goal_atoms_indices;

    /* Axioms */
    std::vector<AxiomPartition> m_problem_and_domain_axiom_partitioning;  ///< Obtained from stratification

    ProblemImpl(Index index,
                PDDLRepositories repositories,
                std::optional<fs::path> filepath,
                Domain domain,
                std::string name,
                Requirements requirements,
                ObjectList objects,
                ObjectList problem_and_domain_objects,
                PredicateList<Derived> derived_predicates,
                PredicateList<Derived> problem_and_domain_derived_predicates,
                GroundLiteralLists<Static, Fluent> initial_literals,
                GroundFunctionValueLists<Static, Fluent> initial_function_values,
                std::optional<GroundFunctionValue<Auxiliary>> auxiliary_function_value,
                GroundLiteralLists<Static, Fluent, Derived> goal_condition,
                GroundNumericConstraintList numeric_goal_condition,
                std::optional<OptimizationMetric> optimization_metric,
                AxiomList axioms,
                AxiomList problem_and_domain_axioms);

    // Give access to the constructor.
    friend class ProblemBuilder;

public:
    // moveable but not copyable
    ProblemImpl(const ProblemImpl& other) = delete;
    ProblemImpl& operator=(const ProblemImpl& other) = delete;
    ProblemImpl(ProblemImpl&& other) = default;
    ProblemImpl& operator=(ProblemImpl&& other) = default;

    static Problem create(const fs::path& domain_filepath, const fs::path& problem_filepath, const loki::Options& options = loki::Options());

    Index get_index() const;
    const PDDLRepositories& get_repositories() const;
    const std::optional<fs::path>& get_filepath() const;
    const Domain& get_domain() const;
    const std::string& get_name() const;
    const Requirements& get_requirements() const;
    const ObjectList& get_objects() const;
    const ObjectList& get_problem_and_domain_objects() const;
    const PredicateList<Derived>& get_derived_predicates() const;
    const PredicateList<Derived>& get_problem_and_domain_derived_predicates() const;
    template<StaticOrFluent P>
    const GroundLiteralList<P>& get_initial_literals() const;
    const GroundLiteralLists<Static, Fluent>& get_hana_initial_literals() const;
    template<StaticOrFluent F>
    const GroundFunctionValueList<F>& get_initial_function_values() const;
    const GroundFunctionValueLists<Static, Fluent>& get_hana_initial_function_values() const;
    const std::optional<GroundFunctionValue<Auxiliary>>& get_auxiliary_function_value() const;
    template<StaticOrFluentOrDerived P>
    const GroundLiteralList<P>& get_goal_condition() const;
    const GroundLiteralLists<Static, Fluent, Derived>& get_hana_goal_condition() const;
    const GroundNumericConstraintList& get_numeric_goal_condition() const;
    const std::optional<OptimizationMetric>& get_optimization_metric() const;
    const AxiomList& get_axioms() const;
    const AxiomList& get_problem_and_domain_axioms() const;

    /**
     * Additional members
     */

    /* Initial state */
    const GroundAtomList<Static>& get_static_initial_atoms() const;
    const FlatBitset& get_static_initial_positive_atoms_bitset() const;
    const FlatIndexList& get_static_initial_positive_atoms_indices() const;
    const AssignmentSet<Static>& get_static_assignment_set() const;
    const NumericAssignmentSet<Static>& get_static_initial_numeric_assignment_set() const;

    const GroundAtomList<Fluent>& get_fluent_initial_atoms() const;

    template<StaticOrFluent F>
    const FlatDoubleList& get_function_to_value() const;
    template<StaticOrFluent F>
    ContinuousCost get_function_value(GroundFunction<F> function) const;

    /* Goal */
    bool static_literal_holds(const GroundLiteral<Static> literal) const;  // TODO: probably can go in the future

    bool static_goal_holds() const;

    template<StaticOrFluentOrDerived P>
    const GroundAtomList<P>& get_positive_goal_atoms() const;
    const GroundAtomLists<Static, Fluent, Derived>& get_hana_positive_goal_atoms() const;
    template<StaticOrFluentOrDerived P>
    const FlatBitset& get_positive_goal_atoms_bitset() const;
    const FlatBitsets<Static, Fluent, Derived>& get_hana_positive_goal_atoms_bitset() const;
    template<StaticOrFluentOrDerived P>
    const FlatIndexList& get_positive_goal_atoms_indices() const;
    const FlatIndexLists<Static, Fluent, Derived>& get_hana_positive_goal_atoms_indices() const;

    template<StaticOrFluentOrDerived P>
    const GroundAtomList<P>& get_negative_goal_atoms() const;
    const GroundAtomLists<Static, Fluent, Derived>& get_hana_negative_goal_atoms() const;
    template<StaticOrFluentOrDerived P>
    const FlatBitset& get_negative_goal_atoms_bitset() const;
    const FlatBitsets<Static, Fluent, Derived>& get_hana_negative_goal_atoms_bitset() const;
    template<StaticOrFluentOrDerived P>
    const FlatIndexList& get_negative_goal_atoms_indices() const;
    const FlatIndexLists<Static, Fluent, Derived>& get_hana_negative_goal_atoms_indices() const;

    /* Axioms */
    const std::vector<AxiomPartition>& get_problem_and_domain_axiom_partitioning() const;

    /**
     * Modifiers (Grounding)
     */

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

    GroundFunctionExpression ground(FunctionExpression fexpr, const ObjectList& binding);

    GroundNumericConstraint ground(NumericConstraint numeric_constraint, const ObjectList& binding);

    template<FluentOrAuxiliary F>
    GroundNumericEffect<F> ground(NumericEffect<F> numeric_effect, const ObjectList& binding);

    template<StaticOrFluentOrAuxiliary F>
    GroundFunction<F> ground(Function<F> function, const ObjectList& binding);

    void ground_and_fill_vector(const NumericConstraintList& numeric_constraints,
                                const ObjectList& binding,
                                FlatExternalPtrList<const GroundNumericConstraintImpl>& ref_numeric_constraints);

    void
    ground_and_fill_vector(const NumericEffectList<Fluent>& numeric_effects, const ObjectList& binding, GroundNumericEffectList<Fluent>& ref_numeric_effects);

    void ground_and_fill_optional(const std::optional<NumericEffect<Auxiliary>>& numeric_effect,
                                  const ObjectList& binding,
                                  cista::optional<FlatExternalPtr<const GroundNumericEffectImpl<Auxiliary>>>& ref_numeric_effect);

    GroundAction ground(Action action, ObjectList binding, const std::vector<std::vector<IndexList>>& candidate_conditional_effect_objects_by_parameter_index);

    const GroundActionList& get_ground_actions() const;
    GroundAction get_ground_action(Index action_index) const;
    size_t get_num_ground_actions() const;
    size_t get_estimated_memory_usage_in_bytes_for_actions() const;

    GroundAxiom ground(Axiom axiom, ObjectList binding);

    const GroundAxiomList& get_ground_axioms() const;
    GroundAxiom get_ground_axiom(Index axiom_index) const;
    size_t get_num_ground_axioms() const;
    size_t get_estimated_memory_usage_in_bytes_for_axioms() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const
    {
        return std::forward_as_tuple(std::as_const(get_domain()),
                                     std::as_const(get_requirements()),
                                     std::as_const(get_objects()),
                                     std::as_const(get_derived_predicates()),
                                     std::as_const(get_initial_literals<Static>()),
                                     std::as_const(get_initial_literals<Fluent>()),
                                     std::as_const(get_initial_function_values<Static>()),
                                     std::as_const(get_initial_function_values<Fluent>()),
                                     std::as_const(get_auxiliary_function_value()),
                                     std::as_const(get_goal_condition<Static>()),
                                     std::as_const(get_goal_condition<Fluent>()),
                                     std::as_const(get_goal_condition<Derived>()),
                                     std::as_const(get_numeric_goal_condition()),
                                     std::as_const(get_optimization_metric()),
                                     std::as_const(get_axioms()));
    }
};

extern std::ostream& operator<<(std::ostream& out, const ProblemImpl& element);

extern std::ostream& operator<<(std::ostream& out, Problem element);

}

#endif
