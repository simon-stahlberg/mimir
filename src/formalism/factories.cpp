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

#include "mimir/formalism/factories.hpp"

namespace mimir
{

PDDLTypeToFactory create_default_pddl_type_to_factory()
{
    return boost::hana::make_map(boost::hana::make_pair(boost::hana::type_c<RequirementsImpl>, RequirementsFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<VariableImpl>, VariableFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<TermImpl>, TermFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<ObjectImpl>, ObjectFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<AtomImpl<Static>>, AtomFactory<Static> {}),
                                 boost::hana::make_pair(boost::hana::type_c<AtomImpl<Fluent>>, AtomFactory<Fluent> {}),
                                 boost::hana::make_pair(boost::hana::type_c<AtomImpl<Derived>>, AtomFactory<Derived> {}),
                                 boost::hana::make_pair(boost::hana::type_c<GroundAtomImpl<Static>>, GroundAtomFactory<Static> {}),
                                 boost::hana::make_pair(boost::hana::type_c<GroundAtomImpl<Fluent>>, GroundAtomFactory<Fluent> {}),
                                 boost::hana::make_pair(boost::hana::type_c<GroundAtomImpl<Derived>>, GroundAtomFactory<Derived> {}),
                                 boost::hana::make_pair(boost::hana::type_c<LiteralImpl<Static>>, LiteralFactory<Static> {}),
                                 boost::hana::make_pair(boost::hana::type_c<LiteralImpl<Fluent>>, LiteralFactory<Fluent> {}),
                                 boost::hana::make_pair(boost::hana::type_c<LiteralImpl<Derived>>, LiteralFactory<Derived> {}),
                                 boost::hana::make_pair(boost::hana::type_c<GroundLiteralImpl<Static>>, GroundLiteralFactory<Static> {}),
                                 boost::hana::make_pair(boost::hana::type_c<GroundLiteralImpl<Fluent>>, GroundLiteralFactory<Fluent> {}),
                                 boost::hana::make_pair(boost::hana::type_c<GroundLiteralImpl<Derived>>, GroundLiteralFactory<Derived> {}),
                                 boost::hana::make_pair(boost::hana::type_c<PredicateImpl<Static>>, PredicateFactory<Static> {}),
                                 boost::hana::make_pair(boost::hana::type_c<PredicateImpl<Fluent>>, PredicateFactory<Fluent> {}),
                                 boost::hana::make_pair(boost::hana::type_c<PredicateImpl<Derived>>, PredicateFactory<Derived> {}),
                                 boost::hana::make_pair(boost::hana::type_c<FunctionExpressionImpl>, FunctionExpressionFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<GroundFunctionExpressionImpl>, GroundFunctionExpressionFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<FunctionImpl>, FunctionFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<GroundFunctionImpl>, GroundFunctionFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<FunctionSkeletonImpl>, FunctionSkeletonFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<EffectSimpleImpl>, EffectSimpleFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<EffectComplexImpl>, EffectUniversalFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<ActionImpl>, ActionFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<AxiomImpl>, AxiomFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<OptimizationMetricImpl>, OptimizationMetricFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<NumericFluentImpl>, NumericFluentFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<DomainImpl>, DomainFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<ProblemImpl>, ProblemFactory {}));
}

PDDLFactories::PDDLFactories() : m_repositories(create_default_pddl_type_to_factory()) {}

PDDLFactories::PDDLFactories(PDDLFactories&& other) = default;

PDDLFactories& PDDLFactories::operator=(PDDLFactories&& other) = default;

Requirements PDDLFactories::get_or_create_requirements(loki::RequirementEnumSet requirement_set)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RequirementsImpl> {}).template get_or_create<RequirementsImpl>(std::move(requirement_set));
}

Variable PDDLFactories::get_or_create_variable(std::string name, size_t parameter_index)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<VariableImpl> {})
        .template get_or_create<VariableImpl>(std::move(name), std::move(parameter_index));
}

Term PDDLFactories::get_or_create_term_variable(Variable variable)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<TermImpl> {}).template get_or_create<TermVariableImpl>(std::move(variable));
}

Term PDDLFactories::get_or_create_term_object(Object object)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<TermImpl> {}).template get_or_create<TermObjectImpl>(std::move(object));
}

Object PDDLFactories::get_or_create_object(std::string name)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ObjectImpl> {}).template get_or_create<ObjectImpl>(std::move(name));
}

template<PredicateCategory P>
Atom<P> PDDLFactories::get_or_create_atom(Predicate<P> predicate, TermList terms)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<AtomImpl<P>> {}).template get_or_create<AtomImpl<P>>(std::move(predicate), std::move(terms));
}

template Atom<Static> PDDLFactories::get_or_create_atom(Predicate<Static> predicate, TermList terms);
template Atom<Fluent> PDDLFactories::get_or_create_atom(Predicate<Fluent> predicate, TermList terms);
template Atom<Derived> PDDLFactories::get_or_create_atom(Predicate<Derived> predicate, TermList terms);

template<PredicateCategory P>
GroundAtom<P> PDDLFactories::get_or_create_ground_atom(Predicate<P> predicate, ObjectList objects)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {})
        .template get_or_create<GroundAtomImpl<P>>(std::move(predicate), std::move(objects));
}

template GroundAtom<Static> PDDLFactories::get_or_create_ground_atom(Predicate<Static> predicate, ObjectList ObjectList);
template GroundAtom<Fluent> PDDLFactories::get_or_create_ground_atom(Predicate<Fluent> predicate, ObjectList ObjectList);
template GroundAtom<Derived> PDDLFactories::get_or_create_ground_atom(Predicate<Derived> predicate, ObjectList ObjectList);

template<PredicateCategory P>
Literal<P> PDDLFactories::get_or_create_literal(bool is_negated, Atom<P> atom)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<LiteralImpl<P>> {}).template get_or_create<LiteralImpl<P>>(is_negated, std::move(atom));
}

template Literal<Static> PDDLFactories::get_or_create_literal(bool is_negated, Atom<Static> atom);
template Literal<Fluent> PDDLFactories::get_or_create_literal(bool is_negated, Atom<Fluent> atom);
template Literal<Derived> PDDLFactories::get_or_create_literal(bool is_negated, Atom<Derived> atom);

template<PredicateCategory P>
GroundLiteral<P> PDDLFactories::get_or_create_ground_literal(bool is_negated, GroundAtom<P> atom)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundLiteralImpl<P>> {})
        .template get_or_create<GroundLiteralImpl<P>>(is_negated, std::move(atom));
}

template GroundLiteral<Static> PDDLFactories::get_or_create_ground_literal(bool is_negated, GroundAtom<Static> atom);
template GroundLiteral<Fluent> PDDLFactories::get_or_create_ground_literal(bool is_negated, GroundAtom<Fluent> atom);
template GroundLiteral<Derived> PDDLFactories::get_or_create_ground_literal(bool is_negated, GroundAtom<Derived> atom);

template<PredicateCategory P>
Predicate<P> PDDLFactories::get_or_create_predicate(std::string name, VariableList parameters)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<PredicateImpl<P>> {}).template get_or_create<PredicateImpl<P>>(name, std::move(parameters));
}

template Predicate<Static> PDDLFactories::get_or_create_predicate(std::string name, VariableList parameters);
template Predicate<Fluent> PDDLFactories::get_or_create_predicate(std::string name, VariableList parameters);
template Predicate<Derived> PDDLFactories::get_or_create_predicate(std::string name, VariableList parameters);

FunctionExpression PDDLFactories::get_or_create_function_expression_number(double number)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {}).template get_or_create<FunctionExpressionNumberImpl>(number);
}

FunctionExpression PDDLFactories::get_or_create_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                    FunctionExpression left_function_expression,
                                                                                    FunctionExpression right_function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {})
        .template get_or_create<FunctionExpressionBinaryOperatorImpl>(binary_operator,
                                                                      std::move(left_function_expression),
                                                                      std::move(right_function_expression));
}

FunctionExpression PDDLFactories::get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                   FunctionExpressionList function_expressions)
{
    /* Canonize before uniqueness test. */
    std::sort(function_expressions.begin(),
              function_expressions.end(),
              [](const auto& l, const auto& r)
              { return std::visit([](const auto& arg) { return arg.get_index(); }, *l) < std::visit([](const auto& arg) { return arg.get_index(); }, *r); });

    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {})
        .template get_or_create<FunctionExpressionMultiOperatorImpl>(multi_operator, std::move(function_expressions));
}

FunctionExpression PDDLFactories::get_or_create_function_expression_minus(FunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {})
        .template get_or_create<FunctionExpressionMinusImpl>(std::move(function_expression));
}

FunctionExpression PDDLFactories::get_or_create_function_expression_function(Function function)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {})
        .template get_or_create<FunctionExpressionFunctionImpl>(std::move(function));
}

GroundFunctionExpression PDDLFactories::get_or_create_ground_function_expression_number(double number)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {})
        .template get_or_create<GroundFunctionExpressionNumberImpl>(number);
}

GroundFunctionExpression PDDLFactories::get_or_create_ground_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                                 GroundFunctionExpression left_function_expression,
                                                                                                 GroundFunctionExpression right_function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {})
        .template get_or_create<GroundFunctionExpressionBinaryOperatorImpl>(binary_operator,
                                                                            std::move(left_function_expression),
                                                                            std::move(right_function_expression));
}

GroundFunctionExpression PDDLFactories::get_or_create_ground_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                                GroundFunctionExpressionList function_expressions)
{
    std::sort(function_expressions.begin(),
              function_expressions.end(),
              [](const auto& l, const auto& r)
              { return std::visit([](const auto& arg) { return arg.get_index(); }, *l) < std::visit([](const auto& arg) { return arg.get_index(); }, *r); });

    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {})
        .template get_or_create<GroundFunctionExpressionMultiOperatorImpl>(multi_operator, std::move(function_expressions));
}

GroundFunctionExpression PDDLFactories::get_or_create_ground_function_expression_minus(GroundFunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {})
        .template get_or_create<GroundFunctionExpressionMinusImpl>(std::move(function_expression));
}

GroundFunctionExpression PDDLFactories::get_or_create_ground_function_expression_function(GroundFunction function)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {})
        .template get_or_create<GroundFunctionExpressionFunctionImpl>(std::move(function));
}

Function PDDLFactories::get_or_create_function(FunctionSkeleton function_skeleton, TermList terms)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionImpl> {})
        .template get_or_create<FunctionImpl>(std::move(function_skeleton), std::move(terms));
}

GroundFunction PDDLFactories::get_or_create_ground_function(FunctionSkeleton function_skeleton, ObjectList objects)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionImpl> {})
        .template get_or_create<GroundFunctionImpl>(std::move(function_skeleton), std::move(objects));
}

FunctionSkeleton PDDLFactories::get_or_create_function_skeleton(std::string name, VariableList parameters)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionSkeletonImpl> {})
        .template get_or_create<FunctionSkeletonImpl>(std::move(name), std::move(parameters));
}

EffectSimple PDDLFactories::get_or_create_simple_effect(Literal<Fluent> effect)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<EffectSimpleImpl> {}).template get_or_create<EffectSimpleImpl>(std::move(effect));
}

EffectComplex PDDLFactories::get_or_create_complex_effect(VariableList parameters,
                                                          LiteralList<Static> static_conditions,
                                                          LiteralList<Fluent> fluent_conditions,
                                                          LiteralList<Derived> derived_conditions,
                                                          Literal<Fluent> effect)
{
    std::sort(static_conditions.begin(), static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_conditions.begin(), fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_conditions.begin(), derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<EffectComplexImpl> {})
        .template get_or_create<EffectComplexImpl>(std::move(parameters),
                                                   std::move(static_conditions),
                                                   std::move(fluent_conditions),
                                                   std::move(derived_conditions),
                                                   std::move(effect));
}

Action PDDLFactories::get_or_create_action(std::string name,
                                           size_t original_arity,
                                           VariableList parameters,
                                           LiteralList<Static> static_conditions,
                                           LiteralList<Fluent> fluent_conditions,
                                           LiteralList<Derived> derived_conditions,
                                           EffectSimpleList simple_effects,
                                           EffectComplexList complex_effects,
                                           FunctionExpression function_expression)
{
    /* Canonize before uniqueness test */
    std::sort(static_conditions.begin(), static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_conditions.begin(), fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_conditions.begin(), derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(simple_effects.begin(), simple_effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    // Sort negative conditional effects to the beginning to process them first, additionally sort then by identifier.
    std::sort(complex_effects.begin(),
              complex_effects.end(),
              [](const auto& l, const auto& r)
              {
                  if (l->get_effect()->is_negated() == r->get_effect()->is_negated())
                  {
                      return l->get_index() < r->get_index();
                  }
                  return l->get_effect()->is_negated() > r->get_effect()->is_negated();
              });

    return boost::hana::at_key(m_repositories, boost::hana::type<ActionImpl> {})
        .template get_or_create<ActionImpl>(std::move(name),
                                            std::move(original_arity),
                                            std::move(parameters),
                                            std::move(static_conditions),
                                            std::move(fluent_conditions),
                                            std::move(derived_conditions),
                                            std::move(simple_effects),
                                            std::move(complex_effects),
                                            std::move(function_expression));
}

Axiom PDDLFactories::get_or_create_axiom(VariableList parameters,
                                         Literal<Derived> literal,
                                         LiteralList<Static> static_conditions,
                                         LiteralList<Fluent> fluent_conditions,
                                         LiteralList<Derived> derived_conditions)
{
    /* Canonize before uniqueness test. */
    std::sort(static_conditions.begin(), static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_conditions.begin(), fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_conditions.begin(), derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<AxiomImpl> {})
        .template get_or_create<AxiomImpl>(std::move(parameters),
                                           std::move(literal),
                                           std::move(static_conditions),
                                           std::move(fluent_conditions),
                                           std::move(derived_conditions));
}

OptimizationMetric PDDLFactories::get_or_create_optimization_metric(loki::OptimizationMetricEnum metric, GroundFunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<OptimizationMetricImpl> {})
        .template get_or_create<OptimizationMetricImpl>(std::move(metric), std::move(function_expression));
}

NumericFluent PDDLFactories::get_or_create_numeric_fluent(GroundFunction function, double number)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericFluentImpl> {})
        .template get_or_create<NumericFluentImpl>(std::move(function), std::move(number));
}

Domain PDDLFactories::get_or_create_domain(std::optional<fs::path> filepath,
                                           std::string name,
                                           Requirements requirements,
                                           ObjectList constants,
                                           PredicateList<Static> static_predicates,
                                           PredicateList<Fluent> fluent_predicates,
                                           PredicateList<Derived> derived_predicates,
                                           FunctionSkeletonList functions,
                                           ActionList actions,
                                           AxiomList axioms)
{
    /* Canonize before uniqueness test. */
    std::sort(constants.begin(), constants.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(static_predicates.begin(), static_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_predicates.begin(), fluent_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_predicates.begin(), derived_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(functions.begin(), functions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(actions.begin(), actions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(axioms.begin(), axioms.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<DomainImpl> {})
        .template get_or_create<DomainImpl>(std::move(filepath),
                                            std::move(name),
                                            std::move(requirements),
                                            std::move(constants),
                                            std::move(static_predicates),
                                            std::move(fluent_predicates),
                                            std::move(derived_predicates),
                                            std::move(functions),
                                            std::move(actions),
                                            std::move(axioms));
}

Problem PDDLFactories::get_or_create_problem(std::optional<fs::path> filepath,
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
                                             AxiomList axioms)
{
    /* Canonize before uniqueness test. */
    std::sort(objects.begin(), objects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_predicates.begin(), derived_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(static_initial_literals.begin(), static_initial_literals.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_initial_literals.begin(), fluent_initial_literals.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(numeric_fluents.begin(), numeric_fluents.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(static_goal_condition.begin(), static_goal_condition.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_goal_condition.begin(), fluent_goal_condition.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_goal_condition.begin(), derived_goal_condition.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(axioms.begin(), axioms.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<ProblemImpl> {})
        .template get_or_create<ProblemImpl>(std::move(filepath),
                                             std::move(domain),
                                             std::move(name),
                                             std::move(requirements),
                                             std::move(objects),
                                             std::move(derived_predicates),
                                             std::move(static_initial_literals),
                                             std::move(fluent_initial_literals),
                                             std::move(numeric_fluents),
                                             std::move(static_goal_condition),
                                             std::move(fluent_goal_condition),
                                             std::move(derived_goal_condition),
                                             std::move(optimization_metric),
                                             std::move(axioms));
}

// Factory
const PDDLTypeToFactory& PDDLFactories::get_pddl_type_to_factory() const { return m_repositories; }

// GroundAtom
template<PredicateCategory P>
GroundAtom<P> PDDLFactories::get_ground_atom(size_t atom_id) const
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {}).at(atom_id);
}

template GroundAtom<Static> PDDLFactories::get_ground_atom<Static>(size_t atom_id) const;
template GroundAtom<Fluent> PDDLFactories::get_ground_atom<Fluent>(size_t atom_id) const;
template GroundAtom<Derived> PDDLFactories::get_ground_atom<Derived>(size_t atom_id) const;

// Object
Object PDDLFactories::get_object(size_t object_id) const { return boost::hana::at_key(m_repositories, boost::hana::type<ObjectImpl> {}).at(object_id); }

// Action
Action PDDLFactories::get_action(size_t action_index) const { return boost::hana::at_key(m_repositories, boost::hana::type<ActionImpl> {}).at(action_index); }

// Axiom
Axiom PDDLFactories::get_axiom(size_t axiom_index) const { return boost::hana::at_key(m_repositories, boost::hana::type<AxiomImpl> {}).at(axiom_index); }

/* Grounding */

void PDDLFactories::ground_variables(const TermList& terms, const ObjectList& binding, ObjectList& out_terms)
{
    out_terms.clear();

    for (const auto& term : terms)
    {
        std::visit(
            [&](const auto& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, TermObjectImpl>)
                {
                    out_terms.emplace_back(arg.get_object());
                }
                else if constexpr (std::is_same_v<T, TermVariableImpl>)
                {
                    assert(arg.get_variable()->get_parameter_index() < binding.size());
                    out_terms.emplace_back(binding[arg.get_variable()->get_parameter_index()]);
                }
            },
            *term);
    }
}

template<PredicateCategory P>
GroundLiteral<P> PDDLFactories::ground_literal(const Literal<P> literal, const ObjectList& binding)
{
    /* 1. Access the type specific grounding tables. */
    auto& grounding_tables = m_grounding_tables.get<GroundLiteral<P>>();

    /* 2. Access the literal specific grounding table */
    const auto literal_id = literal->get_index();
    if (literal_id >= grounding_tables.size())
    {
        grounding_tables.resize(literal_id + 1);
    }

    auto& grounding_table = grounding_tables.at(literal_id);

    /* 3. Check if grounding is cached */
    const auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 4. Ground the literal */

    auto grounded_terms = ObjectList {};
    ground_variables(literal->get_atom()->get_terms(), binding, grounded_terms);
    auto grounded_atom = get_or_create_ground_atom(literal->get_atom()->get_predicate(), grounded_terms);
    auto grounded_literal = get_or_create_ground_literal(literal->is_negated(), grounded_atom);

    /* 5. Insert to grounding_table table */

    grounding_table.emplace(ObjectList(binding), GroundLiteral<P>(grounded_literal));

    /* 6. Return the resulting ground literal */

    return grounded_literal;
}

template<PredicateCategory P>
void PDDLFactories::ground_and_fill_bitset(const std::vector<Literal<P>>& literals,
                                           FlatBitset& ref_positive_bitset,
                                           FlatBitset& ref_negative_bitset,
                                           const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        const auto grounded_literal = ground_literal(literal, binding);

        if (grounded_literal->is_negated())
        {
            ref_negative_bitset.set(grounded_literal->get_atom()->get_index());
        }
        else
        {
            ref_positive_bitset.set(grounded_literal->get_atom()->get_index());
        }
    }
}

template void PDDLFactories::ground_and_fill_bitset(const std::vector<Literal<Static>>& literals,
                                                    FlatBitset& ref_positive_bitset,
                                                    FlatBitset& ref_negative_bitset,
                                                    const ObjectList& binding);
template void PDDLFactories::ground_and_fill_bitset(const std::vector<Literal<Fluent>>& literals,
                                                    FlatBitset& ref_positive_bitset,
                                                    FlatBitset& ref_negative_bitset,
                                                    const ObjectList& binding);
template void PDDLFactories::ground_and_fill_bitset(const std::vector<Literal<Derived>>& literals,
                                                    FlatBitset& ref_positive_bitset,
                                                    FlatBitset& ref_negative_bitset,
                                                    const ObjectList& binding);

template<PredicateCategory P>
void PDDLFactories::ground_and_fill_vector(const std::vector<Literal<P>>& literals,
                                           FlatIndexList& ref_positive_indices,
                                           FlatIndexList& ref_negative_indices,
                                           const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        const auto grounded_literal = ground_literal(literal, binding);

        if (grounded_literal->is_negated())
        {
            ref_negative_indices.push_back(grounded_literal->get_atom()->get_index());
        }
        else
        {
            ref_positive_indices.push_back(grounded_literal->get_atom()->get_index());
        }
    }
    std::sort(ref_positive_indices.begin(), ref_positive_indices.end());
    std::sort(ref_negative_indices.begin(), ref_negative_indices.end());
}

template void PDDLFactories::ground_and_fill_vector(const std::vector<Literal<Static>>& literals,
                                                    FlatIndexList& ref_positive_indices,
                                                    FlatIndexList& ref_negative_indices,
                                                    const ObjectList& binding);
template void PDDLFactories::ground_and_fill_vector(const std::vector<Literal<Fluent>>& literals,
                                                    FlatIndexList& ref_positive_indices,
                                                    FlatIndexList& ref_negative_indices,
                                                    const ObjectList& binding);
template void PDDLFactories::ground_and_fill_vector(const std::vector<Literal<Derived>>& literals,
                                                    FlatIndexList& ref_positive_indices,
                                                    FlatIndexList& ref_negative_indices,
                                                    const ObjectList& binding);
}
