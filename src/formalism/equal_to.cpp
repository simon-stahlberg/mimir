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

#include "mimir/formalism/equal_to.hpp"

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/equal_to.hpp"
#include "mimir/formalism/existentially_quantified_conjunctive_condition.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"

// Explicit empty function definitions for all equal_to specializations

bool std::equal_to<loki::ObserverPtr<const mimir::ActionImpl>>::operator()(loki::ObserverPtr<const mimir::ActionImpl> lhs,
                                                                           loki::ObserverPtr<const mimir::ActionImpl> rhs) const
{
    return (lhs->get_name() == rhs->get_name()) && (lhs->get_parameters() == rhs->get_parameters()) && (lhs->get_precondition() == rhs->get_precondition())
           && (lhs->get_strips_effect() == rhs->get_strips_effect()) && (lhs->get_conditional_effects() == rhs->get_conditional_effects());
}

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::AtomImpl<P>>>::operator()(loki::ObserverPtr<const mimir::AtomImpl<P>> lhs,
                                                                            loki::ObserverPtr<const mimir::AtomImpl<P>> rhs) const
{
    return (lhs->get_predicate() == rhs->get_predicate()) && (lhs->get_terms() == rhs->get_terms());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::AtomImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::AtomImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::AtomImpl<mimir::Derived>>>;

bool std::equal_to<loki::ObserverPtr<const mimir::AxiomImpl>>::operator()(loki::ObserverPtr<const mimir::AxiomImpl> lhs,
                                                                          loki::ObserverPtr<const mimir::AxiomImpl> rhs) const
{
    return (lhs->get_precondition() == rhs->get_precondition()) && (lhs->get_literal() == rhs->get_literal());
}

bool std::equal_to<loki::ObserverPtr<const mimir::DomainImpl>>::operator()(loki::ObserverPtr<const mimir::DomainImpl> lhs,
                                                                           loki::ObserverPtr<const mimir::DomainImpl> rhs) const
{
    return (lhs->get_name() == rhs->get_name()) && (lhs->get_requirements() == rhs->get_requirements()) && (lhs->get_constants() == rhs->get_constants())
           && (lhs->get_predicates<mimir::Static>() == rhs->get_predicates<mimir::Static>())
           && (lhs->get_predicates<mimir::Fluent>() == rhs->get_predicates<mimir::Fluent>())
           && (lhs->get_predicates<mimir::Derived>() == rhs->get_predicates<mimir::Derived>()) && (lhs->get_functions() == rhs->get_functions())
           && (lhs->get_actions() == rhs->get_actions()) && (lhs->get_axioms() == rhs->get_axioms());
}

bool std::equal_to<loki::ObserverPtr<const mimir::EffectStripsImpl>>::operator()(loki::ObserverPtr<const mimir::EffectStripsImpl> lhs,
                                                                                 loki::ObserverPtr<const mimir::EffectStripsImpl> rhs) const
{
    return (lhs->get_effects() == rhs->get_effects()) && (lhs->get_function_expression() == rhs->get_function_expression());
}

bool std::equal_to<loki::ObserverPtr<const mimir::EffectConditionalImpl>>::operator()(loki::ObserverPtr<const mimir::EffectConditionalImpl> lhs,
                                                                                      loki::ObserverPtr<const mimir::EffectConditionalImpl> rhs) const
{
    return (lhs->get_effects() == rhs->get_effects()) && (lhs->get_parameters() == rhs->get_parameters())
           && (lhs->get_conditions<mimir::Static>() == rhs->get_conditions<mimir::Static>())
           && (lhs->get_conditions<mimir::Fluent>() == rhs->get_conditions<mimir::Fluent>())
           && (lhs->get_conditions<mimir::Derived>() == rhs->get_conditions<mimir::Derived>())
           && (lhs->get_function_expression() == rhs->get_function_expression());
}

bool std::equal_to<loki::ObserverPtr<const mimir::ExistentiallyQuantifiedConjunctiveConditionImpl>>::operator()(
    loki::ObserverPtr<const mimir::ExistentiallyQuantifiedConjunctiveConditionImpl> lhs,
    loki::ObserverPtr<const mimir::ExistentiallyQuantifiedConjunctiveConditionImpl> rhs) const
{
    return (lhs->get_parameters() == rhs->get_parameters()) && (lhs->get_literals<mimir::Static>() == rhs->get_literals<mimir::Static>())
           && (lhs->get_literals<mimir::Fluent>() == rhs->get_literals<mimir::Fluent>())
           && (lhs->get_literals<mimir::Derived>() == rhs->get_literals<mimir::Derived>());
}

bool std::equal_to<loki::ObserverPtr<const mimir::FunctionExpressionNumberImpl>>::operator()(
    loki::ObserverPtr<const mimir::FunctionExpressionNumberImpl> lhs,
    loki::ObserverPtr<const mimir::FunctionExpressionNumberImpl> rhs) const
{
    return (lhs->get_number() == rhs->get_number());
}

bool std::equal_to<loki::ObserverPtr<const mimir::FunctionExpressionBinaryOperatorImpl>>::operator()(
    loki::ObserverPtr<const mimir::FunctionExpressionBinaryOperatorImpl> lhs,
    loki::ObserverPtr<const mimir::FunctionExpressionBinaryOperatorImpl> rhs) const
{
    return (lhs->get_binary_operator() == rhs->get_binary_operator()) && (lhs->get_left_function_expression() == rhs->get_left_function_expression())
           && (lhs->get_right_function_expression() == rhs->get_right_function_expression());
}

bool std::equal_to<loki::ObserverPtr<const mimir::FunctionExpressionMultiOperatorImpl>>::operator()(
    loki::ObserverPtr<const mimir::FunctionExpressionMultiOperatorImpl> lhs,
    loki::ObserverPtr<const mimir::FunctionExpressionMultiOperatorImpl> rhs) const
{
    return (lhs->get_multi_operator() == rhs->get_multi_operator()) && (lhs->get_function_expressions() == rhs->get_function_expressions());
}

bool std::equal_to<loki::ObserverPtr<const mimir::FunctionExpressionMinusImpl>>::operator()(
    loki::ObserverPtr<const mimir::FunctionExpressionMinusImpl> lhs,
    loki::ObserverPtr<const mimir::FunctionExpressionMinusImpl> rhs) const
{
    return (lhs->get_function_expression() == rhs->get_function_expression());
}

bool std::equal_to<loki::ObserverPtr<const mimir::FunctionExpressionFunctionImpl>>::operator()(
    loki::ObserverPtr<const mimir::FunctionExpressionFunctionImpl> lhs,
    loki::ObserverPtr<const mimir::FunctionExpressionFunctionImpl> rhs) const
{
    return (lhs->get_function() == rhs->get_function());
}

bool std::equal_to<loki::ObserverPtr<const mimir::FunctionExpressionImpl>>::operator()(loki::ObserverPtr<const mimir::FunctionExpressionImpl> lhs,
                                                                                       loki::ObserverPtr<const mimir::FunctionExpressionImpl> rhs) const
{
    return lhs->get_variant() == rhs->get_variant();
}

bool std::equal_to<loki::ObserverPtr<const mimir::FunctionSkeletonImpl>>::operator()(loki::ObserverPtr<const mimir::FunctionSkeletonImpl> lhs,
                                                                                     loki::ObserverPtr<const mimir::FunctionSkeletonImpl> rhs) const
{
    return (lhs->get_name() == rhs->get_name()) && (lhs->get_parameters() == rhs->get_parameters());
}

bool std::equal_to<loki::ObserverPtr<const mimir::FunctionImpl>>::operator()(loki::ObserverPtr<const mimir::FunctionImpl> lhs,
                                                                             loki::ObserverPtr<const mimir::FunctionImpl> rhs) const
{
    return (lhs->get_function_skeleton() == rhs->get_function_skeleton()) && (lhs->get_terms() == rhs->get_terms());
}

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::GroundAtomImpl<P>>>::operator()(loki::ObserverPtr<const mimir::GroundAtomImpl<P>> lhs,
                                                                                  loki::ObserverPtr<const mimir::GroundAtomImpl<P>> rhs) const
{
    return (lhs->get_predicate() == rhs->get_predicate()) && (lhs->get_objects() == rhs->get_objects());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::GroundAtomImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::GroundAtomImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::GroundAtomImpl<mimir::Derived>>>;

bool std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionExpressionNumberImpl>>::operator()(
    loki::ObserverPtr<const mimir::GroundFunctionExpressionNumberImpl> lhs,
    loki::ObserverPtr<const mimir::GroundFunctionExpressionNumberImpl> rhs) const
{
    return (lhs->get_number() == rhs->get_number());
}

bool std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionExpressionBinaryOperatorImpl>>::operator()(
    loki::ObserverPtr<const mimir::GroundFunctionExpressionBinaryOperatorImpl> lhs,
    loki::ObserverPtr<const mimir::GroundFunctionExpressionBinaryOperatorImpl> rhs) const
{
    return (lhs->get_binary_operator() == rhs->get_binary_operator()) && (lhs->get_left_function_expression() == rhs->get_left_function_expression())
           && (lhs->get_right_function_expression() == rhs->get_right_function_expression());
}

bool std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionExpressionMultiOperatorImpl>>::operator()(
    loki::ObserverPtr<const mimir::GroundFunctionExpressionMultiOperatorImpl> lhs,
    loki::ObserverPtr<const mimir::GroundFunctionExpressionMultiOperatorImpl> rhs) const
{
    return (lhs->get_multi_operator() == rhs->get_multi_operator()) && (lhs->get_function_expressions() == rhs->get_function_expressions());
}

bool std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionExpressionMinusImpl>>::operator()(
    loki::ObserverPtr<const mimir::GroundFunctionExpressionMinusImpl> lhs,
    loki::ObserverPtr<const mimir::GroundFunctionExpressionMinusImpl> rhs) const
{
    return (lhs->get_function_expression() == rhs->get_function_expression());
}

bool std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionExpressionFunctionImpl>>::operator()(
    loki::ObserverPtr<const mimir::GroundFunctionExpressionFunctionImpl> lhs,
    loki::ObserverPtr<const mimir::GroundFunctionExpressionFunctionImpl> rhs) const
{
    return (lhs->get_function() == rhs->get_function());
}

bool std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionExpressionImpl>>::operator()(
    loki::ObserverPtr<const mimir::GroundFunctionExpressionImpl> lhs,
    loki::ObserverPtr<const mimir::GroundFunctionExpressionImpl> rhs) const
{
    return lhs->get_variant() == rhs->get_variant();
}

bool std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionImpl>>::operator()(loki::ObserverPtr<const mimir::GroundFunctionImpl> lhs,
                                                                                   loki::ObserverPtr<const mimir::GroundFunctionImpl> rhs) const
{
    return (lhs->get_function_skeleton() == rhs->get_function_skeleton() && lhs->get_objects() == rhs->get_objects());
}

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::GroundLiteralImpl<P>>>::operator()(loki::ObserverPtr<const mimir::GroundLiteralImpl<P>> lhs,
                                                                                     loki::ObserverPtr<const mimir::GroundLiteralImpl<P>> rhs) const
{
    return (lhs->is_negated() == rhs->is_negated()) && (lhs->get_atom() == rhs->get_atom());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::GroundLiteralImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::GroundLiteralImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::GroundLiteralImpl<mimir::Derived>>>;

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::LiteralImpl<P>>>::operator()(loki::ObserverPtr<const mimir::LiteralImpl<P>> lhs,
                                                                               loki::ObserverPtr<const mimir::LiteralImpl<P>> rhs) const
{
    return (lhs->is_negated() == rhs->is_negated()) && (lhs->get_atom() == rhs->get_atom());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::LiteralImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::LiteralImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::LiteralImpl<mimir::Derived>>>;

bool std::equal_to<loki::ObserverPtr<const mimir::OptimizationMetricImpl>>::operator()(loki::ObserverPtr<const mimir::OptimizationMetricImpl> lhs,
                                                                                       loki::ObserverPtr<const mimir::OptimizationMetricImpl> rhs) const
{
    return (lhs->get_optimization_metric() == rhs->get_optimization_metric()) && (lhs->get_function_expression() == rhs->get_function_expression());
}

bool std::equal_to<loki::ObserverPtr<const mimir::NumericFluentImpl>>::operator()(loki::ObserverPtr<const mimir::NumericFluentImpl> lhs,
                                                                                  loki::ObserverPtr<const mimir::NumericFluentImpl> rhs) const
{
    return (lhs->get_number() == rhs->get_number()) && (lhs->get_function() == rhs->get_function());
}

bool std::equal_to<loki::ObserverPtr<const mimir::ObjectImpl>>::operator()(loki::ObserverPtr<const mimir::ObjectImpl> lhs,
                                                                           loki::ObserverPtr<const mimir::ObjectImpl> rhs) const
{
    return (lhs->get_name() == rhs->get_name());
}

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::PredicateImpl<P>>>::operator()(loki::ObserverPtr<const mimir::PredicateImpl<P>> lhs,
                                                                                 loki::ObserverPtr<const mimir::PredicateImpl<P>> rhs) const
{
    return (lhs->get_name() == rhs->get_name()) && (lhs->get_parameters() == rhs->get_parameters());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::PredicateImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::PredicateImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::PredicateImpl<mimir::Derived>>>;

bool std::equal_to<loki::ObserverPtr<const mimir::ProblemImpl>>::operator()(loki::ObserverPtr<const mimir::ProblemImpl> lhs,
                                                                            loki::ObserverPtr<const mimir::ProblemImpl> rhs) const
{
    return (lhs->get_name() == rhs->get_name()) && (lhs->get_requirements() == rhs->get_requirements()) && (lhs->get_domain() == rhs->get_domain())
           && (lhs->get_objects() == rhs->get_objects()) && (lhs->get_derived_predicates() == rhs->get_derived_predicates())
           && (lhs->get_static_initial_literals() == rhs->get_static_initial_literals())
           && (lhs->get_fluent_initial_literals() == rhs->get_fluent_initial_literals()) && (lhs->get_numeric_fluents() == rhs->get_numeric_fluents())
           && (lhs->get_goal_condition<mimir::Static>() == rhs->get_goal_condition<mimir::Static>())
           && (lhs->get_goal_condition<mimir::Fluent>() == rhs->get_goal_condition<mimir::Fluent>())
           && (lhs->get_goal_condition<mimir::Derived>() == rhs->get_goal_condition<mimir::Derived>())
           && (lhs->get_optimization_metric() == rhs->get_optimization_metric()) && (lhs->get_axioms() == rhs->get_axioms());
}

bool std::equal_to<loki::ObserverPtr<const mimir::RequirementsImpl>>::operator()(loki::ObserverPtr<const mimir::RequirementsImpl> lhs,
                                                                                 loki::ObserverPtr<const mimir::RequirementsImpl> rhs) const
{
    return (lhs->get_requirements() == rhs->get_requirements());
}

bool std::equal_to<loki::ObserverPtr<const mimir::TermImpl>>::operator()(loki::ObserverPtr<const mimir::TermImpl> lhs,
                                                                         loki::ObserverPtr<const mimir::TermImpl> rhs) const
{
    return lhs->get_variant() == rhs->get_variant();
}

bool std::equal_to<loki::ObserverPtr<const mimir::VariableImpl>>::operator()(loki::ObserverPtr<const mimir::VariableImpl> lhs,
                                                                             loki::ObserverPtr<const mimir::VariableImpl> rhs) const
{
    return (lhs->get_name() == rhs->get_name()) && (lhs->get_parameter_index() == rhs->get_parameter_index());
}
