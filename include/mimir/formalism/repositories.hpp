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

#ifndef MIMIR_COMMON_FACTORIES_HPP_
#define MIMIR_COMMON_FACTORIES_HPP_

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/conjunctive_condition.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/ground_effects.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_function_value.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_constraint.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"

#include <boost/hana.hpp>
#include <loki/loki.hpp>
#include <ranges>

namespace mimir
{

template<typename T>
using SegmentedPDDLRepository = loki::SegmentedRepository<T>;

using RequirementsRepository = SegmentedPDDLRepository<RequirementsImpl>;
using VariableRepository = SegmentedPDDLRepository<VariableImpl>;
using TermRepository = SegmentedPDDLRepository<TermImpl>;
using ObjectRepository = SegmentedPDDLRepository<ObjectImpl>;
template<StaticOrFluentOrDerived P>
using AtomRepository = SegmentedPDDLRepository<AtomImpl<P>>;
template<StaticOrFluentOrDerived P>
using GroundAtomRepository = SegmentedPDDLRepository<GroundAtomImpl<P>>;
template<StaticOrFluentOrDerived P>
using LiteralRepository = SegmentedPDDLRepository<LiteralImpl<P>>;
template<StaticOrFluentOrDerived P>
using GroundLiteralRepository = SegmentedPDDLRepository<GroundLiteralImpl<P>>;
template<StaticOrFluentOrDerived P>
using PredicateRepository = SegmentedPDDLRepository<PredicateImpl<P>>;
using FunctionExpressionNumberRepository = SegmentedPDDLRepository<FunctionExpressionNumberImpl>;
using FunctionExpressionBinaryOperatorRepository = SegmentedPDDLRepository<FunctionExpressionBinaryOperatorImpl>;
using FunctionExpressionMultiOperatorRepository = SegmentedPDDLRepository<FunctionExpressionMultiOperatorImpl>;
using FunctionExpressionMinusRepository = SegmentedPDDLRepository<FunctionExpressionMinusImpl>;
template<StaticOrFluentOrAuxiliary F>
using FunctionExpressionFunctionRepository = SegmentedPDDLRepository<FunctionExpressionFunctionImpl<F>>;
using FunctionExpressionRepository = SegmentedPDDLRepository<FunctionExpressionImpl>;
using GroundFunctionExpressionNumberRepository = SegmentedPDDLRepository<GroundFunctionExpressionNumberImpl>;
using GroundFunctionExpressionBinaryOperatorRepository = SegmentedPDDLRepository<GroundFunctionExpressionBinaryOperatorImpl>;
using GroundFunctionExpressionMultiOperatorRepository = SegmentedPDDLRepository<GroundFunctionExpressionMultiOperatorImpl>;
using GroundFunctionExpressionMinusRepository = SegmentedPDDLRepository<GroundFunctionExpressionMinusImpl>;
template<StaticOrFluentOrAuxiliary F>
using GroundFunctionExpressionFunctionRepository = SegmentedPDDLRepository<GroundFunctionExpressionFunctionImpl<F>>;
using GroundFunctionExpressionRepository = SegmentedPDDLRepository<GroundFunctionExpressionImpl>;
template<StaticOrFluentOrAuxiliary F>
using FunctionRepository = SegmentedPDDLRepository<FunctionImpl<F>>;
template<StaticOrFluentOrAuxiliary F>
using GroundFunctionRepository = SegmentedPDDLRepository<GroundFunctionImpl<F>>;
template<StaticOrFluentOrAuxiliary F>
using GroundFunctionValueRepository = SegmentedPDDLRepository<GroundFunctionValueImpl<F>>;
template<StaticOrFluentOrAuxiliary F>
using FunctionSkeletonRepository = SegmentedPDDLRepository<FunctionSkeletonImpl<F>>;
template<FluentOrAuxiliary F>
using NumericEffectRepository = SegmentedPDDLRepository<NumericEffectImpl<F>>;
template<FluentOrAuxiliary F>
using GroundNumericEffectRepository = SegmentedPDDLRepository<GroundNumericEffectImpl<F>>;
using ConjunctiveEffectRepository = SegmentedPDDLRepository<ConjunctiveEffectImpl>;
using ConditionalEffectRepository = SegmentedPDDLRepository<ConditionalEffectImpl>;
using NumericConstraintRepository = SegmentedPDDLRepository<NumericConstraintImpl>;
using GroundNumericConstraintRepository = SegmentedPDDLRepository<GroundNumericConstraintImpl>;
using ConjunctiveConditionRepository = SegmentedPDDLRepository<ConjunctiveConditionImpl>;
using ActionRepository = SegmentedPDDLRepository<ActionImpl>;
using AxiomRepository = SegmentedPDDLRepository<AxiomImpl>;
using OptimizationMetricRepository = SegmentedPDDLRepository<OptimizationMetricImpl>;

using HanaPDDLRepositories = boost::hana::map<
    boost::hana::pair<boost::hana::type<RequirementsImpl>, RequirementsRepository>,
    boost::hana::pair<boost::hana::type<VariableImpl>, VariableRepository>,
    boost::hana::pair<boost::hana::type<TermImpl>, TermRepository>,
    boost::hana::pair<boost::hana::type<ObjectImpl>, ObjectRepository>,
    boost::hana::pair<boost::hana::type<AtomImpl<Static>>, AtomRepository<Static>>,
    boost::hana::pair<boost::hana::type<AtomImpl<Fluent>>, AtomRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<AtomImpl<Derived>>, AtomRepository<Derived>>,
    boost::hana::pair<boost::hana::type<GroundAtomImpl<Static>>, GroundAtomRepository<Static>>,
    boost::hana::pair<boost::hana::type<GroundAtomImpl<Fluent>>, GroundAtomRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<GroundAtomImpl<Derived>>, GroundAtomRepository<Derived>>,
    boost::hana::pair<boost::hana::type<LiteralImpl<Static>>, LiteralRepository<Static>>,
    boost::hana::pair<boost::hana::type<LiteralImpl<Fluent>>, LiteralRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<LiteralImpl<Derived>>, LiteralRepository<Derived>>,
    boost::hana::pair<boost::hana::type<GroundLiteralImpl<Static>>, GroundLiteralRepository<Static>>,
    boost::hana::pair<boost::hana::type<GroundLiteralImpl<Fluent>>, GroundLiteralRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<GroundLiteralImpl<Derived>>, GroundLiteralRepository<Derived>>,
    boost::hana::pair<boost::hana::type<PredicateImpl<Static>>, PredicateRepository<Static>>,
    boost::hana::pair<boost::hana::type<PredicateImpl<Fluent>>, PredicateRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<PredicateImpl<Derived>>, PredicateRepository<Derived>>,
    boost::hana::pair<boost::hana::type<FunctionExpressionNumberImpl>, FunctionExpressionNumberRepository>,
    boost::hana::pair<boost::hana::type<FunctionExpressionBinaryOperatorImpl>, FunctionExpressionBinaryOperatorRepository>,
    boost::hana::pair<boost::hana::type<FunctionExpressionMultiOperatorImpl>, FunctionExpressionMultiOperatorRepository>,
    boost::hana::pair<boost::hana::type<FunctionExpressionMinusImpl>, FunctionExpressionMinusRepository>,
    boost::hana::pair<boost::hana::type<FunctionExpressionFunctionImpl<Static>>, FunctionExpressionFunctionRepository<Static>>,
    boost::hana::pair<boost::hana::type<FunctionExpressionFunctionImpl<Fluent>>, FunctionExpressionFunctionRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<FunctionExpressionFunctionImpl<Auxiliary>>, FunctionExpressionFunctionRepository<Auxiliary>>,
    boost::hana::pair<boost::hana::type<FunctionExpressionImpl>, FunctionExpressionRepository>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionNumberImpl>, GroundFunctionExpressionNumberRepository>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionBinaryOperatorImpl>, GroundFunctionExpressionBinaryOperatorRepository>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionMultiOperatorImpl>, GroundFunctionExpressionMultiOperatorRepository>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionMinusImpl>, GroundFunctionExpressionMinusRepository>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionFunctionImpl<Static>>, GroundFunctionExpressionFunctionRepository<Static>>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionFunctionImpl<Fluent>>, GroundFunctionExpressionFunctionRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionFunctionImpl<Auxiliary>>, GroundFunctionExpressionFunctionRepository<Auxiliary>>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionImpl>, GroundFunctionExpressionRepository>,
    boost::hana::pair<boost::hana::type<FunctionImpl<Static>>, FunctionRepository<Static>>,
    boost::hana::pair<boost::hana::type<FunctionImpl<Fluent>>, FunctionRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<FunctionImpl<Auxiliary>>, FunctionRepository<Auxiliary>>,
    boost::hana::pair<boost::hana::type<GroundFunctionImpl<Static>>, GroundFunctionRepository<Static>>,
    boost::hana::pair<boost::hana::type<GroundFunctionImpl<Fluent>>, GroundFunctionRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<GroundFunctionImpl<Auxiliary>>, GroundFunctionRepository<Auxiliary>>,
    boost::hana::pair<boost::hana::type<GroundFunctionValueImpl<Static>>, GroundFunctionValueRepository<Static>>,
    boost::hana::pair<boost::hana::type<GroundFunctionValueImpl<Fluent>>, GroundFunctionValueRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<GroundFunctionValueImpl<Auxiliary>>, GroundFunctionValueRepository<Auxiliary>>,
    boost::hana::pair<boost::hana::type<FunctionSkeletonImpl<Static>>, FunctionSkeletonRepository<Static>>,
    boost::hana::pair<boost::hana::type<FunctionSkeletonImpl<Fluent>>, FunctionSkeletonRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<FunctionSkeletonImpl<Auxiliary>>, FunctionSkeletonRepository<Auxiliary>>,
    boost::hana::pair<boost::hana::type<NumericEffectImpl<Fluent>>, NumericEffectRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<NumericEffectImpl<Auxiliary>>, NumericEffectRepository<Auxiliary>>,
    boost::hana::pair<boost::hana::type<GroundNumericEffectImpl<Fluent>>, GroundNumericEffectRepository<Fluent>>,
    boost::hana::pair<boost::hana::type<GroundNumericEffectImpl<Auxiliary>>, GroundNumericEffectRepository<Auxiliary>>,
    boost::hana::pair<boost::hana::type<ConjunctiveEffectImpl>, ConjunctiveEffectRepository>,
    boost::hana::pair<boost::hana::type<ConditionalEffectImpl>, ConditionalEffectRepository>,
    boost::hana::pair<boost::hana::type<NumericConstraintImpl>, NumericConstraintRepository>,
    boost::hana::pair<boost::hana::type<GroundNumericConstraintImpl>, GroundNumericConstraintRepository>,
    boost::hana::pair<boost::hana::type<ConjunctiveConditionImpl>, ConjunctiveConditionRepository>,
    boost::hana::pair<boost::hana::type<ActionImpl>, ActionRepository>,
    boost::hana::pair<boost::hana::type<AxiomImpl>, AxiomRepository>,
    boost::hana::pair<boost::hana::type<OptimizationMetricImpl>, OptimizationMetricRepository>>;

/// @brief Collection of factories for the unique creation of PDDL objects.
class PDDLRepositories
{
private:
    HanaPDDLRepositories m_repositories;

public:
    PDDLRepositories() = default;

    // delete copy and allow move
    PDDLRepositories(const PDDLRepositories& other) = delete;
    PDDLRepositories& operator=(const PDDLRepositories& other) = delete;
    PDDLRepositories(PDDLRepositories&& other) = default;
    PDDLRepositories& operator=(PDDLRepositories&& other) = default;

    HanaPDDLRepositories& get_hana_repositories();
    const HanaPDDLRepositories& get_hana_repositories() const;

    ///////////////////////////////////////////////////////////////////////////
    /// Modifiers
    ///////////////////////////////////////////////////////////////////////////

    /// @brief Get or create requriements for the given parameters.
    Requirements get_or_create_requirements(loki::RequirementEnumSet requirement_set);

    /// @brief Get or create a variable for the given parameters.
    Variable get_or_create_variable(std::string name, size_t parameter_index);

    /// @brief Get or create a variable term for the given parameters.
    Term get_or_create_term(Variable variable);
    Term get_or_create_term(Object object);

    /// @brief Get or create an object for the given parameters.
    Object get_or_create_object(std::string name);

    template<StaticOrFluentOrDerived P>
    Atom<P> get_or_create_atom(Predicate<P> predicate, TermList terms);

    template<StaticOrFluentOrDerived P>
    GroundAtom<P> get_or_create_ground_atom(Predicate<P> predicate, ObjectList objects);

    template<StaticOrFluentOrDerived P>
    Literal<P> get_or_create_literal(bool is_negated, Atom<P> atom);

    template<StaticOrFluentOrDerived P>
    GroundLiteral<P> get_or_create_ground_literal(bool is_negated, GroundAtom<P> atom);

    template<StaticOrFluentOrDerived P>
    Predicate<P> get_or_create_predicate(std::string name, VariableList parameters);

    /// @brief Get or create a number function expression for the given parameters.
    FunctionExpressionNumber get_or_create_function_expression_number(double number);

    /// @brief Get or create a binary operator function expression for the given parameters.
    FunctionExpressionBinaryOperator get_or_create_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                       FunctionExpression left_function_expression,
                                                                                       FunctionExpression right_function_expression);

    /// @brief Get or create a multi operator function expression for the given parameters.
    FunctionExpressionMultiOperator get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                     FunctionExpressionList function_expressions_);

    /// @brief Get or create a minus function expression for the given parameters.
    FunctionExpressionMinus get_or_create_function_expression_minus(FunctionExpression function_expression);

    /// @brief Get or create a function function expression for the given parameters.
    template<StaticOrFluent F>
    FunctionExpressionFunction<F> get_or_create_function_expression_function(Function<F> function);

    /// @brief Get or create a function expression for the given parameters.
    FunctionExpression get_or_create_function_expression(FunctionExpressionNumber fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionBinaryOperator fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionMultiOperator fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionMinus fexpr);
    template<StaticOrFluent F>
    FunctionExpression get_or_create_function_expression(FunctionExpressionFunction<F> fexpr);

    /// @brief Get or create a number function expression for the given parameters.
    GroundFunctionExpressionNumber get_or_create_ground_function_expression_number(double number);

    /// @brief Get or create a binary operator function expression for the given parameters.
    GroundFunctionExpressionBinaryOperator get_or_create_ground_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                                    GroundFunctionExpression left_function_expression,
                                                                                                    GroundFunctionExpression right_function_expression);

    /// @brief Get or create a multi operator function expression for the given parameters.
    GroundFunctionExpressionMultiOperator get_or_create_ground_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                                  GroundFunctionExpressionList function_expressions_);

    /// @brief Get or create a minus function expression for the given parameters.
    GroundFunctionExpressionMinus get_or_create_ground_function_expression_minus(GroundFunctionExpression function_expression);

    /// @brief Get or create a function function expression for the given parameters.
    template<StaticOrFluentOrAuxiliary F>
    GroundFunctionExpressionFunction<F> get_or_create_ground_function_expression_function(GroundFunction<F> function);

    /// @brief Get or create a function expression for the given parameters.
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionNumber fexpr);
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionBinaryOperator fexpr);
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionMultiOperator fexpr);
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionMinus fexpr);
    template<StaticOrFluentOrAuxiliary F>
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionFunction<F> fexpr);

    /// @brief Get or create a function for the given parameters.
    template<StaticOrFluentOrAuxiliary F>
    Function<F> get_or_create_function(FunctionSkeleton<F> function_skeleton, TermList terms, IndexList m_parent_terms_to_terms_mapping);

    /// @brief Get or create a function for the given parameters.
    template<StaticOrFluentOrAuxiliary F>
    GroundFunction<F> get_or_create_ground_function(FunctionSkeleton<F> function_skeleton, ObjectList objects);

    /// @brief Get or create a function skeleton for the given parameters.
    template<StaticOrFluentOrAuxiliary F>
    FunctionSkeleton<F> get_or_create_function_skeleton(std::string name, VariableList parameters);

    /// @brief Get or create a numeric effect for the given parameters.
    template<FluentOrAuxiliary F>
    NumericEffect<F> get_or_create_numeric_effect(loki::AssignOperatorEnum assign_operator, Function<F> function, FunctionExpression function_expression);

    template<FluentOrAuxiliary F>
    GroundNumericEffect<F>
    get_or_create_ground_numeric_effect(loki::AssignOperatorEnum assign_operator, GroundFunction<F> function, GroundFunctionExpression function_expression);

    /// @brief Get or create a simple effect for the given parameters.
    ConjunctiveEffect get_or_create_conjunctive_effect(VariableList parameters,
                                                       LiteralList<Fluent> effects,
                                                       NumericEffectList<Fluent> fluent_numeric_effects,
                                                       std::optional<NumericEffect<Auxiliary>> auxiliary_numeric_effect);

    /// @brief Get or create a universal conditional simple effect for the given parameters.
    ConditionalEffect get_or_create_conditional_effect(ConjunctiveCondition conjunctive_condition, ConjunctiveEffect conjunctive_effect);

    /// @brief Get or create a numeric constraint for the given parameters.
    NumericConstraint get_or_create_numeric_constraint(loki::BinaryComparatorEnum binary_comparator,
                                                       FunctionExpression function_expression_left,
                                                       FunctionExpression function_expression_right,
                                                       TermList terms);

    /// @brief Get or create a numeric constraint for the given parameters.
    GroundNumericConstraint get_or_create_ground_numeric_constraint(loki::BinaryComparatorEnum binary_comparator,
                                                                    GroundFunctionExpression function_expression_left,
                                                                    GroundFunctionExpression function_expression_right);

    /// @brief Get or create a existentially quantified conjunctive condition for the given parameters.
    ConjunctiveCondition
    get_or_create_conjunctive_condition(VariableList parameters, LiteralLists<Static, Fluent, Derived> literals, NumericConstraintList numeric_constraints);

    /// @brief Get or create an action for the given parameters.
    Action get_or_create_action(std::string name,
                                size_t original_arity,
                                ConjunctiveCondition conjunctive_condition,
                                ConjunctiveEffect conjunctive_effect,
                                ConditionalEffectList conditional_effects);

    /// @brief Get or create a derived predicate for the given parameters.
    Axiom get_or_create_axiom(ConjunctiveCondition precondition, Literal<Derived> effect_literal);

    /// @brief Get or create an optimization metric for the given parameters.
    OptimizationMetric get_or_create_optimization_metric(loki::OptimizationMetricEnum metric, GroundFunctionExpression function_expression);

    /// @brief Get or create an optimization metric for the given parameters.
    template<StaticOrFluentOrAuxiliary F>
    GroundFunctionValue<F> get_or_create_ground_function_value(GroundFunction<F> function, double number);

    ///////////////////////////////////////////////////////////////////////////
    /// Accessors
    ///////////////////////////////////////////////////////////////////////////

    // GroundNumericConstraint
    GroundNumericConstraint get_ground_numeric_constraint(size_t numeric_constraint_index) const;

    template<std::ranges::forward_range Iterable>
    void get_ground_numeric_constraints_from_indices(const Iterable& numeric_constraint_indices,
                                                     GroundNumericConstraintList& out_ground_numeric_constraints) const;

    template<std::ranges::forward_range Iterable>
    GroundNumericConstraintList get_ground_numeric_constraints_from_indices(const Iterable& numeric_constraint_indices) const;

    // GroundNumericEffect
    template<FluentOrAuxiliary F>
    GroundNumericEffect<F> get_ground_numeric_effect(size_t numeric_effect_index) const;

    template<FluentOrAuxiliary F, std::ranges::forward_range Iterable>
    void get_ground_numeric_effects_from_indices(const Iterable& numeric_effect_indices, GroundNumericEffectList<F>& out_ground_numeric_effect) const;

    template<FluentOrAuxiliary F, std::ranges::forward_range Iterable>
    GroundNumericEffectList<F> get_ground_numeric_effects_from_indices(const Iterable& numeric_effect_indices) const;

    // GroundAtom
    template<StaticOrFluentOrDerived P>
    GroundAtom<P> get_ground_atom(size_t atom_index) const;

    template<StaticOrFluentOrDerived P, std::ranges::forward_range Iterable>
    void get_ground_atoms_from_indices(const Iterable& atom_indices, GroundAtomList<P>& out_ground_atoms) const;

    template<StaticOrFluentOrDerived P, std::ranges::forward_range Iterable>
    GroundAtomList<P> get_ground_atoms_from_indices(const Iterable& atom_indices) const;

    template<StaticOrFluentOrDerived P>
    void get_ground_atoms(GroundAtomList<P>& out_ground_atoms) const;

    template<StaticOrFluentOrDerived P>
    auto get_ground_atoms() const;

    // GroundFunction
    template<StaticOrFluentOrAuxiliary F>
    void get_ground_functions(size_t num_ground_functions, GroundFunctionList<F>& out_ground_functions) const;

    template<StaticOrFluentOrAuxiliary F>
    void get_ground_function_values(const FlatDoubleList& values, std::vector<std::pair<GroundFunction<F>, ContinuousCost>>& out_ground_function_values) const;

    template<StaticOrFluentOrAuxiliary F>
    std::vector<std::pair<GroundFunction<F>, ContinuousCost>> get_ground_function_values(const FlatDoubleList& values) const;

    // Object
    Object get_object(size_t object_index) const;

    template<std::ranges::forward_range Iterable>
    void get_objects_from_indices(const Iterable& object_indices, ObjectList& out_objects) const;

    template<std::ranges::forward_range Iterable>
    ObjectList get_objects_from_indices(const Iterable& object_indices) const;

    // Action
    Action get_action(size_t action_index) const;

    // Axiom
    Axiom get_axiom(size_t axiom_index) const;
};

/**
 * Implementations
 */

// GroundNumericConstraint
template<std::ranges::forward_range Iterable>
void PDDLRepositories::get_ground_numeric_constraints_from_indices(const Iterable& numeric_constraint_indices,
                                                                   GroundNumericConstraintList& out_ground_numeric_constraints) const
{
    out_ground_numeric_constraints.clear();

    for (const auto& index : numeric_constraint_indices)
    {
        out_ground_numeric_constraints.push_back(get_ground_numeric_constraint(index));
    }
}

template<std::ranges::forward_range Iterable>
GroundNumericConstraintList PDDLRepositories::get_ground_numeric_constraints_from_indices(const Iterable& numeric_constraint_indices) const
{
    auto result = GroundNumericConstraintList {};
    get_ground_numeric_constraints_from_indices(numeric_constraint_indices, result);
    return result;
}

// GroundNumericEffect
template<FluentOrAuxiliary F, std::ranges::forward_range Iterable>
void PDDLRepositories::get_ground_numeric_effects_from_indices(const Iterable& numeric_effect_indices,
                                                               GroundNumericEffectList<F>& out_ground_numeric_effect) const
{
    out_ground_numeric_effect.clear();

    for (const auto& index : numeric_effect_indices)
    {
        out_ground_numeric_effect.push_back(get_ground_numeric_effect<F>(index));
    }
}

template<FluentOrAuxiliary F, std::ranges::forward_range Iterable>
GroundNumericEffectList<F> PDDLRepositories::get_ground_numeric_effects_from_indices(const Iterable& numeric_effect_indices) const
{
    auto result = GroundNumericEffectList<F> {};
    get_ground_numeric_effects_from_indices(numeric_effect_indices, result);
    return result;
}

// Atom
template<StaticOrFluentOrDerived P, std::ranges::forward_range Iterable>
void PDDLRepositories::get_ground_atoms_from_indices(const Iterable& atom_indices, GroundAtomList<P>& out_ground_atoms) const
{
    out_ground_atoms.clear();

    for (const auto& index : atom_indices)
    {
        out_ground_atoms.push_back(get_ground_atom<P>(index));
    }
}

template<StaticOrFluentOrDerived P, std::ranges::forward_range Iterable>
GroundAtomList<P> PDDLRepositories::get_ground_atoms_from_indices(const Iterable& atom_indices) const
{
    auto result = GroundAtomList<P> {};
    get_ground_atoms_from_indices(atom_indices, result);
    return result;
}

template<StaticOrFluentOrDerived P>
void PDDLRepositories::get_ground_atoms(GroundAtomList<P>& out_ground_atoms) const
{
    out_ground_atoms.clear();
    for (const auto& atom : boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {}))
    {
        out_ground_atoms.push_back(atom);
    }
}

template<StaticOrFluentOrDerived P>
auto PDDLRepositories::get_ground_atoms() const
{
    const auto& factory = boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {});
    return std::ranges::subrange(factory.begin(), factory.end());
}

template<std::ranges::forward_range Iterable>
void PDDLRepositories::get_objects_from_indices(const Iterable& object_indices, ObjectList& out_objects) const
{
    out_objects.clear();
    for (const auto& object_index : object_indices)
    {
        out_objects.push_back(get_object(object_index));
    }
}

template<std::ranges::forward_range Iterable>
ObjectList PDDLRepositories::get_objects_from_indices(const Iterable& object_indices) const
{
    auto objects = ObjectList {};
    get_objects_from_indices(object_indices, objects);
    return objects;
}

}

#endif
