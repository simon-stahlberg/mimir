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
bool UniquePDDLEqualTo<const ActionImpl*>::operator()(const ActionImpl* l, const ActionImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name()) && (l->get_parameters() == r->get_parameters()) && (l->get_conditions<Static>() == r->get_conditions<Static>())
               && (l->get_conditions<Fluent>() == r->get_conditions<Fluent>()) && (l->get_conditions<Derived>() == r->get_conditions<Derived>())
               && (l->get_simple_effects() == r->get_simple_effects()) && (l->get_complex_effects() == r->get_complex_effects())
               && (l->get_function_expression() == r->get_function_expression());
    }
    return true;
}

template<PredicateTag P>
bool UniquePDDLEqualTo<const AtomImpl<P>*>::operator()(const AtomImpl<P>* l, const AtomImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate()) && (l->get_terms() == r->get_terms());
    }
    return true;
}

template bool UniquePDDLEqualTo<const AtomImpl<Static>*>::operator()(const AtomImpl<Static>* l, const AtomImpl<Static>* r) const;
template bool UniquePDDLEqualTo<const AtomImpl<Fluent>*>::operator()(const AtomImpl<Fluent>* l, const AtomImpl<Fluent>* r) const;
template bool UniquePDDLEqualTo<const AtomImpl<Derived>*>::operator()(const AtomImpl<Derived>* l, const AtomImpl<Derived>* r) const;

bool UniquePDDLEqualTo<const AxiomImpl*>::operator()(const AxiomImpl* l, const AxiomImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_literal() == r->get_literal()) && (l->get_parameters() == r->get_parameters())
               && (l->get_conditions<Static>() == r->get_conditions<Static>()) && (l->get_conditions<Fluent>() == r->get_conditions<Fluent>())
               && (l->get_conditions<Derived>() == r->get_conditions<Derived>());
    }
    return true;
}

bool UniquePDDLEqualTo<const DomainImpl*>::operator()(const DomainImpl* l, const DomainImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name()) && (l->get_requirements() == r->get_requirements()) && (l->get_constants() == r->get_constants())
               && (l->get_predicates<Static>() == r->get_predicates<Static>()) && (l->get_predicates<Fluent>() == r->get_predicates<Fluent>())
               && (l->get_predicates<Derived>() == r->get_predicates<Derived>()) && (l->get_functions() == r->get_functions())
               && (l->get_actions() == r->get_actions()) && (l->get_axioms() == r->get_axioms());
    }
    return true;
}

bool UniquePDDLEqualTo<const EffectSimpleImpl*>::operator()(const EffectSimpleImpl* l, const EffectSimpleImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_effect() == r->get_effect());
    }
    return true;
}

bool UniquePDDLEqualTo<const EffectComplexImpl*>::operator()(const EffectComplexImpl* l, const EffectComplexImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_effect() == r->get_effect()) && (l->get_parameters() == r->get_parameters())
               && (l->get_conditions<Static>() == r->get_conditions<Static>()) && (l->get_conditions<Fluent>() == r->get_conditions<Fluent>())
               && (l->get_conditions<Derived>() == r->get_conditions<Derived>());
    }
    return true;
}

bool UniquePDDLEqualTo<const FunctionExpressionNumberImpl&>::operator()(const FunctionExpressionNumberImpl& l, const FunctionExpressionNumberImpl& r) const
{
    if (&l != &r)
    {
        return (l.get_number() == r.get_number());
    }
    return true;
}

bool UniquePDDLEqualTo<const FunctionExpressionBinaryOperatorImpl&>::operator()(const FunctionExpressionBinaryOperatorImpl& l,
                                                                                const FunctionExpressionBinaryOperatorImpl& r) const
{
    if (&l != &r)
    {
        return (l.get_binary_operator() == r.get_binary_operator()) && (l.get_left_function_expression() == r.get_left_function_expression())
               && (l.get_right_function_expression() == r.get_right_function_expression());
    }
    return true;
}

bool UniquePDDLEqualTo<const FunctionExpressionMultiOperatorImpl&>::operator()(const FunctionExpressionMultiOperatorImpl& l,
                                                                               const FunctionExpressionMultiOperatorImpl& r) const
{
    if (&l != &r)
    {
        return (l.get_multi_operator() == r.get_multi_operator()) && (l.get_function_expressions() == r.get_function_expressions());
    }
    return true;
}

bool UniquePDDLEqualTo<const FunctionExpressionMinusImpl&>::operator()(const FunctionExpressionMinusImpl& l, const FunctionExpressionMinusImpl& r) const
{
    if (&l != &r)
    {
        return (l.get_function_expression() == r.get_function_expression());
    }
    return true;
}

bool UniquePDDLEqualTo<const FunctionExpressionFunctionImpl&>::operator()(const FunctionExpressionFunctionImpl& l,
                                                                          const FunctionExpressionFunctionImpl& r) const
{
    if (&l != &r)
    {
        return (l.get_function() == r.get_function());
    }
    return true;
}

bool UniquePDDLEqualTo<const FunctionExpressionImpl*>::operator()(const FunctionExpressionImpl* l, const FunctionExpressionImpl* r) const
{
    return UniquePDDLEqualTo<FunctionExpressionImpl>()(*l, *r);
}

bool UniquePDDLEqualTo<const FunctionSkeletonImpl*>::operator()(const FunctionSkeletonImpl* l, const FunctionSkeletonImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name()) && (l->get_parameters() == r->get_parameters());
    }
    return true;
}

bool UniquePDDLEqualTo<const FunctionImpl*>::operator()(const FunctionImpl* l, const FunctionImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_function_skeleton() == r->get_function_skeleton()) && (l->get_terms() == r->get_terms());
    }
    return true;
}

template<PredicateTag P>
bool UniquePDDLEqualTo<const GroundAtomImpl<P>*>::operator()(const GroundAtomImpl<P>* l, const GroundAtomImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate()) && (l->get_objects() == r->get_objects());
    }
    return true;
}

template bool UniquePDDLEqualTo<const GroundAtomImpl<Static>*>::operator()(const GroundAtomImpl<Static>* l, const GroundAtomImpl<Static>* r) const;
template bool UniquePDDLEqualTo<const GroundAtomImpl<Fluent>*>::operator()(const GroundAtomImpl<Fluent>* l, const GroundAtomImpl<Fluent>* r) const;
template bool UniquePDDLEqualTo<const GroundAtomImpl<Derived>*>::operator()(const GroundAtomImpl<Derived>* l, const GroundAtomImpl<Derived>* r) const;

bool UniquePDDLEqualTo<const GroundFunctionExpressionNumberImpl&>::operator()(const GroundFunctionExpressionNumberImpl& l,
                                                                              const GroundFunctionExpressionNumberImpl& r) const
{
    if (&l != &r)
    {
        return (l.get_number() == r.get_number());
    }
    return true;
}

bool UniquePDDLEqualTo<const GroundFunctionExpressionBinaryOperatorImpl&>::operator()(const GroundFunctionExpressionBinaryOperatorImpl& l,
                                                                                      const GroundFunctionExpressionBinaryOperatorImpl& r) const
{
    if (&l != &r)
    {
        return (l.get_binary_operator() == r.get_binary_operator()) && (l.get_left_function_expression() == r.get_left_function_expression())
               && (l.get_right_function_expression() == r.get_right_function_expression());
    }
    return true;
}

bool UniquePDDLEqualTo<const GroundFunctionExpressionMultiOperatorImpl&>::operator()(const GroundFunctionExpressionMultiOperatorImpl& l,
                                                                                     const GroundFunctionExpressionMultiOperatorImpl& r) const
{
    if (&l != &r)
    {
        return (l.get_multi_operator() == r.get_multi_operator()) && (l.get_function_expressions() == r.get_function_expressions());
    }
    return true;
}

bool UniquePDDLEqualTo<const GroundFunctionExpressionMinusImpl&>::operator()(const GroundFunctionExpressionMinusImpl& l,
                                                                             const GroundFunctionExpressionMinusImpl& r) const
{
    if (&l != &r)
    {
        return (l.get_function_expression() == r.get_function_expression());
    }
    return true;
}

bool UniquePDDLEqualTo<const GroundFunctionExpressionFunctionImpl&>::operator()(const GroundFunctionExpressionFunctionImpl& l,
                                                                                const GroundFunctionExpressionFunctionImpl& r) const
{
    if (&l != &r)
    {
        return (l.get_function() == r.get_function());
    }
    return true;
}

bool UniquePDDLEqualTo<const GroundFunctionExpressionImpl*>::operator()(const GroundFunctionExpressionImpl* l, const GroundFunctionExpressionImpl* r) const
{
    return UniquePDDLEqualTo<GroundFunctionExpressionImpl>()(*l, *r);
}

bool UniquePDDLEqualTo<const GroundFunctionImpl*>::operator()(const GroundFunctionImpl* l, const GroundFunctionImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_function_skeleton() == r->get_function_skeleton() && l->get_objects() == r->get_objects());
    }
    return true;
}

template<PredicateTag P>
bool UniquePDDLEqualTo<const GroundLiteralImpl<P>*>::operator()(const GroundLiteralImpl<P>* l, const GroundLiteralImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->is_negated() == r->is_negated()) && (l->get_atom() == r->get_atom());
    }
    return true;
}

template bool UniquePDDLEqualTo<const GroundLiteralImpl<Static>*>::operator()(const GroundLiteralImpl<Static>* l, const GroundLiteralImpl<Static>* r) const;
template bool UniquePDDLEqualTo<const GroundLiteralImpl<Fluent>*>::operator()(const GroundLiteralImpl<Fluent>* l, const GroundLiteralImpl<Fluent>* r) const;
template bool UniquePDDLEqualTo<const GroundLiteralImpl<Derived>*>::operator()(const GroundLiteralImpl<Derived>* l, const GroundLiteralImpl<Derived>* r) const;

template<PredicateTag P>
bool UniquePDDLEqualTo<const LiteralImpl<P>*>::operator()(const LiteralImpl<P>* l, const LiteralImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->is_negated() == r->is_negated()) && (l->get_atom() == r->get_atom());
    }
    return true;
}

template bool UniquePDDLEqualTo<const LiteralImpl<Static>*>::operator()(const LiteralImpl<Static>* l, const LiteralImpl<Static>* r) const;
template bool UniquePDDLEqualTo<const LiteralImpl<Fluent>*>::operator()(const LiteralImpl<Fluent>* l, const LiteralImpl<Fluent>* r) const;
template bool UniquePDDLEqualTo<const LiteralImpl<Derived>*>::operator()(const LiteralImpl<Derived>* l, const LiteralImpl<Derived>* r) const;

bool UniquePDDLEqualTo<const OptimizationMetricImpl*>::operator()(const OptimizationMetricImpl* l, const OptimizationMetricImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_optimization_metric() == r->get_optimization_metric()) && (l->get_function_expression() == r->get_function_expression());
    }
    return true;
}

bool UniquePDDLEqualTo<const NumericFluentImpl*>::operator()(const NumericFluentImpl* l, const NumericFluentImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_number() == r->get_number()) && (l->get_function() == r->get_function());
    }
    return true;
}

bool UniquePDDLEqualTo<const ObjectImpl*>::operator()(const ObjectImpl* l, const ObjectImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name());
    }
    return true;
}

template<PredicateTag P>
bool UniquePDDLEqualTo<const PredicateImpl<P>*>::operator()(const PredicateImpl<P>* l, const PredicateImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name()) && (l->get_parameters() == r->get_parameters());
    }
    return true;
}

template bool UniquePDDLEqualTo<const PredicateImpl<Static>*>::operator()(const PredicateImpl<Static>* l, const PredicateImpl<Static>* r) const;
template bool UniquePDDLEqualTo<const PredicateImpl<Fluent>*>::operator()(const PredicateImpl<Fluent>* l, const PredicateImpl<Fluent>* r) const;
template bool UniquePDDLEqualTo<const PredicateImpl<Derived>*>::operator()(const PredicateImpl<Derived>* l, const PredicateImpl<Derived>* r) const;

bool UniquePDDLEqualTo<const ProblemImpl*>::operator()(const ProblemImpl* l, const ProblemImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name()) && (l->get_requirements() == r->get_requirements()) && (l->get_domain() == r->get_domain())
               && (l->get_objects() == r->get_objects()) && (l->get_derived_predicates() == r->get_derived_predicates())
               && (l->get_static_initial_literals() == r->get_static_initial_literals())
               && (l->get_fluent_initial_literals() == r->get_fluent_initial_literals()) && (l->get_numeric_fluents() == r->get_numeric_fluents())
               && (l->get_goal_condition<Static>() == r->get_goal_condition<Static>()) && (l->get_goal_condition<Fluent>() == r->get_goal_condition<Fluent>())
               && (l->get_goal_condition<Derived>() == r->get_goal_condition<Derived>()) && (l->get_optimization_metric() == r->get_optimization_metric())
               && (l->get_axioms() == r->get_axioms());
    }
    return true;
}

bool UniquePDDLEqualTo<const RequirementsImpl*>::operator()(const RequirementsImpl* l, const RequirementsImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_requirements() == r->get_requirements());
    }
    return true;
}

bool UniquePDDLEqualTo<const TermObjectImpl&>::operator()(const TermObjectImpl& l, const TermObjectImpl& r) const
{
    if (&l != &r)
    {
        return (l.get_object() == r.get_object());
    }
    return true;
}

bool UniquePDDLEqualTo<const TermVariableImpl&>::operator()(const TermVariableImpl& l, const TermVariableImpl& r) const
{
    if (&l != &r)
    {
        return (l.get_variable() == r.get_variable());
    }
    return true;
}

bool UniquePDDLEqualTo<const TermImpl*>::operator()(const TermImpl* l, const TermImpl* r) const { return UniquePDDLEqualTo<TermImpl>()(*l, *r); }

bool UniquePDDLEqualTo<const VariableImpl*>::operator()(const VariableImpl* l, const VariableImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name()) && (l->get_parameter_index() == r->get_parameter_index());
    }
    return true;
}

}
