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

#include "mimir/formalism/hash.hpp"

#include "mimir/common/hash.hpp"
#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
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

size_t std::hash<loki::ObserverPtr<const mimir::ActionImpl>>::operator()(loki::ObserverPtr<const mimir::ActionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_name(), ptr->get_parameters(), ptr->get_precondition(), ptr->get_strips_effect(), ptr->get_conditional_effects());
}

template<mimir::PredicateTag P>
size_t std::hash<loki::ObserverPtr<const mimir::AtomImpl<P>>>::operator()(loki::ObserverPtr<const mimir::AtomImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->get_predicate(), ptr->get_terms());
}

template struct std::hash<loki::ObserverPtr<const mimir::AtomImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::AtomImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::AtomImpl<mimir::Derived>>>;

size_t std::hash<loki::ObserverPtr<const mimir::AxiomImpl>>::operator()(loki::ObserverPtr<const mimir::AxiomImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_literal(), ptr->get_precondition(), ptr->get_parameters());
}

size_t std::hash<loki::ObserverPtr<const mimir::DomainImpl>>::operator()(loki::ObserverPtr<const mimir::DomainImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_name(),
                               ptr->get_requirements(),
                               ptr->get_constants(),
                               ptr->get_predicates<mimir::Static>(),
                               ptr->get_predicates<mimir::Fluent>(),
                               ptr->get_predicates<mimir::Derived>(),
                               ptr->get_functions(),
                               ptr->get_actions(),
                               ptr->get_axioms());
}

size_t std::hash<loki::ObserverPtr<const mimir::EffectStripsImpl>>::operator()(loki::ObserverPtr<const mimir::EffectStripsImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_effects(), ptr->get_function_expression());
}

size_t std::hash<loki::ObserverPtr<const mimir::EffectConditionalImpl>>::operator()(loki::ObserverPtr<const mimir::EffectConditionalImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_effects(),
                               ptr->get_parameters(),
                               ptr->get_conditions<mimir::Static>(),
                               ptr->get_conditions<mimir::Fluent>(),
                               ptr->get_conditions<mimir::Derived>(),
                               ptr->get_function_expression());
}

size_t std::hash<loki::ObserverPtr<const mimir::ExistentiallyQuantifiedConjunctiveConditionImpl>>::operator()(
    loki::ObserverPtr<const mimir::ExistentiallyQuantifiedConjunctiveConditionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_parameters(),
                               ptr->get_literals<mimir::Static>(),
                               ptr->get_literals<mimir::Fluent>(),
                               ptr->get_literals<mimir::Derived>());
}

size_t
std::hash<loki::ObserverPtr<const mimir::FunctionExpressionNumberImpl>>::operator()(loki::ObserverPtr<const mimir::FunctionExpressionNumberImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_number());
}

size_t std::hash<loki::ObserverPtr<const mimir::FunctionExpressionBinaryOperatorImpl>>::operator()(
    loki::ObserverPtr<const mimir::FunctionExpressionBinaryOperatorImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_binary_operator(), ptr->get_left_function_expression(), ptr->get_right_function_expression());
}

size_t std::hash<loki::ObserverPtr<const mimir::FunctionExpressionMultiOperatorImpl>>::operator()(
    loki::ObserverPtr<const mimir::FunctionExpressionMultiOperatorImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_multi_operator(), ptr->get_function_expressions());
}

size_t std::hash<loki::ObserverPtr<const mimir::FunctionExpressionMinusImpl>>::operator()(loki::ObserverPtr<const mimir::FunctionExpressionMinusImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_function_expression());
}

size_t
std::hash<loki::ObserverPtr<const mimir::FunctionExpressionFunctionImpl>>::operator()(loki::ObserverPtr<const mimir::FunctionExpressionFunctionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_function());
}

size_t std::hash<loki::ObserverPtr<const mimir::FunctionExpressionImpl>>::operator()(loki::ObserverPtr<const mimir::FunctionExpressionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_variant());
}

size_t std::hash<loki::ObserverPtr<const mimir::FunctionSkeletonImpl>>::operator()(loki::ObserverPtr<const mimir::FunctionSkeletonImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_name(), ptr->get_parameters());
}

size_t std::hash<loki::ObserverPtr<const mimir::FunctionImpl>>::operator()(loki::ObserverPtr<const mimir::FunctionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_function_skeleton(), ptr->get_terms());
}

template<mimir::PredicateTag P>
size_t std::hash<loki::ObserverPtr<const mimir::GroundAtomImpl<P>>>::operator()(loki::ObserverPtr<const mimir::GroundAtomImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->get_predicate(), ptr->get_objects());
}

template struct std::hash<loki::ObserverPtr<const mimir::GroundAtomImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::GroundAtomImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::GroundAtomImpl<mimir::Derived>>>;

size_t std::hash<loki::ObserverPtr<const mimir::GroundFunctionExpressionNumberImpl>>::operator()(
    loki::ObserverPtr<const mimir::GroundFunctionExpressionNumberImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_number());
}

size_t std::hash<loki::ObserverPtr<const mimir::GroundFunctionExpressionBinaryOperatorImpl>>::operator()(
    loki::ObserverPtr<const mimir::GroundFunctionExpressionBinaryOperatorImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_binary_operator(), ptr->get_left_function_expression(), ptr->get_right_function_expression());
}

size_t std::hash<loki::ObserverPtr<const mimir::GroundFunctionExpressionMultiOperatorImpl>>::operator()(
    loki::ObserverPtr<const mimir::GroundFunctionExpressionMultiOperatorImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_multi_operator(), ptr->get_function_expressions());
}

size_t std::hash<loki::ObserverPtr<const mimir::GroundFunctionExpressionMinusImpl>>::operator()(
    loki::ObserverPtr<const mimir::GroundFunctionExpressionMinusImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_function_expression());
}

size_t std::hash<loki::ObserverPtr<const mimir::GroundFunctionExpressionFunctionImpl>>::operator()(
    loki::ObserverPtr<const mimir::GroundFunctionExpressionFunctionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_function());
}

size_t
std::hash<loki::ObserverPtr<const mimir::GroundFunctionExpressionImpl>>::operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_variant());
}

size_t std::hash<loki::ObserverPtr<const mimir::GroundFunctionImpl>>::operator()(loki::ObserverPtr<const mimir::GroundFunctionImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_function_skeleton(), ptr->get_objects());
}

template<mimir::PredicateTag P>
size_t std::hash<loki::ObserverPtr<const mimir::GroundLiteralImpl<P>>>::operator()(loki::ObserverPtr<const mimir::GroundLiteralImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->is_negated(), ptr->get_atom());
}

template struct std::hash<loki::ObserverPtr<const mimir::GroundLiteralImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::GroundLiteralImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::GroundLiteralImpl<mimir::Derived>>>;

template<mimir::PredicateTag P>
size_t std::hash<loki::ObserverPtr<const mimir::LiteralImpl<P>>>::operator()(loki::ObserverPtr<const mimir::LiteralImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->is_negated(), ptr->get_atom());
}

template struct std::hash<loki::ObserverPtr<const mimir::LiteralImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::LiteralImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::LiteralImpl<mimir::Derived>>>;

size_t std::hash<loki::ObserverPtr<const mimir::OptimizationMetricImpl>>::operator()(loki::ObserverPtr<const mimir::OptimizationMetricImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_optimization_metric(), ptr->get_function_expression());
}

size_t std::hash<loki::ObserverPtr<const mimir::NumericFluentImpl>>::operator()(loki::ObserverPtr<const mimir::NumericFluentImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_number(), ptr->get_function());
}

size_t std::hash<loki::ObserverPtr<const mimir::ObjectImpl>>::operator()(loki::ObserverPtr<const mimir::ObjectImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_name());
}

template<mimir::PredicateTag P>
size_t std::hash<loki::ObserverPtr<const mimir::PredicateImpl<P>>>::operator()(loki::ObserverPtr<const mimir::PredicateImpl<P>> ptr) const
{
    return mimir::hash_combine(ptr->get_name(), ptr->get_parameters());
}

template struct std::hash<loki::ObserverPtr<const mimir::PredicateImpl<mimir::Static>>>;
template struct std::hash<loki::ObserverPtr<const mimir::PredicateImpl<mimir::Fluent>>>;
template struct std::hash<loki::ObserverPtr<const mimir::PredicateImpl<mimir::Derived>>>;

size_t std::hash<loki::ObserverPtr<const mimir::ProblemImpl>>::operator()(loki::ObserverPtr<const mimir::ProblemImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_name(),
                               ptr->get_requirements(),
                               ptr->get_domain(),
                               ptr->get_objects(),
                               ptr->get_derived_predicates(),
                               ptr->get_static_initial_literals(),
                               ptr->get_fluent_initial_literals(),
                               ptr->get_numeric_fluents(),
                               ptr->get_goal_condition<mimir::Static>(),
                               ptr->get_goal_condition<mimir::Fluent>(),
                               ptr->get_goal_condition<mimir::Derived>(),
                               ptr->get_optimization_metric(),
                               ptr->get_axioms());
}

size_t std::hash<loki::ObserverPtr<const mimir::RequirementsImpl>>::operator()(loki::ObserverPtr<const mimir::RequirementsImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_requirements());
}

size_t std::hash<loki::ObserverPtr<const mimir::TermImpl>>::operator()(loki::ObserverPtr<const mimir::TermImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_variant());
}

size_t std::hash<loki::ObserverPtr<const mimir::VariableImpl>>::operator()(loki::ObserverPtr<const mimir::VariableImpl> ptr) const
{
    return mimir::hash_combine(ptr->get_name(), ptr->get_parameter_index());
}
