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
               && (l->get_simple_effects() == r->get_simple_effects()) && (l->get_conditional_effects() == r->get_conditional_effects())
               && (l->get_universal_effects() == r->get_universal_effects()) && (l->get_function_expression() == r->get_function_expression());
    }
    return true;
}

template<PredicateCategory P>
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

bool UniquePDDLEqualTo<const EffectConditionalImpl*>::operator()(const EffectConditionalImpl* l, const EffectConditionalImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_effect() == r->get_effect()) && (l->get_conditions<Static>() == r->get_conditions<Static>())
               && (l->get_conditions<Fluent>() == r->get_conditions<Fluent>()) && (l->get_conditions<Derived>() == r->get_conditions<Derived>());
    }
    return true;
}

bool UniquePDDLEqualTo<const EffectUniversalImpl*>::operator()(const EffectUniversalImpl* l, const EffectUniversalImpl* r) const
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
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const FunctionExpressionBinaryOperatorImpl&>::operator()(const FunctionExpressionBinaryOperatorImpl& l,
                                                                                const FunctionExpressionBinaryOperatorImpl& r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const FunctionExpressionMultiOperatorImpl&>::operator()(const FunctionExpressionMultiOperatorImpl& l,
                                                                               const FunctionExpressionMultiOperatorImpl& r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const FunctionExpressionMinusImpl&>::operator()(const FunctionExpressionMinusImpl& l, const FunctionExpressionMinusImpl& r) const {}

bool UniquePDDLEqualTo<const FunctionExpressionFunctionImpl&>::operator()(const FunctionExpressionFunctionImpl& l,
                                                                          const FunctionExpressionFunctionImpl& r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const FunctionExpressionImpl*>::operator()(const FunctionExpressionImpl* l, const FunctionExpressionImpl* r) const
{
    return UniquePDDLEqualTo<FunctionExpressionImpl>()(*l, *r);
}

bool UniquePDDLEqualTo<const FunctionSkeletonImpl*>::operator()(const FunctionSkeletonImpl* l, const FunctionSkeletonImpl* r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const FunctionImpl*>::operator()(const FunctionImpl* l, const FunctionImpl* r) const
{
    if (&l != &r) {}
    return true;
}

template<PredicateCategory P>
bool UniquePDDLEqualTo<const GroundAtomImpl<P>*>::operator()(const GroundAtomImpl<P>* l, const GroundAtomImpl<P>* r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const GroundFunctionExpressionNumberImpl&>::operator()(const GroundFunctionExpressionNumberImpl& l,
                                                                              const GroundFunctionExpressionNumberImpl& r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const GroundFunctionExpressionBinaryOperatorImpl&>::operator()(const GroundFunctionExpressionBinaryOperatorImpl& l,
                                                                                      const GroundFunctionExpressionBinaryOperatorImpl& r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const GroundFunctionExpressionMultiOperatorImpl&>::operator()(const GroundFunctionExpressionMultiOperatorImpl& l,
                                                                                     const GroundFunctionExpressionMultiOperatorImpl& r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const GroundFunctionExpressionMinusImpl&>::operator()(const GroundFunctionExpressionMinusImpl& l,
                                                                             const GroundFunctionExpressionMinusImpl& r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const GroundFunctionExpressionFunctionImpl&>::operator()(const GroundFunctionExpressionFunctionImpl& l,
                                                                                const GroundFunctionExpressionFunctionImpl& r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const GroundFunctionExpressionImpl*>::operator()(const GroundFunctionExpressionImpl* l, const GroundFunctionExpressionImpl* r) const
{
    return UniquePDDLEqualTo<GroundFunctionExpressionImpl>()(*l, *r);
}

bool UniquePDDLEqualTo<const GroundFunctionImpl*>::operator()(const GroundFunctionImpl* l, const GroundFunctionImpl* r) const
{
    if (&l != &r) {}
    return true;
}

template<PredicateCategory P>
bool UniquePDDLEqualTo<const GroundLiteralImpl<P>*>::operator()(const GroundLiteralImpl<P>* l, const GroundLiteralImpl<P>* r) const
{
    if (&l != &r) {}
    return true;
}

template<PredicateCategory P>
bool UniquePDDLEqualTo<const LiteralImpl<P>*>::operator()(const LiteralImpl<P>* l, const LiteralImpl<P>* r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const OptimizationMetricImpl*>::operator()(const OptimizationMetricImpl* l, const OptimizationMetricImpl* r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const NumericFluentImpl*>::operator()(const NumericFluentImpl* l, const NumericFluentImpl* r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const ObjectImpl*>::operator()(const ObjectImpl* l, const ObjectImpl* r) const
{
    if (&l != &r) {}
    return true;
}

template<PredicateCategory P>
bool UniquePDDLEqualTo<const PredicateImpl<P>*>::operator()(const PredicateImpl<P>* l, const PredicateImpl<P>* r) const
{
    if (&l != &r) {}
    return true;
}

bool UniquePDDLEqualTo<const ProblemImpl*>::operator()(const ProblemImpl* l, const ProblemImpl* r) const
{
    if (&l != &r) {}
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
