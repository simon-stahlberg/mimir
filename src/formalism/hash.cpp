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
size_t UniquePDDLHasher<Action>::operator()(Action e) const
{
    return UniquePDDLHashCombiner()(e->get_name(),
                                    e->get_parameters(),
                                    e->get_conditions<Static>(),
                                    e->get_conditions<Fluent>(),
                                    e->get_conditions<Derived>(),
                                    e->get_strips_effect(),
                                    e->get_conditional_effects());
}

template<PredicateTag P>
size_t UniquePDDLHasher<Atom<P>>::operator()(Atom<P> e) const
{
    return UniquePDDLHashCombiner()(e->get_predicate(), e->get_terms());
}

template size_t UniquePDDLHasher<const AtomImpl<Static>*>::operator()(const AtomImpl<Static>* e) const;
template size_t UniquePDDLHasher<const AtomImpl<Fluent>*>::operator()(const AtomImpl<Fluent>* e) const;
template size_t UniquePDDLHasher<const AtomImpl<Derived>*>::operator()(const AtomImpl<Derived>* e) const;

size_t UniquePDDLHasher<Axiom>::operator()(Axiom e) const
{
    return UniquePDDLHashCombiner()(e->get_literal(),
                                    e->get_parameters(),
                                    e->get_conditions<Static>(),
                                    e->get_conditions<Fluent>(),
                                    e->get_conditions<Derived>());
}

size_t UniquePDDLHasher<Domain>::operator()(Domain e) const
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

size_t UniquePDDLHasher<EffectStrips>::operator()(EffectStrips e) const { return UniquePDDLHashCombiner()(e->get_effects(), e->get_function_expression()); }

size_t UniquePDDLHasher<EffectConditional>::operator()(EffectConditional e) const
{
    return UniquePDDLHashCombiner()(e->get_effects(),
                                    e->get_parameters(),
                                    e->get_conditions<Static>(),
                                    e->get_conditions<Fluent>(),
                                    e->get_conditions<Derived>(),
                                    e->get_function_expression());
}

size_t UniquePDDLHasher<FunctionExpressionNumber>::operator()(FunctionExpressionNumber e) const { return UniquePDDLHashCombiner()(e->get_number()); }

size_t UniquePDDLHasher<FunctionExpressionBinaryOperator>::operator()(FunctionExpressionBinaryOperator e) const
{
    return UniquePDDLHashCombiner()(e->get_binary_operator(), e->get_left_function_expression(), e->get_right_function_expression());
}

size_t UniquePDDLHasher<FunctionExpressionMultiOperator>::operator()(FunctionExpressionMultiOperator e) const
{
    return UniquePDDLHashCombiner()(e->get_multi_operator(), e->get_function_expressions());
}

size_t UniquePDDLHasher<FunctionExpressionMinus>::operator()(FunctionExpressionMinus e) const { return UniquePDDLHashCombiner()(e->get_function_expression()); }

size_t UniquePDDLHasher<FunctionExpressionFunction>::operator()(FunctionExpressionFunction e) const { return UniquePDDLHashCombiner()(e->get_function()); }

size_t UniquePDDLHasher<FunctionExpression>::operator()(FunctionExpression e) const { return UniquePDDLHashCombiner()(e->get_variant()); }

size_t UniquePDDLHasher<FunctionSkeleton>::operator()(FunctionSkeleton e) const { return UniquePDDLHashCombiner()(e->get_name(), e->get_parameters()); }

size_t UniquePDDLHasher<Function>::operator()(Function e) const { return UniquePDDLHashCombiner()(e->get_function_skeleton(), e->get_terms()); }

template<PredicateTag P>
size_t UniquePDDLHasher<GroundAtom<P>>::operator()(GroundAtom<P> e) const
{
    return UniquePDDLHashCombiner()(e->get_predicate(), e->get_objects());
}

template size_t UniquePDDLHasher<const GroundAtomImpl<Static>*>::operator()(const GroundAtomImpl<Static>* e) const;
template size_t UniquePDDLHasher<const GroundAtomImpl<Fluent>*>::operator()(const GroundAtomImpl<Fluent>* e) const;
template size_t UniquePDDLHasher<const GroundAtomImpl<Derived>*>::operator()(const GroundAtomImpl<Derived>* e) const;

size_t UniquePDDLHasher<GroundFunctionExpressionNumber>::operator()(GroundFunctionExpressionNumber e) const
{
    return UniquePDDLHashCombiner()(e->get_number());
}

size_t UniquePDDLHasher<GroundFunctionExpressionBinaryOperator>::operator()(GroundFunctionExpressionBinaryOperator e) const
{
    return UniquePDDLHashCombiner()(e->get_binary_operator(), e->get_left_function_expression(), e->get_right_function_expression());
}

size_t UniquePDDLHasher<GroundFunctionExpressionMultiOperator>::operator()(GroundFunctionExpressionMultiOperator e) const
{
    return UniquePDDLHashCombiner()(e->get_multi_operator(), e->get_function_expressions());
}

size_t UniquePDDLHasher<GroundFunctionExpressionMinus>::operator()(GroundFunctionExpressionMinus e) const
{
    return UniquePDDLHashCombiner()(e->get_function_expression());
}

size_t UniquePDDLHasher<GroundFunctionExpressionFunction>::operator()(GroundFunctionExpressionFunction e) const
{
    return UniquePDDLHashCombiner()(e->get_function());
}

size_t UniquePDDLHasher<GroundFunctionExpression>::operator()(GroundFunctionExpression e) const { return UniquePDDLHashCombiner()(e->get_variant()); }

size_t UniquePDDLHasher<GroundFunction>::operator()(GroundFunction e) const { return UniquePDDLHashCombiner()(e->get_function_skeleton(), e->get_objects()); }

template<PredicateTag P>
size_t UniquePDDLHasher<GroundLiteral<P>>::operator()(GroundLiteral<P> e) const
{
    return UniquePDDLHashCombiner()(e->is_negated(), e->get_atom());
}

template size_t UniquePDDLHasher<const GroundLiteralImpl<Static>*>::operator()(const GroundLiteralImpl<Static>* e) const;
template size_t UniquePDDLHasher<const GroundLiteralImpl<Fluent>*>::operator()(const GroundLiteralImpl<Fluent>* e) const;
template size_t UniquePDDLHasher<const GroundLiteralImpl<Derived>*>::operator()(const GroundLiteralImpl<Derived>* e) const;

template<PredicateTag P>
size_t UniquePDDLHasher<Literal<P>>::operator()(Literal<P> e) const
{
    return UniquePDDLHashCombiner()(e->is_negated(), e->get_atom());
}

template size_t UniquePDDLHasher<const LiteralImpl<Static>*>::operator()(const LiteralImpl<Static>* e) const;
template size_t UniquePDDLHasher<const LiteralImpl<Fluent>*>::operator()(const LiteralImpl<Fluent>* e) const;
template size_t UniquePDDLHasher<const LiteralImpl<Derived>*>::operator()(const LiteralImpl<Derived>* e) const;

size_t UniquePDDLHasher<OptimizationMetric>::operator()(OptimizationMetric e) const
{
    return UniquePDDLHashCombiner()(e->get_optimization_metric(), e->get_function_expression());
}

size_t UniquePDDLHasher<NumericFluent>::operator()(NumericFluent e) const { return UniquePDDLHashCombiner()(e->get_number(), e->get_function()); }

size_t UniquePDDLHasher<Object>::operator()(Object e) const { return UniquePDDLHashCombiner()(e->get_name()); }

template<PredicateTag P>
size_t UniquePDDLHasher<Predicate<P>>::operator()(Predicate<P> e) const
{
    return UniquePDDLHashCombiner()(e->get_name(), e->get_parameters());
}

template size_t UniquePDDLHasher<const PredicateImpl<Static>*>::operator()(const PredicateImpl<Static>* e) const;
template size_t UniquePDDLHasher<const PredicateImpl<Fluent>*>::operator()(const PredicateImpl<Fluent>* e) const;
template size_t UniquePDDLHasher<const PredicateImpl<Derived>*>::operator()(const PredicateImpl<Derived>* e) const;

size_t UniquePDDLHasher<Problem>::operator()(Problem e) const
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

size_t UniquePDDLHasher<Requirements>::operator()(Requirements e) const { return UniquePDDLHashCombiner()(e->get_requirements()); }

size_t UniquePDDLHasher<Term>::operator()(Term e) const { return UniquePDDLHashCombiner()(e->get_variant()); }

size_t UniquePDDLHasher<Variable>::operator()(Variable e) const { return UniquePDDLHashCombiner()(e->get_name(), e->get_parameter_index()); }

}
