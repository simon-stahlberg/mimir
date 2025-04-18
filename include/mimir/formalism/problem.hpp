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
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/problem_details.hpp"
#include "mimir/formalism/repositories.hpp"

namespace mimir::formalism
{
class ProblemImpl
{
private:
    Index m_index;
    Repositories m_repositories;
    std::optional<fs::path> m_filepath;
    Domain m_domain;
    std::string m_name;
    Requirements m_requirements;
    ObjectList m_objects;
    ObjectList m_problem_and_domain_objects;  ///< Includes domain constants
    PredicateList<DerivedTag> m_derived_predicates;
    PredicateList<DerivedTag> m_problem_and_domain_derived_predicates;  ///< Includes domain derived predicates
    GroundLiteralLists<StaticTag, FluentTag> m_initial_literals;
    GroundFunctionValueLists<StaticTag, FluentTag> m_initial_function_values;
    std::optional<GroundFunctionValue<AuxiliaryTag>> m_auxiliary_function_value;
    GroundLiteralLists<StaticTag, FluentTag, DerivedTag> m_goal_condition;
    GroundNumericConstraintList m_numeric_goal_condition;
    std::optional<OptimizationMetric> m_optimization_metric;
    AxiomList m_axioms;
    AxiomList m_problem_and_domain_axioms;  ///< Includes domain axioms

    // Below: add additional members if needed and initialize them in the constructor

    problem::Details m_details;  ///< We hide the details in a struct.

    FlatIndexListSet m_flat_index_list_set;    ///< Stores all created atom lists.
    FlatDoubleListSet m_flat_double_list_set;  ///< Stores all created numeric variable lists.

    ProblemImpl(Index index,
                Repositories repositories,
                std::optional<fs::path> filepath,
                Domain domain,
                std::string name,
                Requirements requirements,
                ObjectList objects,
                ObjectList problem_and_domain_objects,
                PredicateList<DerivedTag> derived_predicates,
                PredicateList<DerivedTag> problem_and_domain_derived_predicates,
                GroundLiteralLists<StaticTag, FluentTag> initial_literals,
                GroundFunctionValueLists<StaticTag, FluentTag> initial_function_values,
                std::optional<GroundFunctionValue<AuxiliaryTag>> auxiliary_function_value,
                GroundLiteralLists<StaticTag, FluentTag, DerivedTag> goal_condition,
                GroundNumericConstraintList numeric_goal_condition,
                std::optional<OptimizationMetric> optimization_metric,
                AxiomList axioms,
                AxiomList problem_and_domain_axioms);

    // Give access to the constructor.
    friend class ProblemBuilder;

public:
    // not moveable and not copieable
    ProblemImpl(const ProblemImpl& other) = delete;
    ProblemImpl& operator=(const ProblemImpl& other) = delete;
    ProblemImpl(ProblemImpl&& other) = delete;
    ProblemImpl& operator=(ProblemImpl&& other) = delete;

    static Problem create(const fs::path& domain_filepath, const fs::path& problem_filepath, const loki::Options& options = loki::Options());

    Index get_index() const;
    const Repositories& get_repositories() const;
    const std::optional<fs::path>& get_filepath() const;
    const Domain& get_domain() const;
    const std::string& get_name() const;
    Requirements get_requirements() const;
    const ObjectList& get_objects() const;
    const ObjectList& get_problem_and_domain_objects() const;
    const PredicateList<DerivedTag>& get_derived_predicates() const;
    const PredicateList<DerivedTag>& get_problem_and_domain_derived_predicates() const;
    template<IsStaticOrFluentTag P>
    const GroundLiteralList<P>& get_initial_literals() const;
    const GroundLiteralLists<StaticTag, FluentTag>& get_hana_initial_literals() const;
    template<IsStaticOrFluentTag F>
    const GroundFunctionValueList<F>& get_initial_function_values() const;
    const GroundFunctionValueLists<StaticTag, FluentTag>& get_hana_initial_function_values() const;
    const std::optional<GroundFunctionValue<AuxiliaryTag>>& get_auxiliary_function_value() const;
    template<IsStaticOrFluentOrDerivedTag P>
    const GroundLiteralList<P>& get_goal_condition() const;
    const GroundLiteralLists<StaticTag, FluentTag, DerivedTag>& get_hana_goal_condition() const;
    const GroundNumericConstraintList& get_numeric_goal_condition() const;
    const std::optional<OptimizationMetric>& get_optimization_metric() const;
    const AxiomList& get_axioms() const;
    const AxiomList& get_problem_and_domain_axioms() const;

    /// @brief Get the total number of bytes.
    /// @return the number of bytes.
    size_t get_estimated_memory_usage_in_bytes() const;

    /**
     * Additional members
     */

    /* Initial state */
    const GroundAtomList<StaticTag>& get_static_initial_atoms() const;
    const FlatBitset& get_static_initial_positive_atoms_bitset() const;
    const FlatIndexList& get_static_initial_positive_atoms_indices() const;
    const AssignmentSet<StaticTag>& get_static_assignment_set() const;
    const NumericAssignmentSet<StaticTag>& get_static_initial_numeric_assignment_set() const;

    const GroundAtomList<FluentTag>& get_fluent_initial_atoms() const;

    template<IsStaticOrFluentTag F>
    const FlatDoubleList& get_initial_function_to_value() const;
    const FlatDoubleLists<StaticTag, FluentTag>& get_hana_initial_function_to_value() const;
    template<IsStaticOrFluentTag F>
    ContinuousCost get_initial_function_value(GroundFunction<F> function) const;

    /* Goal */

    bool static_literal_holds(const GroundLiteral<StaticTag> literal) const;  // TODO: probably can go in the future

    bool static_goal_holds() const;

    template<IsStaticOrFluentOrDerivedTag P>
    const GroundAtomList<P>& get_positive_goal_atoms() const;
    const GroundAtomLists<StaticTag, FluentTag, DerivedTag>& get_hana_positive_goal_atoms() const;
    template<IsStaticOrFluentOrDerivedTag P>
    const FlatBitset& get_positive_goal_atoms_bitset() const;
    const FlatBitsets<StaticTag, FluentTag, DerivedTag>& get_hana_positive_goal_atoms_bitset() const;
    template<IsStaticOrFluentOrDerivedTag P>
    const FlatIndexList& get_positive_goal_atoms_indices() const;
    const FlatIndexLists<StaticTag, FluentTag, DerivedTag>& get_hana_positive_goal_atoms_indices() const;

    template<IsStaticOrFluentOrDerivedTag P>
    const GroundAtomList<P>& get_negative_goal_atoms() const;
    const GroundAtomLists<StaticTag, FluentTag, DerivedTag>& get_hana_negative_goal_atoms() const;
    template<IsStaticOrFluentOrDerivedTag P>
    const FlatBitset& get_negative_goal_atoms_bitset() const;
    const FlatBitsets<StaticTag, FluentTag, DerivedTag>& get_hana_negative_goal_atoms_bitset() const;
    template<IsStaticOrFluentOrDerivedTag P>
    const FlatIndexList& get_negative_goal_atoms_indices() const;
    const FlatIndexLists<StaticTag, FluentTag, DerivedTag>& get_hana_negative_goal_atoms_indices() const;

    /* Axioms */

    const std::vector<AxiomPartition>& get_problem_and_domain_axiom_partitioning() const;

    /* Grounding */

    const FlatIndexList* get_or_create_index_list(const FlatIndexList& list);
    const FlatDoubleList* get_or_create_double_list(const FlatDoubleList& list);

    template<IsStaticOrFluentOrDerivedTag P>
    GroundAtom<P> get_or_create_ground_atom(Predicate<P> predicate, const ObjectList& objects);

    template<IsStaticOrFluentOrDerivedTag P>
    GroundLiteral<P> ground(Literal<P> literal, const ObjectList& binding);

    GroundFunctionExpression ground(FunctionExpression fexpr, const ObjectList& binding);

    GroundNumericConstraint ground(NumericConstraint numeric_constraint, const ObjectList& binding);

    template<IsFluentOrAuxiliaryTag F>
    GroundNumericEffect<F> ground(NumericEffect<F> numeric_effect, const ObjectList& binding);

    template<IsStaticOrFluentOrAuxiliaryTag F>
    GroundFunction<F> ground(Function<F> function, const ObjectList& binding);

    GroundConjunctiveCondition ground(ConjunctiveCondition conjunctive_condition, const ObjectList& binding);

    GroundConjunctiveEffect ground(ConjunctiveEffect conjunctive_effect, const ObjectList& binding);

    GroundConditionalEffect ground(ConditionalEffect conditional_effect, const ObjectList& binding);

    GroundAction ground(Action action, const ObjectList& binding);

    GroundAxiom ground(Axiom axiom, const ObjectList& binding);

    /* Lifting */

    Variable get_or_create_variable(std::string name, size_t parameter_index);

    Term get_or_create_term(Variable variable);
    Term get_or_create_term(Object object);

    template<IsStaticOrFluentOrDerivedTag P>
    Atom<P> get_or_create_atom(Predicate<P> predicate, TermList terms);

    template<IsStaticOrFluentOrDerivedTag P>
    Literal<P> get_or_create_literal(bool polarity, Atom<P> atom);

    template<IsStaticOrFluentOrAuxiliaryTag F>
    Function<F> get_or_create_function(FunctionSkeleton<F> function_skeleton, TermList terms, IndexList parent_terms_to_terms_mapping);

    FunctionExpressionNumber get_or_create_function_expression_number(double number);

    FunctionExpressionBinaryOperator get_or_create_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                       FunctionExpression left_function_expression,
                                                                                       FunctionExpression right_function_expression);

    FunctionExpressionMultiOperator get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                     FunctionExpressionList function_expressions_);

    FunctionExpressionMinus get_or_create_function_expression_minus(FunctionExpression function_expression);

    template<IsStaticOrFluentTag F>
    FunctionExpressionFunction<F> get_or_create_function_expression_function(Function<F> function);

    FunctionExpression get_or_create_function_expression(FunctionExpressionNumber fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionBinaryOperator fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionMultiOperator fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionMinus fexpr);
    template<IsStaticOrFluentTag F>
    FunctionExpression get_or_create_function_expression(FunctionExpressionFunction<F> fexpr);

    NumericConstraint get_or_create_numeric_constraint(loki::BinaryComparatorEnum binary_comparator,
                                                       FunctionExpression left_function_expression,
                                                       FunctionExpression right_function_expression,
                                                       TermList terms);

    ConjunctiveCondition get_or_create_conjunctive_condition(VariableList parameters,
                                                             LiteralLists<StaticTag, FluentTag, DerivedTag> literals,
                                                             NumericConstraintList numeric_constraints);

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const
    {
        return std::tuple(std::cref(get_domain()),
                          get_requirements(),
                          std::cref(get_objects()),
                          std::cref(get_derived_predicates()),
                          std::cref(get_initial_literals<StaticTag>()),
                          std::cref(get_initial_literals<FluentTag>()),
                          std::cref(get_initial_function_values<StaticTag>()),
                          std::cref(get_initial_function_values<FluentTag>()),
                          get_auxiliary_function_value(),
                          std::cref(get_goal_condition<StaticTag>()),
                          std::cref(get_goal_condition<FluentTag>()),
                          std::cref(get_goal_condition<DerivedTag>()),
                          get_numeric_goal_condition(),
                          get_optimization_metric(),
                          std::cref(get_axioms()));
    }
};

extern std::ostream& operator<<(std::ostream& out, const ProblemImpl& element);

extern std::ostream& operator<<(std::ostream& out, Problem element);

}

#endif
