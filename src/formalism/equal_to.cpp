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
bool UniquePDDLEqualTo<Action>::operator()(Action l, Action r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name()) && (l->get_parameters() == r->get_parameters()) && (l->get_conditions<Static>() == r->get_conditions<Static>())
               && (l->get_conditions<Fluent>() == r->get_conditions<Fluent>()) && (l->get_conditions<Derived>() == r->get_conditions<Derived>())
               && (l->get_strips_effect() == r->get_strips_effect()) && (l->get_conditional_effects() == r->get_conditional_effects());
    }
    return true;
}

template<PredicateTag P>
bool UniquePDDLEqualTo<Atom<P>>::operator()(Atom<P> l, Atom<P> r) const
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

bool UniquePDDLEqualTo<Axiom>::operator()(Axiom l, Axiom r) const
{
    if (&l != &r)
    {
        return (l->get_literal() == r->get_literal()) && (l->get_parameters() == r->get_parameters())
               && (l->get_conditions<Static>() == r->get_conditions<Static>()) && (l->get_conditions<Fluent>() == r->get_conditions<Fluent>())
               && (l->get_conditions<Derived>() == r->get_conditions<Derived>());
    }
    return true;
}

bool UniquePDDLEqualTo<Domain>::operator()(Domain l, Domain r) const
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

bool UniquePDDLEqualTo<EffectStrips>::operator()(EffectStrips l, EffectStrips r) const
{
    if (&l != &r)
    {
        return (l->get_effects() == r->get_effects()) && (l->get_function_expression() == r->get_function_expression());
    }
    return true;
}

bool UniquePDDLEqualTo<EffectConditional>::operator()(EffectConditional l, EffectConditional r) const
{
    if (&l != &r)
    {
        return (l->get_effects() == r->get_effects()) && (l->get_parameters() == r->get_parameters())
               && (l->get_conditions<Static>() == r->get_conditions<Static>()) && (l->get_conditions<Fluent>() == r->get_conditions<Fluent>())
               && (l->get_conditions<Derived>() == r->get_conditions<Derived>()) && (l->get_function_expression() == r->get_function_expression());
    }
    return true;
}

bool UniquePDDLEqualTo<FunctionExpressionNumber>::operator()(FunctionExpressionNumber l, FunctionExpressionNumber r) const
{
    if (&l != &r)
    {
        return (l->get_number() == r->get_number());
    }
    return true;
}

bool UniquePDDLEqualTo<FunctionExpressionBinaryOperator>::operator()(FunctionExpressionBinaryOperator l, FunctionExpressionBinaryOperator r) const
{
    if (&l != &r)
    {
        return (l->get_binary_operator() == r->get_binary_operator()) && (l->get_left_function_expression() == r->get_left_function_expression())
               && (l->get_right_function_expression() == r->get_right_function_expression());
    }
    return true;
}

bool UniquePDDLEqualTo<FunctionExpressionMultiOperator>::operator()(FunctionExpressionMultiOperator l, FunctionExpressionMultiOperator r) const
{
    if (&l != &r)
    {
        return (l->get_multi_operator() == r->get_multi_operator()) && (l->get_function_expressions() == r->get_function_expressions());
    }
    return true;
}

bool UniquePDDLEqualTo<FunctionExpressionMinus>::operator()(FunctionExpressionMinus l, FunctionExpressionMinus r) const
{
    if (&l != &r)
    {
        return (l->get_function_expression() == r->get_function_expression());
    }
    return true;
}

bool UniquePDDLEqualTo<FunctionExpressionFunction>::operator()(FunctionExpressionFunction l, FunctionExpressionFunction r) const
{
    if (&l != &r)
    {
        return (l->get_function() == r->get_function());
    }
    return true;
}

bool UniquePDDLEqualTo<FunctionExpression>::operator()(FunctionExpression l, FunctionExpression r) const { return l->get_variant() == r->get_variant(); }

bool UniquePDDLEqualTo<FunctionSkeleton>::operator()(FunctionSkeleton l, FunctionSkeleton r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name()) && (l->get_parameters() == r->get_parameters());
    }
    return true;
}

bool UniquePDDLEqualTo<Function>::operator()(Function l, Function r) const
{
    if (&l != &r)
    {
        return (l->get_function_skeleton() == r->get_function_skeleton()) && (l->get_terms() == r->get_terms());
    }
    return true;
}

template<PredicateTag P>
bool UniquePDDLEqualTo<GroundAtom<P>>::operator()(GroundAtom<P> l, GroundAtom<P> r) const
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

bool UniquePDDLEqualTo<GroundFunctionExpressionNumber>::operator()(GroundFunctionExpressionNumber l, GroundFunctionExpressionNumber r) const
{
    if (&l != &r)
    {
        return (l->get_number() == r->get_number());
    }
    return true;
}

bool UniquePDDLEqualTo<GroundFunctionExpressionBinaryOperator>::operator()(GroundFunctionExpressionBinaryOperator l,
                                                                           GroundFunctionExpressionBinaryOperator r) const
{
    if (&l != &r)
    {
        return (l->get_binary_operator() == r->get_binary_operator()) && (l->get_left_function_expression() == r->get_left_function_expression())
               && (l->get_right_function_expression() == r->get_right_function_expression());
    }
    return true;
}

bool UniquePDDLEqualTo<GroundFunctionExpressionMultiOperator>::operator()(GroundFunctionExpressionMultiOperator l,
                                                                          GroundFunctionExpressionMultiOperator r) const
{
    if (&l != &r)
    {
        return (l->get_multi_operator() == r->get_multi_operator()) && (l->get_function_expressions() == r->get_function_expressions());
    }
    return true;
}

bool UniquePDDLEqualTo<GroundFunctionExpressionMinus>::operator()(GroundFunctionExpressionMinus l, GroundFunctionExpressionMinus r) const
{
    if (&l != &r)
    {
        return (l->get_function_expression() == r->get_function_expression());
    }
    return true;
}

bool UniquePDDLEqualTo<GroundFunctionExpressionFunction>::operator()(GroundFunctionExpressionFunction l, GroundFunctionExpressionFunction r) const
{
    if (&l != &r)
    {
        return (l->get_function() == r->get_function());
    }
    return true;
}

bool UniquePDDLEqualTo<GroundFunctionExpression>::operator()(GroundFunctionExpression l, GroundFunctionExpression r) const
{
    return l->get_variant() == r->get_variant();
}

bool UniquePDDLEqualTo<GroundFunction>::operator()(GroundFunction l, GroundFunction r) const
{
    if (&l != &r)
    {
        return (l->get_function_skeleton() == r->get_function_skeleton() && l->get_objects() == r->get_objects());
    }
    return true;
}

template<PredicateTag P>
bool UniquePDDLEqualTo<GroundLiteral<P>>::operator()(GroundLiteral<P> l, GroundLiteral<P> r) const
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
bool UniquePDDLEqualTo<Literal<P>>::operator()(Literal<P> l, Literal<P> r) const
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

bool UniquePDDLEqualTo<OptimizationMetric>::operator()(OptimizationMetric l, OptimizationMetric r) const
{
    if (&l != &r)
    {
        return (l->get_optimization_metric() == r->get_optimization_metric()) && (l->get_function_expression() == r->get_function_expression());
    }
    return true;
}

bool UniquePDDLEqualTo<NumericFluent>::operator()(NumericFluent l, NumericFluent r) const
{
    if (&l != &r)
    {
        return (l->get_number() == r->get_number()) && (l->get_function() == r->get_function());
    }
    return true;
}

bool UniquePDDLEqualTo<Object>::operator()(Object l, Object r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name());
    }
    return true;
}

template<PredicateTag P>
bool UniquePDDLEqualTo<Predicate<P>>::operator()(Predicate<P> l, Predicate<P> r) const
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

bool UniquePDDLEqualTo<Problem>::operator()(Problem l, Problem r) const
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

bool UniquePDDLEqualTo<Requirements>::operator()(Requirements l, Requirements r) const
{
    if (&l != &r)
    {
        return (l->get_requirements() == r->get_requirements());
    }
    return true;
}

bool UniquePDDLEqualTo<Term>::operator()(Term l, Term r) const { return l->get_variant() == r->get_variant(); }

bool UniquePDDLEqualTo<Variable>::operator()(Variable l, Variable r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name()) && (l->get_parameter_index() == r->get_parameter_index());
    }
    return true;
}

}
