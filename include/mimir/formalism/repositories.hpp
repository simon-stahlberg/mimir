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

#ifndef MIMIR_FORMALISM_REPOSITORIES_HPP_
#define MIMIR_FORMALISM_REPOSITORIES_HPP_

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
#include "mimir/formalism/ground_conjunctive_condition.hpp"
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
#include "mimir/formalism/parameter.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/type.hpp"
#include "mimir/formalism/variable.hpp"

#include <boost/hana.hpp>
#include <loki/loki.hpp>
#include <ranges>

namespace mimir::formalism
{

using RequirementsRepository = loki::IndexedHashSet<RequirementsImpl>;
using TypeRepository = loki::IndexedHashSet<TypeImpl>;
using VariableRepository = loki::IndexedHashSet<VariableImpl>;
using ParameterRepository = loki::IndexedHashSet<ParameterImpl>;
using TermRepository = loki::IndexedHashSet<TermImpl>;
using ObjectRepository = loki::IndexedHashSet<ObjectImpl>;
template<IsStaticOrFluentOrDerivedTag P>
using AtomRepository = loki::IndexedHashSet<AtomImpl<P>>;
template<IsStaticOrFluentOrDerivedTag P>
using GroundAtomRepository = loki::IndexedHashSet<GroundAtomImpl<P>>;
template<IsStaticOrFluentOrDerivedTag P>
using LiteralRepository = loki::IndexedHashSet<LiteralImpl<P>>;
template<IsStaticOrFluentOrDerivedTag P>
using GroundLiteralRepository = loki::IndexedHashSet<GroundLiteralImpl<P>>;
template<IsStaticOrFluentOrDerivedTag P>
using PredicateRepository = loki::IndexedHashSet<PredicateImpl<P>>;
using FunctionExpressionNumberRepository = loki::IndexedHashSet<FunctionExpressionNumberImpl>;
using FunctionExpressionBinaryOperatorRepository = loki::IndexedHashSet<FunctionExpressionBinaryOperatorImpl>;
using FunctionExpressionMultiOperatorRepository = loki::IndexedHashSet<FunctionExpressionMultiOperatorImpl>;
using FunctionExpressionMinusRepository = loki::IndexedHashSet<FunctionExpressionMinusImpl>;
template<IsStaticOrFluentOrAuxiliaryTag F>
using FunctionExpressionFunctionRepository = loki::IndexedHashSet<FunctionExpressionFunctionImpl<F>>;
using FunctionExpressionRepository = loki::IndexedHashSet<FunctionExpressionImpl>;
using GroundFunctionExpressionNumberRepository = loki::IndexedHashSet<GroundFunctionExpressionNumberImpl>;
using GroundFunctionExpressionBinaryOperatorRepository = loki::IndexedHashSet<GroundFunctionExpressionBinaryOperatorImpl>;
using GroundFunctionExpressionMultiOperatorRepository = loki::IndexedHashSet<GroundFunctionExpressionMultiOperatorImpl>;
using GroundFunctionExpressionMinusRepository = loki::IndexedHashSet<GroundFunctionExpressionMinusImpl>;
template<IsStaticOrFluentOrAuxiliaryTag F>
using GroundFunctionExpressionFunctionRepository = loki::IndexedHashSet<GroundFunctionExpressionFunctionImpl<F>>;
using GroundFunctionExpressionRepository = loki::IndexedHashSet<GroundFunctionExpressionImpl>;
template<IsStaticOrFluentOrAuxiliaryTag F>
using FunctionRepository = loki::IndexedHashSet<FunctionImpl<F>>;
template<IsStaticOrFluentOrAuxiliaryTag F>
using GroundFunctionRepository = loki::IndexedHashSet<GroundFunctionImpl<F>>;
template<IsStaticOrFluentOrAuxiliaryTag F>
using GroundFunctionValueRepository = loki::IndexedHashSet<GroundFunctionValueImpl<F>>;
template<IsStaticOrFluentOrAuxiliaryTag F>
using FunctionSkeletonRepository = loki::IndexedHashSet<FunctionSkeletonImpl<F>>;
template<IsFluentOrAuxiliaryTag F>
using NumericEffectRepository = loki::IndexedHashSet<NumericEffectImpl<F>>;
template<IsFluentOrAuxiliaryTag F>
using GroundNumericEffectRepository = loki::IndexedHashSet<GroundNumericEffectImpl<F>>;
using ConjunctiveEffectRepository = loki::IndexedHashSet<ConjunctiveEffectImpl>;
using GroundConjunctiveEffectRepository = loki::IndexedHashSet<GroundConjunctiveEffectImpl>;
using ConditionalEffectRepository = loki::IndexedHashSet<ConditionalEffectImpl>;
using GroundConditionalEffectRepository = loki::IndexedHashSet<GroundConditionalEffectImpl>;
using NumericConstraintRepository = loki::IndexedHashSet<NumericConstraintImpl>;
using GroundNumericConstraintRepository = loki::IndexedHashSet<GroundNumericConstraintImpl>;
using ConjunctiveConditionRepository = loki::IndexedHashSet<ConjunctiveConditionImpl>;
using GroundConjunctiveConditionRepository = loki::IndexedHashSet<GroundConjunctiveConditionImpl>;
using ActionRepository = loki::IndexedHashSet<ActionImpl>;
using GroundActionRepository = loki::IndexedHashSet<GroundActionImpl>;
using AxiomRepository = loki::IndexedHashSet<AxiomImpl>;
using GroundAxiomRepository = loki::IndexedHashSet<GroundAxiomImpl>;
using OptimizationMetricRepository = loki::IndexedHashSet<OptimizationMetricImpl>;

using HanaRepositories = boost::hana::map<
    boost::hana::pair<boost::hana::type<RequirementsImpl>, RequirementsRepository>,
    boost::hana::pair<boost::hana::type<TypeImpl>, TypeRepository>,
    boost::hana::pair<boost::hana::type<VariableImpl>, VariableRepository>,
    boost::hana::pair<boost::hana::type<ParameterImpl>, ParameterRepository>,
    boost::hana::pair<boost::hana::type<TermImpl>, TermRepository>,
    boost::hana::pair<boost::hana::type<ObjectImpl>, ObjectRepository>,
    boost::hana::pair<boost::hana::type<AtomImpl<StaticTag>>, AtomRepository<StaticTag>>,
    boost::hana::pair<boost::hana::type<AtomImpl<FluentTag>>, AtomRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<AtomImpl<DerivedTag>>, AtomRepository<DerivedTag>>,
    boost::hana::pair<boost::hana::type<GroundAtomImpl<StaticTag>>, GroundAtomRepository<StaticTag>>,
    boost::hana::pair<boost::hana::type<GroundAtomImpl<FluentTag>>, GroundAtomRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<GroundAtomImpl<DerivedTag>>, GroundAtomRepository<DerivedTag>>,
    boost::hana::pair<boost::hana::type<LiteralImpl<StaticTag>>, LiteralRepository<StaticTag>>,
    boost::hana::pair<boost::hana::type<LiteralImpl<FluentTag>>, LiteralRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<LiteralImpl<DerivedTag>>, LiteralRepository<DerivedTag>>,
    boost::hana::pair<boost::hana::type<GroundLiteralImpl<StaticTag>>, GroundLiteralRepository<StaticTag>>,
    boost::hana::pair<boost::hana::type<GroundLiteralImpl<FluentTag>>, GroundLiteralRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<GroundLiteralImpl<DerivedTag>>, GroundLiteralRepository<DerivedTag>>,
    boost::hana::pair<boost::hana::type<PredicateImpl<StaticTag>>, PredicateRepository<StaticTag>>,
    boost::hana::pair<boost::hana::type<PredicateImpl<FluentTag>>, PredicateRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<PredicateImpl<DerivedTag>>, PredicateRepository<DerivedTag>>,
    boost::hana::pair<boost::hana::type<FunctionExpressionNumberImpl>, FunctionExpressionNumberRepository>,
    boost::hana::pair<boost::hana::type<FunctionExpressionBinaryOperatorImpl>, FunctionExpressionBinaryOperatorRepository>,
    boost::hana::pair<boost::hana::type<FunctionExpressionMultiOperatorImpl>, FunctionExpressionMultiOperatorRepository>,
    boost::hana::pair<boost::hana::type<FunctionExpressionMinusImpl>, FunctionExpressionMinusRepository>,
    boost::hana::pair<boost::hana::type<FunctionExpressionFunctionImpl<StaticTag>>, FunctionExpressionFunctionRepository<StaticTag>>,
    boost::hana::pair<boost::hana::type<FunctionExpressionFunctionImpl<FluentTag>>, FunctionExpressionFunctionRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<FunctionExpressionFunctionImpl<AuxiliaryTag>>, FunctionExpressionFunctionRepository<AuxiliaryTag>>,
    boost::hana::pair<boost::hana::type<FunctionExpressionImpl>, FunctionExpressionRepository>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionNumberImpl>, GroundFunctionExpressionNumberRepository>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionBinaryOperatorImpl>, GroundFunctionExpressionBinaryOperatorRepository>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionMultiOperatorImpl>, GroundFunctionExpressionMultiOperatorRepository>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionMinusImpl>, GroundFunctionExpressionMinusRepository>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionFunctionImpl<StaticTag>>, GroundFunctionExpressionFunctionRepository<StaticTag>>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionFunctionImpl<FluentTag>>, GroundFunctionExpressionFunctionRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionFunctionImpl<AuxiliaryTag>>, GroundFunctionExpressionFunctionRepository<AuxiliaryTag>>,
    boost::hana::pair<boost::hana::type<GroundFunctionExpressionImpl>, GroundFunctionExpressionRepository>,
    boost::hana::pair<boost::hana::type<FunctionImpl<StaticTag>>, FunctionRepository<StaticTag>>,
    boost::hana::pair<boost::hana::type<FunctionImpl<FluentTag>>, FunctionRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<FunctionImpl<AuxiliaryTag>>, FunctionRepository<AuxiliaryTag>>,
    boost::hana::pair<boost::hana::type<GroundFunctionImpl<StaticTag>>, GroundFunctionRepository<StaticTag>>,
    boost::hana::pair<boost::hana::type<GroundFunctionImpl<FluentTag>>, GroundFunctionRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<GroundFunctionImpl<AuxiliaryTag>>, GroundFunctionRepository<AuxiliaryTag>>,
    boost::hana::pair<boost::hana::type<GroundFunctionValueImpl<StaticTag>>, GroundFunctionValueRepository<StaticTag>>,
    boost::hana::pair<boost::hana::type<GroundFunctionValueImpl<FluentTag>>, GroundFunctionValueRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<GroundFunctionValueImpl<AuxiliaryTag>>, GroundFunctionValueRepository<AuxiliaryTag>>,
    boost::hana::pair<boost::hana::type<FunctionSkeletonImpl<StaticTag>>, FunctionSkeletonRepository<StaticTag>>,
    boost::hana::pair<boost::hana::type<FunctionSkeletonImpl<FluentTag>>, FunctionSkeletonRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<FunctionSkeletonImpl<AuxiliaryTag>>, FunctionSkeletonRepository<AuxiliaryTag>>,
    boost::hana::pair<boost::hana::type<NumericEffectImpl<FluentTag>>, NumericEffectRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<NumericEffectImpl<AuxiliaryTag>>, NumericEffectRepository<AuxiliaryTag>>,
    boost::hana::pair<boost::hana::type<GroundNumericEffectImpl<FluentTag>>, GroundNumericEffectRepository<FluentTag>>,
    boost::hana::pair<boost::hana::type<GroundNumericEffectImpl<AuxiliaryTag>>, GroundNumericEffectRepository<AuxiliaryTag>>,
    boost::hana::pair<boost::hana::type<ConjunctiveEffectImpl>, ConjunctiveEffectRepository>,
    boost::hana::pair<boost::hana::type<GroundConjunctiveEffectImpl>, GroundConjunctiveEffectRepository>,
    boost::hana::pair<boost::hana::type<ConditionalEffectImpl>, ConditionalEffectRepository>,
    boost::hana::pair<boost::hana::type<GroundConditionalEffectImpl>, GroundConditionalEffectRepository>,
    boost::hana::pair<boost::hana::type<NumericConstraintImpl>, NumericConstraintRepository>,
    boost::hana::pair<boost::hana::type<GroundNumericConstraintImpl>, GroundNumericConstraintRepository>,
    boost::hana::pair<boost::hana::type<ConjunctiveConditionImpl>, ConjunctiveConditionRepository>,
    boost::hana::pair<boost::hana::type<GroundConjunctiveConditionImpl>, GroundConjunctiveConditionRepository>,
    boost::hana::pair<boost::hana::type<ActionImpl>, ActionRepository>,
    boost::hana::pair<boost::hana::type<GroundActionImpl>, GroundActionRepository>,
    boost::hana::pair<boost::hana::type<AxiomImpl>, AxiomRepository>,
    boost::hana::pair<boost::hana::type<GroundAxiomImpl>, GroundAxiomRepository>,
    boost::hana::pair<boost::hana::type<OptimizationMetricImpl>, OptimizationMetricRepository>>;

/// @brief `PDDLRepositories` encapsulates repositories for the unique instantiation of PDDL formalism related structures.
class Repositories
{
private:
    HanaRepositories m_repositories;

public:
    explicit Repositories(const Repositories* parent = nullptr);

    // delete copy and allow move
    Repositories(const Repositories& other) = delete;
    Repositories& operator=(const Repositories& other) = delete;
    Repositories(Repositories&& other) = default;
    Repositories& operator=(Repositories&& other) = default;

    HanaRepositories& get_hana_repositories();
    const HanaRepositories& get_hana_repositories() const;

    ///////////////////////////////////////////////////////////////////////////
    /// Grounding
    ///////////////////////////////////////////////////////////////////////////

    template<IsStaticOrFluentOrDerivedTag P>
    GroundLiteral<P> ground(Literal<P> literal, const ObjectList& binding);

    GroundFunctionExpression ground(FunctionExpression fexpr, const ObjectList& binding);

    template<IsStaticOrFluentOrAuxiliaryTag F>
    GroundFunction<F> ground(Function<F> function, const ObjectList& binding);

    GroundNumericConstraint ground(NumericConstraint numeric_constraint, const ObjectList& binding);

    template<IsFluentOrAuxiliaryTag F>
    GroundNumericEffect<F> ground(NumericEffect<F> numeric_effect, const ObjectList& binding);

    ///////////////////////////////////////////////////////////////////////////
    /// Modifiers
    ///////////////////////////////////////////////////////////////////////////

    /// @brief Get or create requriements for the given parameters.
    Requirements get_or_create_requirements(loki::RequirementEnumSet requirement_set);

    /// @brief Get or create a type for the given parameters.
    Type get_or_create_type(std::string name, TypeList bases);

    /// @brief Get or create a variable for the given parameters.
    Variable get_or_create_variable(std::string name, size_t parameter_index);

    /// @brief Get or create a parameter for the given parameters.
    Parameter get_or_create_parameter(Variable variable, TypeList types);

    /// @brief Get or create a variable term for the given parameters.
    Term get_or_create_term(Variable variable);
    Term get_or_create_term(Object object);

    /// @brief Get or create an object for the given parameters.
    Object get_or_create_object(std::string name, TypeList types);

    template<IsStaticOrFluentOrDerivedTag P>
    Atom<P> get_or_create_atom(Predicate<P> predicate, TermList terms);

    template<IsStaticOrFluentOrDerivedTag P>
    GroundAtom<P> get_or_create_ground_atom(Predicate<P> predicate, const ObjectList& binding);

    template<IsStaticOrFluentOrDerivedTag P>
    Literal<P> get_or_create_literal(bool polarity, Atom<P> atom);

    template<IsStaticOrFluentOrDerivedTag P>
    GroundLiteral<P> get_or_create_ground_literal(bool polarity, GroundAtom<P> atom);

    template<IsStaticOrFluentOrDerivedTag P>
    Predicate<P> get_or_create_predicate(std::string name, ParameterList parameters);

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
    template<IsStaticOrFluentTag F>
    FunctionExpressionFunction<F> get_or_create_function_expression_function(Function<F> function);

    /// @brief Get or create a function expression for the given parameters.
    FunctionExpression get_or_create_function_expression(FunctionExpressionNumber fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionBinaryOperator fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionMultiOperator fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionMinus fexpr);
    template<IsStaticOrFluentTag F>
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
    template<IsStaticOrFluentOrAuxiliaryTag F>
    GroundFunctionExpressionFunction<F> get_or_create_ground_function_expression_function(GroundFunction<F> function);

    /// @brief Get or create a function expression for the given parameters.
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionNumber fexpr);
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionBinaryOperator fexpr);
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionMultiOperator fexpr);
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionMinus fexpr);
    template<IsStaticOrFluentOrAuxiliaryTag F>
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionFunction<F> fexpr);

    /// @brief Get or create a function for the given parameters.
    template<IsStaticOrFluentOrAuxiliaryTag F>
    Function<F> get_or_create_function(FunctionSkeleton<F> function_skeleton, TermList terms);

    /// @brief Get or create a function for the given parameters.
    template<IsStaticOrFluentOrAuxiliaryTag F>
    GroundFunction<F> get_or_create_ground_function(FunctionSkeleton<F> function_skeleton, const ObjectList& binding);

    /// @brief Get or create a function skeleton for the given parameters.
    template<IsStaticOrFluentOrAuxiliaryTag F>
    FunctionSkeleton<F> get_or_create_function_skeleton(std::string name, ParameterList parameters);

    /// @brief Get or create a numeric effect for the given parameters.
    template<IsFluentOrAuxiliaryTag F>
    NumericEffect<F> get_or_create_numeric_effect(loki::AssignOperatorEnum assign_operator, Function<F> function, FunctionExpression function_expression);

    template<IsFluentOrAuxiliaryTag F>
    GroundNumericEffect<F>
    get_or_create_ground_numeric_effect(loki::AssignOperatorEnum assign_operator, GroundFunction<F> function, GroundFunctionExpression function_expression);

    /// @brief Get or create a universally quantified conjunctive effect for the given parameters.
    ConjunctiveEffect get_or_create_conjunctive_effect(ParameterList parameters,
                                                       LiteralList<FluentTag> effects,
                                                       NumericEffectList<FluentTag> fluent_numeric_effects,
                                                       std::optional<NumericEffect<AuxiliaryTag>> auxiliary_numeric_effect);

    /// @brief Get or create a ground conjunctive effect for the given parameters.
    GroundConjunctiveEffect get_or_create_ground_conjunctive_effect(HanaContainer<const FlatIndexList*, PositiveTag, NegativeTag> propositional_effects,
                                                                    GroundNumericEffectList<FluentTag> fluent_numeric_effects,
                                                                    std::optional<const GroundNumericEffect<AuxiliaryTag>> auxiliary_numeric_effect);

    /// @brief Get or create a conditional effect for the given parameters.
    ConditionalEffect get_or_create_conditional_effect(ConjunctiveCondition conjunctive_condition, ConjunctiveEffect conjunctive_effect);

    /// @brief Get or create a ground conditional effect for the given parameters.
    GroundConditionalEffect get_or_create_ground_conditional_effect(GroundConjunctiveCondition conjunctive_condition,
                                                                    GroundConjunctiveEffect conjunctive_effect);

    /// @brief Get or create a numeric constraint for the given parameters.
    NumericConstraint get_or_create_numeric_constraint(loki::BinaryComparatorEnum binary_comparator,
                                                       FunctionExpression left_function_expression,
                                                       FunctionExpression right_function_expression,
                                                       TermList terms);

    /// @brief Get or create a numeric constraint for the given parameters.
    GroundNumericConstraint get_or_create_ground_numeric_constraint(loki::BinaryComparatorEnum binary_comparator,
                                                                    GroundFunctionExpression left_function_expression,
                                                                    GroundFunctionExpression right_function_expression);

    /// @brief Get or create a existentially quantified conjunctive condition for the given parameters.
    ConjunctiveCondition get_or_create_conjunctive_condition(ParameterList parameters,
                                                             LiteralLists<StaticTag, FluentTag, DerivedTag> literals,
                                                             NumericConstraintList numeric_constraints);

    /// @brief Get or create a ground conjunctive condition for the given parameters.
    GroundConjunctiveCondition get_or_create_ground_conjunctive_condition(
        HanaContainer<HanaContainer<const FlatIndexList*, StaticTag, FluentTag, DerivedTag>, PositiveTag, NegativeTag> preconditions,
        GroundNumericConstraintList numeric_constraints);

    /// @brief Get or create an action for the given parameters.
    Action get_or_create_action(std::string name, size_t original_arity, ConjunctiveCondition conjunctive_condition, ConditionalEffectList conditional_effects);

    /// @brief get or create a ground action for the given parameters.
    GroundAction get_or_create_ground_action(Action action,
                                             const ObjectList& binding,
                                             GroundConjunctiveCondition condition,
                                             GroundConditionalEffectList conditional_effects);

    /// @brief Get or create an axiom for the given parameters.
    Axiom get_or_create_axiom(ConjunctiveCondition precondition, Literal<DerivedTag> effect_literal);

    /// @brief Get or create a ground axiom for the given parameters.
    GroundAxiom get_or_create_ground_axiom(Axiom axiom, const ObjectList& binding, GroundConjunctiveCondition condition, GroundLiteral<DerivedTag> effect);

    /// @brief Get or create an optimization metric for the given parameters.
    OptimizationMetric get_or_create_optimization_metric(loki::OptimizationMetricEnum metric, GroundFunctionExpression function_expression);

    /// @brief Get or create an optimization metric for the given parameters.
    template<IsStaticOrFluentOrAuxiliaryTag F>
    GroundFunctionValue<F> get_or_create_ground_function_value(GroundFunction<F> function, double number);

    ///////////////////////////////////////////////////////////////////////////
    /// Accessors
    ///////////////////////////////////////////////////////////////////////////

    // GroundNumericConstraint
    GroundNumericConstraint get_ground_numeric_constraint(size_t numeric_constraint_index) const;

    template<std::ranges::input_range Iterable>
    void get_ground_numeric_constraints_from_indices(const Iterable& numeric_constraint_indices,
                                                     GroundNumericConstraintList& out_ground_numeric_constraints) const;

    template<std::ranges::input_range Iterable>
    GroundNumericConstraintList get_ground_numeric_constraints_from_indices(const Iterable& numeric_constraint_indices) const;

    // GroundNumericEffect
    template<IsFluentOrAuxiliaryTag F>
    GroundNumericEffect<F> get_ground_numeric_effect(size_t numeric_effect_index) const;

    template<IsFluentOrAuxiliaryTag F, std::ranges::input_range Iterable>
    void get_ground_numeric_effects_from_indices(const Iterable& numeric_effect_indices, GroundNumericEffectList<F>& out_ground_numeric_effect) const;

    template<IsFluentOrAuxiliaryTag F, std::ranges::input_range Iterable>
    GroundNumericEffectList<F> get_ground_numeric_effects_from_indices(const Iterable& numeric_effect_indices) const;

    // GroundAtom
    template<IsStaticOrFluentOrDerivedTag P>
    GroundAtom<P> get_ground_atom(size_t atom_index) const;

    template<IsStaticOrFluentOrDerivedTag P, std::ranges::input_range Iterable>
    void get_ground_atoms_from_indices(const Iterable& atom_indices, GroundAtomList<P>& out_ground_atoms) const;

    template<IsStaticOrFluentOrDerivedTag P, std::ranges::input_range Iterable>
    GroundAtomList<P> get_ground_atoms_from_indices(const Iterable& atom_indices) const;

    template<IsStaticOrFluentOrDerivedTag P>
    void get_ground_atoms(GroundAtomList<P>& out_ground_atoms) const;

    template<IsStaticOrFluentOrDerivedTag P>
    auto get_ground_atoms() const;

    // GroundFunction
    template<IsStaticOrFluentOrAuxiliaryTag F>
    void get_ground_functions(size_t num_ground_functions, GroundFunctionList<F>& out_ground_functions) const;

    template<IsStaticOrFluentOrAuxiliaryTag F>
    void get_ground_function_values(const FlatDoubleList& values, std::vector<std::pair<GroundFunction<F>, ContinuousCost>>& out_ground_function_values) const;

    template<IsStaticOrFluentOrAuxiliaryTag F>
    std::vector<std::pair<GroundFunction<F>, ContinuousCost>> get_ground_function_values(const FlatDoubleList& values) const;

    // Object
    Object get_object(size_t object_index) const;

    template<std::ranges::input_range Iterable>
    void get_objects_from_indices(const Iterable& object_indices, ObjectList& out_objects) const;

    template<std::ranges::input_range Iterable>
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
template<std::ranges::input_range Iterable>
void Repositories::get_ground_numeric_constraints_from_indices(const Iterable& numeric_constraint_indices,
                                                               GroundNumericConstraintList& out_ground_numeric_constraints) const
{
    out_ground_numeric_constraints.clear();

    for (const auto& index : numeric_constraint_indices)
    {
        out_ground_numeric_constraints.push_back(get_ground_numeric_constraint(index));
    }
}

template<std::ranges::input_range Iterable>
GroundNumericConstraintList Repositories::get_ground_numeric_constraints_from_indices(const Iterable& numeric_constraint_indices) const
{
    auto result = GroundNumericConstraintList {};
    get_ground_numeric_constraints_from_indices(numeric_constraint_indices, result);
    return result;
}

// GroundNumericEffect
template<IsFluentOrAuxiliaryTag F, std::ranges::input_range Iterable>
void Repositories::get_ground_numeric_effects_from_indices(const Iterable& numeric_effect_indices, GroundNumericEffectList<F>& out_ground_numeric_effect) const
{
    out_ground_numeric_effect.clear();

    for (const auto& index : numeric_effect_indices)
    {
        out_ground_numeric_effect.push_back(get_ground_numeric_effect<F>(index));
    }
}

template<IsFluentOrAuxiliaryTag F, std::ranges::input_range Iterable>
GroundNumericEffectList<F> Repositories::get_ground_numeric_effects_from_indices(const Iterable& numeric_effect_indices) const
{
    auto result = GroundNumericEffectList<F> {};
    get_ground_numeric_effects_from_indices(numeric_effect_indices, result);
    return result;
}

// Atom
template<IsStaticOrFluentOrDerivedTag P, std::ranges::input_range Iterable>
void Repositories::get_ground_atoms_from_indices(const Iterable& atom_indices, GroundAtomList<P>& out_ground_atoms) const
{
    out_ground_atoms.clear();

    for (const auto& index : atom_indices)
    {
        out_ground_atoms.push_back(get_ground_atom<P>(index));
    }
}

template<IsStaticOrFluentOrDerivedTag P, std::ranges::input_range Iterable>
GroundAtomList<P> Repositories::get_ground_atoms_from_indices(const Iterable& atom_indices) const
{
    auto result = GroundAtomList<P> {};
    get_ground_atoms_from_indices(atom_indices, result);
    return result;
}

template<IsStaticOrFluentOrDerivedTag P>
void Repositories::get_ground_atoms(GroundAtomList<P>& out_ground_atoms) const
{
    out_ground_atoms.clear();
    for (const auto& atom : boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {}))
    {
        out_ground_atoms.push_back(atom);
    }
}

template<IsStaticOrFluentOrDerivedTag P>
auto Repositories::get_ground_atoms() const
{
    const auto& factory = boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {});
    return std::ranges::subrange(factory.begin(), factory.end());
}

template<std::ranges::input_range Iterable>
void Repositories::get_objects_from_indices(const Iterable& object_indices, ObjectList& out_objects) const
{
    out_objects.clear();
    for (const auto& object_index : object_indices)
    {
        out_objects.push_back(get_object(object_index));
    }
}

template<std::ranges::input_range Iterable>
ObjectList Repositories::get_objects_from_indices(const Iterable& object_indices) const
{
    auto objects = ObjectList {};
    get_objects_from_indices(object_indices, objects);
    return objects;
}

}

#endif
