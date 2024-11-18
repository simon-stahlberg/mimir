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

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
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

namespace mimir
{
size_t UniquePDDLHasher<const ActionImpl*>::operator()(const ActionImpl* e) const
{
    return UniquePDDLHashCombiner()(e->get_name(),
                                    e->get_parameters(),
                                    e->get_conditions<Static>(),
                                    e->get_conditions<Fluent>(),
                                    e->get_conditions<Derived>(),
                                    e->get_simple_effects(),
                                    e->get_complex_effects(),
                                    e->get_function_expression());
}

template<PredicateTag P>
size_t UniquePDDLHasher<const AtomImpl<P>*>::operator()(const AtomImpl<P>* e) const
{
    return UniquePDDLHashCombiner()(e->get_predicate(), e->get_terms());
}

template size_t UniquePDDLHasher<const AtomImpl<Static>*>::operator()(const AtomImpl<Static>* e) const;
template size_t UniquePDDLHasher<const AtomImpl<Fluent>*>::operator()(const AtomImpl<Fluent>* e) const;
template size_t UniquePDDLHasher<const AtomImpl<Derived>*>::operator()(const AtomImpl<Derived>* e) const;

size_t UniquePDDLHasher<const AxiomImpl*>::operator()(const AxiomImpl* e) const
{
    return UniquePDDLHashCombiner()(e->get_literal(),
                                    e->get_parameters(),
                                    e->get_conditions<Static>(),
                                    e->get_conditions<Fluent>(),
                                    e->get_conditions<Derived>());
}

size_t UniquePDDLHasher<const DomainImpl*>::operator()(const DomainImpl* e) const
{
    return UniquePDDLHashCombiner()(e->get_name(),
                                    e->get_requirements(),
                                    e->get_constants(),
                                    e->get_predicates<Static>(),
                                    e->get_predicates<Fluent>(),
                                    e->get_predicates<Derived>(),
                                    e->get_functions(),
                                    e->get_actions(),
                                    e->get_axioms());
}

size_t UniquePDDLHasher<const EffectSimpleImpl*>::operator()(const EffectSimpleImpl* e) const { return UniquePDDLHashCombiner()(e->get_effect()); }

size_t UniquePDDLHasher<const EffectComplexImpl*>::operator()(const EffectComplexImpl* e) const
{
    return UniquePDDLHashCombiner()(e->get_effect(),
                                    e->get_parameters(),
                                    e->get_conditions<Static>(),
                                    e->get_conditions<Fluent>(),
                                    e->get_conditions<Derived>());
}

size_t UniquePDDLHasher<const FunctionExpressionNumberImpl&>::operator()(const FunctionExpressionNumberImpl& e) const
{
    return UniquePDDLHashCombiner()(e.get_number());
}

size_t UniquePDDLHasher<const FunctionExpressionBinaryOperatorImpl&>::operator()(const FunctionExpressionBinaryOperatorImpl& e) const
{
    return UniquePDDLHashCombiner()(e.get_binary_operator(), e.get_left_function_expression(), e.get_right_function_expression());
}

size_t UniquePDDLHasher<const FunctionExpressionMultiOperatorImpl&>::operator()(const FunctionExpressionMultiOperatorImpl& e) const
{
    return UniquePDDLHashCombiner()(e.get_multi_operator(), e.get_function_expressions());
}

size_t UniquePDDLHasher<const FunctionExpressionMinusImpl&>::operator()(const FunctionExpressionMinusImpl& e) const
{
    return UniquePDDLHashCombiner()(e.get_function_expression());
}

size_t UniquePDDLHasher<const FunctionExpressionFunctionImpl&>::operator()(const FunctionExpressionFunctionImpl& e) const
{
    return UniquePDDLHashCombiner()(e.get_function());
}

size_t UniquePDDLHasher<const FunctionExpressionImpl*>::operator()(const FunctionExpressionImpl* e) const
{
    return std::visit([](const auto& arg) { return UniquePDDLHasher<decltype(arg)>()(arg); }, *e);
}

size_t UniquePDDLHasher<const FunctionSkeletonImpl*>::operator()(const FunctionSkeletonImpl* e) const
{
    return UniquePDDLHashCombiner()(e->get_name(), e->get_parameters());
}

size_t UniquePDDLHasher<const FunctionImpl*>::operator()(const FunctionImpl* e) const
{
    return UniquePDDLHashCombiner()(e->get_function_skeleton(), e->get_terms());
}

template<PredicateTag P>
size_t UniquePDDLHasher<const GroundAtomImpl<P>*>::operator()(const GroundAtomImpl<P>* e) const
{
    return UniquePDDLHashCombiner()(e->get_predicate(), e->get_objects());
}

template size_t UniquePDDLHasher<const GroundAtomImpl<Static>*>::operator()(const GroundAtomImpl<Static>* e) const;
template size_t UniquePDDLHasher<const GroundAtomImpl<Fluent>*>::operator()(const GroundAtomImpl<Fluent>* e) const;
template size_t UniquePDDLHasher<const GroundAtomImpl<Derived>*>::operator()(const GroundAtomImpl<Derived>* e) const;

size_t UniquePDDLHasher<const GroundFunctionExpressionNumberImpl&>::operator()(const GroundFunctionExpressionNumberImpl& e) const
{
    return UniquePDDLHashCombiner()(e.get_number());
}

size_t UniquePDDLHasher<const GroundFunctionExpressionBinaryOperatorImpl&>::operator()(const GroundFunctionExpressionBinaryOperatorImpl& e) const
{
    return UniquePDDLHashCombiner()(e.get_binary_operator(), e.get_left_function_expression(), e.get_right_function_expression());
}

size_t UniquePDDLHasher<const GroundFunctionExpressionMultiOperatorImpl&>::operator()(const GroundFunctionExpressionMultiOperatorImpl& e) const
{
    return UniquePDDLHashCombiner()(e.get_multi_operator(), e.get_function_expressions());
}

size_t UniquePDDLHasher<const GroundFunctionExpressionMinusImpl&>::operator()(const GroundFunctionExpressionMinusImpl& e) const
{
    return UniquePDDLHashCombiner()(e.get_function_expression());
}

size_t UniquePDDLHasher<const GroundFunctionExpressionFunctionImpl&>::operator()(const GroundFunctionExpressionFunctionImpl& e) const
{
    return UniquePDDLHashCombiner()(e.get_function());
}

size_t UniquePDDLHasher<const GroundFunctionExpressionImpl*>::operator()(const GroundFunctionExpressionImpl* e) const
{
    return std::visit([](const auto& arg) { return UniquePDDLHasher<decltype(arg)>()(arg); }, *e);
}

size_t UniquePDDLHasher<const GroundFunctionImpl*>::operator()(const GroundFunctionImpl* e) const
{
    return UniquePDDLHashCombiner()(e->get_function_skeleton(), e->get_objects());
}

template<PredicateTag P>
size_t UniquePDDLHasher<const GroundLiteralImpl<P>*>::operator()(const GroundLiteralImpl<P>* e) const
{
    return UniquePDDLHashCombiner()(e->is_negated(), e->get_atom());
}

template size_t UniquePDDLHasher<const GroundLiteralImpl<Static>*>::operator()(const GroundLiteralImpl<Static>* e) const;
template size_t UniquePDDLHasher<const GroundLiteralImpl<Fluent>*>::operator()(const GroundLiteralImpl<Fluent>* e) const;
template size_t UniquePDDLHasher<const GroundLiteralImpl<Derived>*>::operator()(const GroundLiteralImpl<Derived>* e) const;

template<PredicateTag P>
size_t UniquePDDLHasher<const LiteralImpl<P>*>::operator()(const LiteralImpl<P>* e) const
{
    return UniquePDDLHashCombiner()(e->is_negated(), e->get_atom());
}

template size_t UniquePDDLHasher<const LiteralImpl<Static>*>::operator()(const LiteralImpl<Static>* e) const;
template size_t UniquePDDLHasher<const LiteralImpl<Fluent>*>::operator()(const LiteralImpl<Fluent>* e) const;
template size_t UniquePDDLHasher<const LiteralImpl<Derived>*>::operator()(const LiteralImpl<Derived>* e) const;

size_t UniquePDDLHasher<const OptimizationMetricImpl*>::operator()(const OptimizationMetricImpl* e) const
{
    return UniquePDDLHashCombiner()(e->get_optimization_metric(), e->get_function_expression());
}

size_t UniquePDDLHasher<const NumericFluentImpl*>::operator()(const NumericFluentImpl* e) const
{
    return UniquePDDLHashCombiner()(e->get_number(), e->get_function());
}

size_t UniquePDDLHasher<const ObjectImpl*>::operator()(const ObjectImpl* e) const { return UniquePDDLHashCombiner()(e->get_name()); }

template<PredicateTag P>
size_t UniquePDDLHasher<const PredicateImpl<P>*>::operator()(const PredicateImpl<P>* e) const
{
    return UniquePDDLHashCombiner()(e->get_name(), e->get_parameters());
}

template size_t UniquePDDLHasher<const PredicateImpl<Static>*>::operator()(const PredicateImpl<Static>* e) const;
template size_t UniquePDDLHasher<const PredicateImpl<Fluent>*>::operator()(const PredicateImpl<Fluent>* e) const;
template size_t UniquePDDLHasher<const PredicateImpl<Derived>*>::operator()(const PredicateImpl<Derived>* e) const;

size_t UniquePDDLHasher<const ProblemImpl*>::operator()(const ProblemImpl* e) const
{
    return UniquePDDLHashCombiner()(e->get_name(),
                                    e->get_requirements(),
                                    e->get_domain(),
                                    e->get_objects(),
                                    e->get_derived_predicates(),
                                    e->get_static_initial_literals(),
                                    e->get_fluent_initial_literals(),
                                    e->get_numeric_fluents(),
                                    e->get_goal_condition<Static>(),
                                    e->get_goal_condition<Fluent>(),
                                    e->get_goal_condition<Derived>(),
                                    e->get_optimization_metric(),
                                    e->get_axioms());
}

size_t UniquePDDLHasher<const RequirementsImpl*>::operator()(const RequirementsImpl* e) const { return UniquePDDLHashCombiner()(e->get_requirements()); }

size_t UniquePDDLHasher<const TermObjectImpl&>::operator()(const TermObjectImpl& e) const { return UniquePDDLHashCombiner()(e.get_object()); }

size_t UniquePDDLHasher<const TermVariableImpl&>::operator()(const TermVariableImpl& e) const { return UniquePDDLHashCombiner()(e.get_variable()); }

size_t UniquePDDLHasher<const TermImpl*>::operator()(const TermImpl* e) const
{
    return std::visit([](const auto& arg) { return UniquePDDLHasher<decltype(arg)>()(arg); }, *e);
}

size_t UniquePDDLHasher<const VariableImpl*>::operator()(const VariableImpl* e) const
{
    return UniquePDDLHashCombiner()(e->get_name(), e->get_parameter_index());
}

}
