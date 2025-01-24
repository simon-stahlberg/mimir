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

#include "mimir/formalism/translators/to_mimir_structures.hpp"

#include "mimir/common/collections.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/translators/utils.hpp"

#include <loki/details/utils/hash.hpp>
#include <numeric>
#include <typeinfo>
#include <unordered_map>

namespace mimir
{
/**
 * Prepare common
 */
void ToMimirStructures::prepare_common(const loki::FunctionSkeletonImpl& function_skeleton)
{
    prepare_common(function_skeleton.get_parameters());
    prepare_common(*function_skeleton.get_type());
}
void ToMimirStructures::prepare_common(const loki::ObjectImpl& object) { prepare_common(object.get_bases()); }
void ToMimirStructures::prepare_common(const loki::ParameterImpl& parameter) { prepare_common(*parameter.get_variable()); }
void ToMimirStructures::prepare_common(const loki::PredicateImpl& predicate) { prepare_common(predicate.get_parameters()); }
void ToMimirStructures::prepare_common(const loki::RequirementsImpl& requirements) {}
void ToMimirStructures::prepare_common(const loki::TypeImpl& type) { prepare_common(type.get_bases()); }
void ToMimirStructures::prepare_common(const loki::VariableImpl& variable) {}

/**
 * Prepare lifted
 */

void ToMimirStructures::prepare_lifted(const loki::TermImpl& term)
{
    std::visit([this](auto&& arg) { return this->prepare_common(*arg); }, term.get_object_or_variable());
}

void ToMimirStructures::prepare_lifted(const loki::AtomImpl& atom)
{
    prepare_common(*atom.get_predicate());
    prepare_lifted(atom.get_terms());
}
void ToMimirStructures::prepare_lifted(const loki::LiteralImpl& literal) { prepare_lifted(*literal.get_atom()); }
void ToMimirStructures::prepare_lifted(const loki::FunctionExpressionNumberImpl& function_expression) {}
void ToMimirStructures::prepare_lifted(const loki::FunctionExpressionBinaryOperatorImpl& function_expression)
{
    prepare_lifted(*function_expression.get_left_function_expression());
    prepare_lifted(*function_expression.get_right_function_expression());
}
void ToMimirStructures::prepare_lifted(const loki::FunctionExpressionMultiOperatorImpl& function_expression)
{
    this->prepare_lifted(function_expression.get_function_expressions());
}
void ToMimirStructures::prepare_lifted(const loki::FunctionExpressionMinusImpl& function_expression)
{
    this->prepare_lifted(*function_expression.get_function_expression());
}
void ToMimirStructures::prepare_lifted(const loki::FunctionExpressionFunctionImpl& function_expression)
{
    m_lifted_fexpr_functions.insert(function_expression.get_function()->get_function_skeleton()->get_name());

    this->prepare_lifted(*function_expression.get_function());
}
void ToMimirStructures::prepare_lifted(const loki::FunctionExpressionImpl& function_expression)
{
    std::visit([this](auto&& arg) { return this->prepare_lifted(*arg); }, function_expression.get_function_expression());
}
void ToMimirStructures::prepare_lifted(const loki::FunctionImpl& function)
{
    prepare_common(*function.get_function_skeleton());
    prepare_lifted(function.get_terms());
}
void ToMimirStructures::prepare_lifted(const loki::ConditionImpl& condition)
{
    auto condition_ptr = &condition;

    if (const auto condition_and = std::get_if<loki::ConditionAnd>(&condition_ptr->get_condition()))
    {
        for (const auto& part : (*condition_and)->get_conditions())
        {
            if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&part->get_condition()))
            {
                prepare_lifted(*(*condition_literal)->get_literal());
            }
            else if (const auto condition_numeric = std::get_if<loki::ConditionNumericConstraint>(&part->get_condition()))
            {
                prepare_lifted(*(*condition_numeric)->get_function_expression_left());
                prepare_lifted(*(*condition_numeric)->get_function_expression_right());
            }
            else
            {
                // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                throw std::logic_error("Expected literal in conjunctive condition.");
            }
        }
    }
    else if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&condition_ptr->get_condition()))
    {
        prepare_lifted(*(*condition_literal)->get_literal());
    }
    else
    {
        // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *condition_ptr) << std::endl;

        throw std::logic_error("Expected conjunctive condition.");
    }
}

void ToMimirStructures::prepare_lifted(const loki::EffectImpl& effect)
{
    const auto prepare_effect_func = [&](const loki::Effect& arg_effect)
    {
        auto tmp_effect = arg_effect;

        // 2. Prepare universal part
        if (const auto& tmp_effect_forall = std::get_if<loki::EffectCompositeForall>(&tmp_effect->get_effect()))
        {
            prepare_common((*tmp_effect_forall)->get_parameters());

            tmp_effect = (*tmp_effect_forall)->get_effect();
        }

        // 3. Prepare conditional part
        if (const auto& tmp_effect_when = std::get_if<loki::EffectCompositeWhen>(&tmp_effect->get_effect()))
        {
            if (const auto condition_and = std::get_if<loki::ConditionAnd>(&(*tmp_effect_when)->get_condition()->get_condition()))
            {
                for (const auto& part : (*condition_and)->get_conditions())
                {
                    if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&part->get_condition()))
                    {
                        prepare_lifted(*(*condition_literal)->get_literal());
                    }
                    else
                    {
                        // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                        throw std::logic_error("Expected literal in conjunctive condition.");
                    }
                }
            }
            else if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&(*tmp_effect_when)->get_condition()->get_condition()))
            {
                prepare_lifted(*(*condition_literal)->get_literal());
            }

            tmp_effect = (*tmp_effect_when)->get_effect();
        }

        // 4. Parse simple effect
        if (const auto& effect_literal = std::get_if<loki::EffectLiteral>(&tmp_effect->get_effect()))
        {
            prepare_lifted(*(*effect_literal)->get_literal());

            // Found predicate affected by an effect
            m_fluent_predicates.insert((*effect_literal)->get_literal()->get_atom()->get_predicate()->get_name());
        }
        else if (const auto& effect_numeric = std::get_if<loki::EffectNumeric>(&tmp_effect->get_effect()))
        {
            // Found function affected by an effect
            m_fluent_functions.insert((*effect_numeric)->get_function()->get_function_skeleton()->get_name());

            prepare_lifted(*(*effect_numeric)->get_function_expression());
        }
        else
        {
            // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *tmp_effect) << std::endl;

            throw std::logic_error("Expected simple effect.");
        }
    };

    // 1. Prepare conjunctive part
    if (const auto& effect_and = std::get_if<loki::EffectAnd>(&effect.get_effect()))
    {
        for (const auto& nested_effect : (*effect_and)->get_effects())
        {
            prepare_effect_func(nested_effect);
        }
    }
    else
    {
        prepare_effect_func(&effect);
    }
}
void ToMimirStructures::prepare_lifted(const loki::ActionImpl& action)
{
    prepare_common(action.get_parameters());
    if (action.get_condition().has_value())
    {
        prepare_lifted(*action.get_condition().value());
    }
    if (action.get_effect().has_value())
    {
        prepare_lifted(*action.get_effect().value());
    }
}
void ToMimirStructures::prepare_lifted(const loki::AxiomImpl& axiom)
{
    prepare_common(axiom.get_parameters());
    prepare_lifted(*axiom.get_condition());

    m_derived_predicates.insert(axiom.get_derived_predicate_name());
}
void ToMimirStructures::prepare_lifted(const loki::DomainImpl& domain)
{
    prepare_common(*domain.get_requirements());
    prepare_common(domain.get_types());
    prepare_common(domain.get_constants());
    prepare_common(domain.get_predicates());
    prepare_common(domain.get_functions());
    prepare_lifted(domain.get_actions());
    prepare_lifted(domain.get_axioms());

    m_action_costs_enabled = domain.get_requirements()->test(loki::RequirementEnum::ACTION_COSTS);
}

/**
 * Prepare grounded
 */

void ToMimirStructures::prepare_grounded(const loki::TermImpl& term)
{
    std::visit([this](auto&& arg) { return this->prepare_common(*arg); }, term.get_object_or_variable());
}

void ToMimirStructures::prepare_grounded(const loki::AtomImpl& atom)
{
    prepare_common(*atom.get_predicate());
    prepare_grounded(atom.get_terms());
}
void ToMimirStructures::prepare_grounded(const loki::LiteralImpl& literal) { prepare_grounded(*literal.get_atom()); }
void ToMimirStructures::prepare_grounded(const loki::FunctionExpressionNumberImpl& function_expression) {}
void ToMimirStructures::prepare_grounded(const loki::FunctionExpressionBinaryOperatorImpl& function_expression)
{
    prepare_grounded(*function_expression.get_left_function_expression());
    prepare_grounded(*function_expression.get_right_function_expression());
}
void ToMimirStructures::prepare_grounded(const loki::FunctionExpressionMultiOperatorImpl& function_expression)
{
    this->prepare_grounded(function_expression.get_function_expressions());
}
void ToMimirStructures::prepare_grounded(const loki::FunctionExpressionMinusImpl& function_expression)
{
    this->prepare_grounded(*function_expression.get_function_expression());
}
void ToMimirStructures::prepare_grounded(const loki::FunctionExpressionFunctionImpl& function_expression)
{
    m_grounded_fexpr_functions.insert(function_expression.get_function()->get_function_skeleton()->get_name());

    this->prepare_grounded(*function_expression.get_function());
}
void ToMimirStructures::prepare_grounded(const loki::FunctionExpressionImpl& function_expression)
{
    std::visit([this](auto&& arg) { return this->prepare_grounded(*arg); }, function_expression.get_function_expression());
}
void ToMimirStructures::prepare_grounded(const loki::FunctionImpl& function)
{
    prepare_common(*function.get_function_skeleton());
    prepare_grounded(function.get_terms());
}
void ToMimirStructures::prepare_grounded(const loki::FunctionValueImpl& function_value) { prepare_grounded(*function_value.get_function()); }
void ToMimirStructures::prepare_grounded(const loki::ConditionImpl& condition)
{
    auto condition_ptr = &condition;

    if (const auto condition_and = std::get_if<loki::ConditionAnd>(&condition_ptr->get_condition()))
    {
        for (const auto& part : (*condition_and)->get_conditions())
        {
            if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&part->get_condition()))
            {
                prepare_grounded(*(*condition_literal)->get_literal());
            }
            else
            {
                // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                throw std::logic_error("Expected literal in conjunctive condition.");
            }
        }
    }
    else if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&condition_ptr->get_condition()))
    {
        prepare_grounded(*(*condition_literal)->get_literal());
    }
    else
    {
        // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *condition_ptr) << std::endl;

        throw std::logic_error("Expected conjunctive condition.");
    }
}
void ToMimirStructures::prepare_grounded(const loki::OptimizationMetricImpl& metric) { prepare_grounded(*metric.get_function_expression()); }

void ToMimirStructures::prepare_grounded(const loki::ProblemImpl& problem)
{
    prepare_lifted(*problem.get_domain());
    prepare_common(*problem.get_requirements());
    prepare_common(problem.get_objects());
    prepare_common(problem.get_derived_predicates());
    prepare_grounded(problem.get_initial_literals());
    prepare_grounded(problem.get_function_values());
    if (problem.get_goal_condition().has_value())
    {
        prepare_lifted(*problem.get_goal_condition().value());
    }
    if (problem.get_optimization_metric().has_value())
    {
        prepare_grounded(*problem.get_optimization_metric().value());
    }
    prepare_lifted(problem.get_axioms());

    for (const auto& derived_predicate : problem.get_derived_predicates())
    {
        m_derived_predicates.insert(derived_predicate->get_name());
    }
}

/**
 * Common
 */

StaticOrFluentOrAuxiliaryFunctionSkeleton ToMimirStructures::translate_common(const loki::FunctionSkeletonImpl& function_skeleton)
{
    // If the metric is monotone, we can instantiate auxiliary functions and store them in a search node.
    if (!m_lifted_fexpr_functions.contains(function_skeleton.get_name()))
    {
        return m_pddl_repositories.template get_or_create_function_skeleton<Auxiliary>(function_skeleton.get_name(),
                                                                                       translate_common(function_skeleton.get_parameters()));
    }
    else if (m_fluent_functions.contains(function_skeleton.get_name()))
    {
        return m_pddl_repositories.template get_or_create_function_skeleton<Fluent>(function_skeleton.get_name(),
                                                                                    translate_common(function_skeleton.get_parameters()));
    }
    else
    {
        return m_pddl_repositories.template get_or_create_function_skeleton<Static>(function_skeleton.get_name(),
                                                                                    translate_common(function_skeleton.get_parameters()));
    }
}

Variable ToMimirStructures::translate_common(const loki::ParameterImpl& parameter) { return translate_common(*parameter.get_variable()); }

Requirements ToMimirStructures::translate_common(const loki::RequirementsImpl& requirements)
{
    return m_pddl_repositories.get_or_create_requirements(requirements.get_requirements());
}

Variable ToMimirStructures::translate_common(const loki::VariableImpl& variable) { return m_pddl_repositories.get_or_create_variable(variable.get_name(), 0); }

Object ToMimirStructures::translate_common(const loki::ObjectImpl& object)
{
    assert(object.get_bases().empty());
    return m_pddl_repositories.get_or_create_object(object.get_name());
}

StaticOrFluentOrDerivedPredicate ToMimirStructures::translate_common(const loki::PredicateImpl& predicate)
{
    auto parameters = translate_common(predicate.get_parameters());

    if (m_fluent_predicates.count(predicate.get_name()) && !m_derived_predicates.count(predicate.get_name()))
    {
        return m_pddl_repositories.get_or_create_predicate<Fluent>(predicate.get_name(), parameters);
    }
    else if (m_derived_predicates.count(predicate.get_name()))
    {
        const auto derived_predicate = m_pddl_repositories.get_or_create_predicate<Derived>(predicate.get_name(), parameters);

        m_derived_predicates_by_name.emplace(derived_predicate->get_name(), derived_predicate);

        return derived_predicate;
    }
    else
    {
        const auto static_predicate = m_pddl_repositories.get_or_create_predicate<Static>(predicate.get_name(), parameters);

        if (predicate.get_name() == "=")
            m_equal_predicate = static_predicate;

        return static_predicate;
    }
}

/**
 * Lifted
 */

Term ToMimirStructures::translate_lifted(const loki::TermImpl& term)
{
    return std::visit([this](auto&& arg) -> Term { return this->m_pddl_repositories.get_or_create_term(this->translate_common(*arg)); },
                      term.get_object_or_variable());
}

StaticOrFluentOrDerivedAtom ToMimirStructures::translate_lifted(const loki::AtomImpl& atom)
{
    auto static_or_fluent__or_derived_predicate = translate_common(*atom.get_predicate());

    return std::visit(
        [this, &atom](auto&& arg) -> StaticOrFluentOrDerivedAtom
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Predicate<Static>>)
            {
                return m_pddl_repositories.get_or_create_atom(arg, translate_lifted(atom.get_terms()));
            }
            else if constexpr (std::is_same_v<T, Predicate<Fluent>>)
            {
                return m_pddl_repositories.get_or_create_atom(arg, translate_lifted(atom.get_terms()));
            }
            else if constexpr (std::is_same_v<T, Predicate<Derived>>)
            {
                return m_pddl_repositories.get_or_create_atom(arg, translate_lifted(atom.get_terms()));
            }
            else
            {
                static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for Predicate type.");
            }
        },
        static_or_fluent__or_derived_predicate);
}

StaticOrFluentOrDerivedLiteral ToMimirStructures::translate_lifted(const loki::LiteralImpl& literal)
{
    auto static_or_fluent_or_derived_atom = translate_lifted(*literal.get_atom());

    return std::visit(
        [this, &literal](auto&& arg) -> StaticOrFluentOrDerivedLiteral
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Atom<Static>>)
            {
                return m_pddl_repositories.get_or_create_literal(literal.is_negated(), arg);
            }
            else if constexpr (std::is_same_v<T, Atom<Fluent>>)
            {
                return m_pddl_repositories.get_or_create_literal(literal.is_negated(), arg);
            }
            else if constexpr (std::is_same_v<T, Atom<Derived>>)
            {
                return m_pddl_repositories.get_or_create_literal(literal.is_negated(), arg);
            }
            else
            {
                static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for Atom type.");
            }
        },
        static_or_fluent_or_derived_atom);
}

FunctionExpression ToMimirStructures::translate_lifted(const loki::FunctionExpressionNumberImpl& function_expression)
{
    return m_pddl_repositories.get_or_create_function_expression(
        m_pddl_repositories.get_or_create_function_expression_number(function_expression.get_number()));
}

FunctionExpression ToMimirStructures::translate_lifted(const loki::FunctionExpressionBinaryOperatorImpl& function_expression)
{
    return m_pddl_repositories.get_or_create_function_expression(
        m_pddl_repositories.get_or_create_function_expression_binary_operator(function_expression.get_binary_operator(),
                                                                              translate_lifted(*function_expression.get_left_function_expression()),
                                                                              translate_lifted(*function_expression.get_right_function_expression())));
}

FunctionExpression ToMimirStructures::translate_lifted(const loki::FunctionExpressionMultiOperatorImpl& function_expression)
{
    return m_pddl_repositories.get_or_create_function_expression(
        m_pddl_repositories.get_or_create_function_expression_multi_operator(function_expression.get_multi_operator(),
                                                                             translate_lifted(function_expression.get_function_expressions())));
}

FunctionExpression ToMimirStructures::translate_lifted(const loki::FunctionExpressionMinusImpl& function_expression)
{
    return m_pddl_repositories.get_or_create_function_expression(
        m_pddl_repositories.get_or_create_function_expression_minus(translate_lifted(*function_expression.get_function_expression())));
}

FunctionExpression ToMimirStructures::translate_lifted(const loki::FunctionExpressionFunctionImpl& function_expression)
{
    return std::visit(
        [this](auto&& function) -> FunctionExpression
        {
            using T = std::decay_t<decltype(function)>;
            if constexpr (std::is_same_v<T, Function<Static>> || std::is_same_v<T, Function<Fluent>>)
            {
                return this->m_pddl_repositories.get_or_create_function_expression(
                    this->m_pddl_repositories.get_or_create_function_expression_function(function));
            }
            else if constexpr (std::is_same_v<T, Function<Auxiliary>>)
            {
                throw std::logic_error("ToMimirStructures::translate_lifted: FunctionExpressionFunction<Auxiliary> must not exist.");
            }
            else
            {
                static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for Function type.");
            }
        },
        translate_lifted(*function_expression.get_function()));
}

FunctionExpression ToMimirStructures::translate_lifted(const loki::FunctionExpressionImpl& function_expression)
{
    return std::visit([this](auto&& arg) { return this->translate_lifted(*arg); }, function_expression.get_function_expression());
}

NumericConstraint ToMimirStructures::translate_lifted(const loki::ConditionNumericConstraintImpl& condition)
{
    return m_pddl_repositories.get_or_create_numeric_constraint(condition.get_binary_comparator(),
                                                                translate_lifted(*condition.get_function_expression_left()),
                                                                translate_lifted(*condition.get_function_expression_right()));
}

StaticOrFluentOrAuxiliaryFunction ToMimirStructures::translate_lifted(const loki::FunctionImpl& function)
{
    return std::visit([&](auto&& function_skeleton) -> StaticOrFluentOrAuxiliaryFunction
                      { return this->m_pddl_repositories.get_or_create_function(function_skeleton, translate_lifted(function.get_terms())); },
                      translate_common(*function.get_function_skeleton()));
}

ConjunctiveCondition ToMimirStructures::translate_lifted(const loki::ConditionImpl& condition, const VariableList& parameters)
{
    auto condition_ptr = &condition;

    const auto func_insert_literal = [](const StaticOrFluentOrDerivedLiteral& static_or_fluent_or_derived_literal,
                                        LiteralList<Static>& ref_static_literals,
                                        LiteralList<Fluent>& ref_fluent_literals,
                                        LiteralList<Derived>& ref_derived_literals)
    {
        std::visit(
            [&ref_static_literals, &ref_fluent_literals, &ref_derived_literals](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Literal<Static>>)
                {
                    ref_static_literals.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, Literal<Fluent>>)
                {
                    ref_fluent_literals.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, Literal<Derived>>)
                {
                    ref_derived_literals.push_back(arg);
                }
            },
            static_or_fluent_or_derived_literal);
    };

    if (const auto condition_and = std::get_if<loki::ConditionAnd>(&condition_ptr->get_condition()))
    {
        auto static_literals = LiteralList<Static> {};
        auto fluent_literals = LiteralList<Fluent> {};
        auto derived_literals = LiteralList<Derived> {};
        auto numeric_constraints = NumericConstraintList {};

        for (const auto& part : (*condition_and)->get_conditions())
        {
            if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&part->get_condition()))
            {
                const auto static_or_fluent_literal = translate_lifted(*(*condition_literal)->get_literal());

                func_insert_literal(static_or_fluent_literal, static_literals, fluent_literals, derived_literals);
            }
            else if (const auto condition_numeric_constraint = std::get_if<loki::ConditionNumericConstraint>(&part->get_condition()))
            {
                const auto numeric_constraint = translate_lifted(*(*condition_numeric_constraint));

                numeric_constraints.push_back(numeric_constraint);
            }
            else
            {
                // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                throw std::logic_error("Expected literal in conjunctive condition.");
            }
        }
        return m_pddl_repositories.get_or_create_conjunctive_condition(parameters, static_literals, fluent_literals, derived_literals, numeric_constraints);
    }
    else if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&condition_ptr->get_condition()))
    {
        auto static_literals = LiteralList<Static> {};
        auto fluent_literals = LiteralList<Fluent> {};
        auto derived_literals = LiteralList<Derived> {};
        auto numeric_constraints = NumericConstraintList {};

        const auto static_or_fluent_or_derived_literal = translate_lifted(*(*condition_literal)->get_literal());

        func_insert_literal(static_or_fluent_or_derived_literal, static_literals, fluent_literals, derived_literals);

        return m_pddl_repositories.get_or_create_conjunctive_condition(parameters, static_literals, fluent_literals, derived_literals, numeric_constraints);
    }

    // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *condition_ptr) << std::endl;

    throw std::logic_error("Expected conjunctive condition.");
}

template<DynamicFunctionTag F>
static NumericEffect<F> get_or_create_total_cost_numeric_effect(FunctionSkeleton<F> total_cost_function,
                                                                bool action_costs_enabled,
                                                                NumericEffectList<F>& numeric_effects,
                                                                PDDLRepositories& pddl_repositories)
{
    for (const auto& effect : numeric_effects)
    {
        if (effect->get_function()->get_function_skeleton()->get_name() == "total-cost")
            return effect;
    }

    const auto total_cost_numeric_effect = pddl_repositories.get_or_create_numeric_effect(
        loki::AssignOperatorEnum::INCREASE,
        pddl_repositories.get_or_create_function(total_cost_function, TermList {}),
        pddl_repositories.get_or_create_function_expression(pddl_repositories.get_or_create_function_expression_number(action_costs_enabled ? 0 : 1)));

    numeric_effects.push_back(total_cost_numeric_effect);

    return total_cost_numeric_effect;
}

std::tuple<ConjunctiveEffect, ConditionalEffectList> ToMimirStructures::translate_lifted(const loki::EffectImpl& effect, const VariableList& parameters)
{
    using ConjunctiveEffectData = std::tuple<LiteralList<Fluent>, NumericEffectList<Fluent>, NumericEffectList<Auxiliary>>;
    using ConditionalEffectData =
        std::unordered_map<ConjunctiveCondition, std::tuple<LiteralList<Fluent>, NumericEffectList<Fluent>, NumericEffectList<Auxiliary>>>;

    const auto translate_effect_func =
        [&](const loki::Effect& effect, ConjunctiveEffectData& ref_conjunctive_effect_data, ConditionalEffectData& ref_conditional_effect_data)
    {
        auto tmp_effect = effect;

        /* 1. Parse universal part. */
        auto parameters = VariableList {};
        if (const auto& tmp_effect_forall = std::get_if<loki::EffectCompositeForall>(&tmp_effect->get_effect()))
        {
            parameters = translate_common((*tmp_effect_forall)->get_parameters());

            tmp_effect = (*tmp_effect_forall)->get_effect();
        }

        /* 2. Parse conditional part */
        auto conjunctive_condition = ConjunctiveCondition { nullptr };
        if (const auto tmp_effect_when = std::get_if<loki::EffectCompositeWhen>(&tmp_effect->get_effect()))
        {
            conjunctive_condition = translate_lifted(*(*tmp_effect_when)->get_condition(), parameters);

            tmp_effect = (*tmp_effect_when)->get_effect();
        }

        // Fetch container to store the effects
        auto& [data_fluent_literals, data_fluent_numeric_effects, data_auxiliary_numeric_effects] =
            (conjunctive_condition) ? ref_conditional_effect_data[conjunctive_condition] : ref_conjunctive_effect_data;

        /* 3. Parse effect part */
        if (const auto& effect_literal = std::get_if<loki::EffectLiteral>(&tmp_effect->get_effect()))
        {
            const auto static_or_fluent_or_derived_effect = translate_lifted(*(*effect_literal)->get_literal());

            if (std::get_if<Literal<Derived>>(&static_or_fluent_or_derived_effect))
            {
                throw std::runtime_error("Only fluent literals are allowed in effects!");
            }

            const auto fluent_effect = std::get<Literal<Fluent>>(static_or_fluent_or_derived_effect);

            data_fluent_literals.push_back(fluent_effect);
        }
        else if (const auto& effect_numeric = std::get_if<loki::EffectNumeric>(&tmp_effect->get_effect()))
        {
            const auto static_fluent_or_auxiliary_function = translate_lifted(*(*effect_numeric)->get_function());
            const auto function_expression = translate_lifted(*(*effect_numeric)->get_function_expression());

            std::visit(
                [&](auto&& function)
                {
                    using T = std::decay_t<decltype(function)>;
                    if constexpr (std::is_same_v<T, Function<Fluent>>)
                    {
                        data_fluent_numeric_effects.push_back(
                            m_pddl_repositories.get_or_create_numeric_effect<Fluent>((*effect_numeric)->get_assign_operator(), function, function_expression));
                    }
                    else if constexpr (std::is_same_v<T, Function<Auxiliary>>)
                    {
                        data_auxiliary_numeric_effects.push_back(
                            m_pddl_repositories.get_or_create_numeric_effect<Auxiliary>((*effect_numeric)->get_assign_operator(),
                                                                                        function,
                                                                                        function_expression));
                    }
                    else if constexpr (std::is_same_v<T, Function<Static>>)
                    {
                        throw std::logic_error("ToMimirStructures::translate_lifted: Function<Static> must not exist.");
                    }
                    else
                    {
                        static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for Function type.");
                    }
                },
                static_fluent_or_auxiliary_function);
        }
        else
        {
            // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *tmp_effect) << std::endl;

            throw std::logic_error("Unexpected effect type. This error indicates a bug in the translation.");
        }
    };

    /* Parse the effect */
    auto effect_ptr = &effect;
    auto conjunctive_effect_data = ConjunctiveEffectData {};
    auto conditional_effect_data = ConditionalEffectData {};
    // Parse conjunctive part
    if (const auto& effect_and = std::get_if<loki::EffectAnd>(&effect_ptr->get_effect()))
    {
        for (const auto& nested_effect : (*effect_and)->get_effects())
        {
            translate_effect_func(nested_effect, conjunctive_effect_data, conditional_effect_data);
        }
    }
    else
    {
        // Parse non conjunctive
        translate_effect_func(effect_ptr, conjunctive_effect_data, conditional_effect_data);
    }

    /* Instantiate conjunctive effect. */
    auto& [conjunctive_effect_fluent_literals, conjunctive_effect_fluent_numeric_effects, conjunctive_effect_auxiliary_numeric_effects] =
        conjunctive_effect_data;

    // Instantiate total cost effect if necessary.
    std::visit(
        [&](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, FunctionSkeleton<Fluent>>)
            {
                get_or_create_total_cost_numeric_effect(arg, m_action_costs_enabled, conjunctive_effect_fluent_numeric_effects, m_pddl_repositories);
            }
            else if constexpr (std::is_same_v<T, FunctionSkeleton<Auxiliary>>)
            {
                get_or_create_total_cost_numeric_effect(arg, m_action_costs_enabled, conjunctive_effect_auxiliary_numeric_effects, m_pddl_repositories);
            }
            else
            {
                static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for FunctionSkeleton type.");
            }
        },
        m_total_cost_function);

    const auto conjunctive_effect = this->m_pddl_repositories.get_or_create_conjunctive_effect(parameters,
                                                                                               conjunctive_effect_fluent_literals,
                                                                                               conjunctive_effect_fluent_numeric_effects,
                                                                                               conjunctive_effect_auxiliary_numeric_effects);

    /* Instantiate conditional effects. */
    auto conditional_effects = ConditionalEffectList {};
    for (const auto& [cond_conjunctive_condition, value] : conditional_effect_data)
    {
        const auto& [cond_effect_fluent_literals, cond_effect_fluent_numeric_effects, cond_effect_auxiliary_numeric_effects] = value;

        conditional_effects.push_back(this->m_pddl_repositories.get_or_create_conditional_effect(
            cond_conjunctive_condition,
            this->m_pddl_repositories.get_or_create_conjunctive_effect(parameters,
                                                                       cond_effect_fluent_literals,
                                                                       cond_effect_fluent_numeric_effects,
                                                                       cond_effect_auxiliary_numeric_effects)));
    }

    return { conjunctive_effect, conditional_effects };
}

Action ToMimirStructures::translate_lifted(const loki::ActionImpl& action)
{
    // We sort the additional parameters to enforce some additional approximate syntactic equivalence.
    auto translated_parameters = translate_common(action.get_parameters());
    std::sort(translated_parameters.begin() + action.get_original_arity(), translated_parameters.end());

    // 1. Translate conditions
    auto conjunctive_condition = ConjunctiveCondition { nullptr };
    if (action.get_condition().has_value())
    {
        conjunctive_condition = translate_lifted(*action.get_condition().value(), translated_parameters);
    }

    // 2. Translate effects
    auto conjunctive_effect = ConjunctiveEffect { nullptr };
    auto conditional_effects = ConditionalEffectList {};
    if (action.get_effect().has_value())
    {
        const auto [conjunctive_effect_, conditional_effects_] = translate_lifted(*action.get_effect().value(), translated_parameters);
        conjunctive_effect = conjunctive_effect_;
        conditional_effects = conditional_effects_;
    }
    else
    {
        // TODO: We are missing total-cost effect...
        conjunctive_effect = m_pddl_repositories.get_or_create_conjunctive_effect(translated_parameters,
                                                                                  LiteralList<Fluent> {},
                                                                                  NumericEffectList<Fluent> {},
                                                                                  NumericEffectList<Auxiliary> {});
    }

    return m_pddl_repositories.get_or_create_action(action.get_name(),
                                                    action.get_original_arity(),
                                                    conjunctive_condition,
                                                    conjunctive_effect,
                                                    conditional_effects);
}

Axiom ToMimirStructures::translate_lifted(const loki::AxiomImpl& axiom)
{
    auto parameters = translate_common(axiom.get_parameters());

    const auto conjunctive_condition = translate_lifted(*axiom.get_condition(), parameters);

    // TODO: make this nicer by storing a predicate in the axiom in loki...
    const auto derived_predicate_name = axiom.get_derived_predicate_name();
    if (!m_derived_predicates_by_name.count(derived_predicate_name))
    {
        // Create a derived predicate that resulted from translation
        // and is not part of the predicates section.
        // The parameters of the derived predicate are only those needed for ground the head
        // and do not contain other parameters obtained from other free variables.
        m_derived_predicates_by_name.emplace(derived_predicate_name,
                                             m_pddl_repositories.get_or_create_predicate<Derived>(
                                                 derived_predicate_name,
                                                 VariableList(parameters.begin(), parameters.begin() + axiom.get_num_parameters_to_ground_head())));
    }
    const auto derived_predicate = m_derived_predicates_by_name.at(axiom.get_derived_predicate_name());

    // The number of terms is only as large as needed and matches the derived predicate
    auto terms = TermList {};
    for (size_t i = 0; i < axiom.get_num_parameters_to_ground_head(); ++i)
    {
        terms.push_back(m_pddl_repositories.get_or_create_term(parameters[i]));
    }
    assert(terms.size() == derived_predicate->get_arity());

    // Our axiom heads are always true literals
    const auto literal = m_pddl_repositories.get_or_create_literal(false, m_pddl_repositories.get_or_create_atom(derived_predicate, terms));

    return m_pddl_repositories.get_or_create_axiom(conjunctive_condition, literal);
}

static std::variant<FunctionSkeleton<Fluent>, FunctionSkeleton<Auxiliary>>
get_or_create_total_cost_function(const FunctionSkeletonList<Fluent>& fluent_functions,
                                  FunctionSkeletonList<Auxiliary>& auxiliary_functions,
                                  PDDLRepositories& pddl_repositories)
{
    for (const auto& function : fluent_functions)
    {
        if (function->get_name() == "total-cost")
            return function;
    }
    for (const auto& function : auxiliary_functions)
    {
        if (function->get_name() == "total-cost")
            return function;
    }

    const auto total_cost_function = pddl_repositories.get_or_create_function_skeleton<Auxiliary>("total-cost", VariableList {});

    std::cout << "[ToMimir] Adding default " << total_cost_function << " to \":functions\"." << std::endl;

    auxiliary_functions.push_back(total_cost_function);
    return total_cost_function;
}

Domain ToMimirStructures::translate_lifted(const loki::DomainImpl& domain)
{
    const auto requirements = translate_common(*domain.get_requirements());
    const auto constants = translate_common(domain.get_constants());

    auto predicates = translate_common(domain.get_predicates());
    auto static_predicates = PredicateList<Static> {};
    auto fluent_predicates = PredicateList<Fluent> {};
    auto derived_predicates = PredicateList<Derived> {};
    for (const auto& static_or_fluent_or_derived_predicate : translate_common(domain.get_predicates()))
    {
        std::visit(
            [&static_predicates, &fluent_predicates, &derived_predicates](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Predicate<Static>>)
                {
                    static_predicates.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, Predicate<Fluent>>)
                {
                    fluent_predicates.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, Predicate<Derived>>)
                {
                    derived_predicates.push_back(arg);
                }
                else
                {
                    static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for Predicate type.");
                }
            },
            static_or_fluent_or_derived_predicate);
    }

    auto static_functions = FunctionSkeletonList<Static> {};
    auto fluent_functions = FunctionSkeletonList<Fluent> {};
    auto auxiliary_functions = FunctionSkeletonList<Auxiliary> {};
    for (const auto& static_or_fluent_or_auxiliary_function : translate_common(domain.get_functions()))
    {
        std::visit(
            [&static_functions, &fluent_functions, &auxiliary_functions](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, FunctionSkeleton<Static>>)
                {
                    static_functions.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, FunctionSkeleton<Fluent>>)
                {
                    fluent_functions.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, FunctionSkeleton<Auxiliary>>)
                {
                    auxiliary_functions.push_back(arg);
                }
                else
                {
                    static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for FunctionSkeleton type.");
                }
            },
            static_or_fluent_or_auxiliary_function);
    }
    m_total_cost_function = get_or_create_total_cost_function(fluent_functions, auxiliary_functions, m_pddl_repositories);

    const auto actions = translate_lifted(domain.get_actions());
    const auto axioms = translate_lifted(domain.get_axioms());

    // Add equal predicate that was hidden from predicate section
    // This must occur after translating all domain contents
    if (m_equal_predicate)
    {
        predicates.push_back(m_equal_predicate);
        static_predicates.push_back(m_equal_predicate);
    }

    return m_pddl_repositories.get_or_create_domain(domain.get_filepath(),
                                                    domain.get_name(),
                                                    requirements,
                                                    uniquify_elements(constants),
                                                    uniquify_elements(static_predicates),
                                                    uniquify_elements(fluent_predicates),
                                                    uniquify_elements(derived_predicates),
                                                    uniquify_elements(static_functions),
                                                    uniquify_elements(fluent_functions),
                                                    uniquify_elements(auxiliary_functions),
                                                    uniquify_elements(actions),
                                                    uniquify_elements(axioms));
}

/**
 * Grounded
 */

Object ToMimirStructures::translate_grounded(const loki::TermImpl& term)
{
    if (const auto object = std::get_if<loki::Object>(&term.get_object_or_variable()))
    {
        return translate_common(**object);
    }

    throw std::logic_error("Expected ground term.");
}

StaticOrFluentOrDerivedGroundAtom ToMimirStructures::translate_grounded(const loki::AtomImpl& atom)
{
    auto static_or_fluent_or_derived_predicate = translate_common(*atom.get_predicate());

    return std::visit(
        [this, &atom](auto&& arg) -> StaticOrFluentOrDerivedGroundAtom
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Predicate<Static>>)
            {
                return m_pddl_repositories.get_or_create_ground_atom(arg, translate_grounded(atom.get_terms()));
            }
            else if constexpr (std::is_same_v<T, Predicate<Fluent>>)
            {
                return m_pddl_repositories.get_or_create_ground_atom(arg, translate_grounded(atom.get_terms()));
            }
            else if constexpr (std::is_same_v<T, Predicate<Derived>>)
            {
                return m_pddl_repositories.get_or_create_ground_atom(arg, translate_grounded(atom.get_terms()));
            }
            else
            {
                static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for Predicate type.");
            }
        },
        static_or_fluent_or_derived_predicate);
}

StaticOrFluentOrDerivedGroundLiteral ToMimirStructures::translate_grounded(const loki::LiteralImpl& literal)
{
    auto static_or_fluent_or_derived_ground_atom = translate_grounded(*literal.get_atom());

    return std::visit(
        [this, &literal](auto&& arg) -> StaticOrFluentOrDerivedGroundLiteral
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, GroundAtom<Static>>)
            {
                return m_pddl_repositories.get_or_create_ground_literal(literal.is_negated(), arg);
            }
            else if constexpr (std::is_same_v<T, GroundAtom<Fluent>>)
            {
                return m_pddl_repositories.get_or_create_ground_literal(literal.is_negated(), arg);
            }
            else if constexpr (std::is_same_v<T, GroundAtom<Derived>>)
            {
                return m_pddl_repositories.get_or_create_ground_literal(literal.is_negated(), arg);
            }
            else
            {
                static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for GroundAtom type.");
            }
        },
        static_or_fluent_or_derived_ground_atom);
}

StaticOrFluentOrAuxiliaryGroundFunctionValue ToMimirStructures::translate_grounded(const loki::FunctionValueImpl& function_value)
{
    const auto static_or_fluent_or_auxiliary_ground_function = translate_grounded(*function_value.get_function());

    return std::visit([&](auto&& ground_function) -> StaticOrFluentOrAuxiliaryGroundFunctionValue
                      { return m_pddl_repositories.get_or_create_ground_function_value(ground_function, function_value.get_number()); },
                      static_or_fluent_or_auxiliary_ground_function);
}

GroundFunctionExpression ToMimirStructures::translate_grounded(const loki::FunctionExpressionNumberImpl& function_expression)
{
    return m_pddl_repositories.get_or_create_ground_function_expression(
        m_pddl_repositories.get_or_create_ground_function_expression_number(function_expression.get_number()));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(const loki::FunctionExpressionBinaryOperatorImpl& function_expression)
{
    return m_pddl_repositories.get_or_create_ground_function_expression(
        m_pddl_repositories.get_or_create_ground_function_expression_binary_operator(function_expression.get_binary_operator(),
                                                                                     translate_grounded(*function_expression.get_left_function_expression()),
                                                                                     translate_grounded(*function_expression.get_right_function_expression())));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(const loki::FunctionExpressionMultiOperatorImpl& function_expression)
{
    return m_pddl_repositories.get_or_create_ground_function_expression(
        m_pddl_repositories.get_or_create_ground_function_expression_multi_operator(function_expression.get_multi_operator(),
                                                                                    translate_grounded(function_expression.get_function_expressions())));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(const loki::FunctionExpressionMinusImpl& function_expression)
{
    return m_pddl_repositories.get_or_create_ground_function_expression(
        m_pddl_repositories.get_or_create_ground_function_expression_minus(translate_grounded(*function_expression.get_function_expression())));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(const loki::FunctionExpressionFunctionImpl& function_expression)
{
    return std::visit(
        [this](auto&& ground_function)
        {
            using T = std::decay_t<decltype(ground_function)>;
            if constexpr (std::is_same_v<T, GroundFunction<Static>>)
            {
                if (!m_static_function_to_value.contains(ground_function))
                {
                    throw std::runtime_error("ToMimirStructures::translate_grounded(function_expression): undefined static function value "
                                             + to_string(ground_function) + ".");
                }
                return m_pddl_repositories.get_or_create_ground_function_expression(
                    m_pddl_repositories.get_or_create_ground_function_expression_number(m_static_function_to_value.at(ground_function)));
            }
            else if constexpr (std::is_same_v<T, GroundFunction<Fluent>>)
            {
                return m_pddl_repositories.get_or_create_ground_function_expression(
                    m_pddl_repositories.get_or_create_ground_function_expression_function(ground_function));
            }
            else if constexpr (std::is_same_v<T, GroundFunction<Auxiliary>>)
            {
                return m_pddl_repositories.get_or_create_ground_function_expression(
                    m_pddl_repositories.get_or_create_ground_function_expression_function(ground_function));
            }
            else
            {
                static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for GroundFunction type.");
            }
        },
        translate_grounded(*function_expression.get_function()));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(const loki::FunctionExpressionImpl& function_expression)
{
    return std::visit([this](auto&& arg) { return this->translate_grounded(*arg); }, function_expression.get_function_expression());
}

StaticOrFluentOrAuxiliaryGroundFunction ToMimirStructures::translate_grounded(const loki::FunctionImpl& function)
{
    const auto static_or_fluent_or_auxiliary_function_skeleton = translate_common(*function.get_function_skeleton());
    const auto objects = translate_grounded(function.get_terms());

    return std::visit([&](auto&& function_skeleton) -> StaticOrFluentOrAuxiliaryGroundFunction
                      { return m_pddl_repositories.get_or_create_ground_function(function_skeleton, objects); },
                      static_or_fluent_or_auxiliary_function_skeleton);
}

GroundNumericConstraint ToMimirStructures::translate_grounded(const loki::ConditionNumericConstraintImpl& condition)
{
    return m_pddl_repositories.get_or_create_ground_numeric_constraint(condition.get_binary_comparator(),
                                                                       translate_grounded(*condition.get_function_expression_left()),
                                                                       translate_grounded(*condition.get_function_expression_right()));
}

std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>, GroundNumericConstraintList>
ToMimirStructures::translate_grounded(const loki::ConditionImpl& condition)
{
    auto condition_ptr = &condition;

    const auto func_insert_ground_literal = [](const StaticOrFluentOrDerivedGroundLiteral& static_or_fluent_or_derived_literal,
                                               GroundLiteralList<Static>& ref_static_ground_literals,
                                               GroundLiteralList<Fluent>& ref_fluent_ground_literals,
                                               GroundLiteralList<Derived>& ref_derived_ground_literals)
    {
        std::visit(
            [&ref_static_ground_literals, &ref_fluent_ground_literals, &ref_derived_ground_literals](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, GroundLiteral<Static>>)
                {
                    ref_static_ground_literals.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, GroundLiteral<Fluent>>)
                {
                    ref_fluent_ground_literals.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, GroundLiteral<Derived>>)
                {
                    ref_derived_ground_literals.push_back(arg);
                }
                else
                {
                    static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for GroundLiteral type.");
                }
            },
            static_or_fluent_or_derived_literal);
    };

    if (const auto condition_and = std::get_if<loki::ConditionAnd>(&condition_ptr->get_condition()))
    {
        auto static_ground_literals = GroundLiteralList<Static> {};
        auto fluent_ground_literals = GroundLiteralList<Fluent> {};
        auto derived_ground_literals = GroundLiteralList<Derived> {};
        auto numeric_constraints = GroundNumericConstraintList {};

        for (const auto& part : (*condition_and)->get_conditions())
        {
            if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&part->get_condition()))
            {
                const auto static_or_fluent_ground_literal = translate_grounded(*(*condition_literal)->get_literal());

                func_insert_ground_literal(static_or_fluent_ground_literal, static_ground_literals, fluent_ground_literals, derived_ground_literals);
            }
            else if (const auto condition_numeric = std::get_if<loki::ConditionNumericConstraint>(&part->get_condition()))
            {
                numeric_constraints.push_back(translate_grounded(**condition_numeric));
            }
            else
            {
                // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                throw std::logic_error("Expected literal in conjunctive condition.");
            }
        }
        return std::make_tuple(static_ground_literals, fluent_ground_literals, derived_ground_literals, numeric_constraints);
    }
    else if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&condition_ptr->get_condition()))
    {
        auto static_ground_literals = GroundLiteralList<Static> {};
        auto fluent_ground_literals = GroundLiteralList<Fluent> {};
        auto derived_ground_literals = GroundLiteralList<Derived> {};
        auto numeric_constraints = GroundNumericConstraintList {};

        const auto static_or_fluent_or_derived_ground_literal = translate_grounded(*(*condition_literal)->get_literal());

        func_insert_ground_literal(static_or_fluent_or_derived_ground_literal, static_ground_literals, fluent_ground_literals, derived_ground_literals);

        return std::make_tuple(static_ground_literals, fluent_ground_literals, derived_ground_literals, numeric_constraints);
    }
    else if (const auto& condition_numeric = std::get_if<loki::ConditionNumericConstraint>(&condition_ptr->get_condition()))
    {
        auto static_ground_literals = GroundLiteralList<Static> {};
        auto fluent_ground_literals = GroundLiteralList<Fluent> {};
        auto derived_ground_literals = GroundLiteralList<Derived> {};
        auto numeric_constraints = GroundNumericConstraintList {};

        numeric_constraints.push_back(translate_grounded(**condition_numeric));

        return std::make_tuple(static_ground_literals, fluent_ground_literals, derived_ground_literals, numeric_constraints);
    }

    // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *condition_ptr) << std::endl;

    throw std::logic_error("Expected conjunctive condition.");
}

OptimizationMetric ToMimirStructures::translate_grounded(const loki::OptimizationMetricImpl& optimization_metric)
{
    return m_pddl_repositories.get_or_create_optimization_metric(optimization_metric.get_optimization_metric(),
                                                                 translate_grounded(*optimization_metric.get_function_expression()));
}

template<DynamicFunctionTag F>
static GroundFunctionValue<F> get_or_create_total_cost_function_value(FunctionSkeleton<F> total_cost_function,
                                                                      GroundFunctionValueList<F>& function_values,
                                                                      PDDLRepositories& pddl_repositories)
{
    for (const auto& function_value : function_values)
    {
        if (function_value->get_function()->get_function_skeleton() == total_cost_function)
        {
            return function_value;
        }
    }

    const auto total_cost_function_value =
        pddl_repositories.get_or_create_ground_function_value(pddl_repositories.get_or_create_ground_function(total_cost_function, ObjectList {}), 0.);

    std::cout << "[ToMimir] Adding default " << total_cost_function_value << " to \":init\"." << std::endl;

    function_values.push_back(total_cost_function_value);
    return total_cost_function_value;
}

template GroundFunctionValue<Fluent> get_or_create_total_cost_function_value(FunctionSkeleton<Fluent> total_cost_function,
                                                                             GroundFunctionValueList<Fluent>& function_values,
                                                                             PDDLRepositories& pddl_repositories);
template GroundFunctionValue<Auxiliary> get_or_create_total_cost_function_value(FunctionSkeleton<Auxiliary> total_cost_function,
                                                                                GroundFunctionValueList<Auxiliary>& function_values,
                                                                                PDDLRepositories& pddl_repositories);

Problem ToMimirStructures::translate_grounded(const loki::ProblemImpl& problem)
{
    // Translate domain first, to get predicate indices 0,1,2,...
    const auto translated_domain = translate_lifted(*problem.get_domain());
    // Translate derived predicates to fetch parameter indices
    auto derived_predicates = PredicateList<Derived> {};
    for (const auto static_or_fluent_or_derived_predicate : translate_common(problem.get_derived_predicates()))
    {
        derived_predicates.push_back(std::get<Predicate<Derived>>(static_or_fluent_or_derived_predicate));
    }

    // Add constants to objects in problem.
    const auto constants = translate_common(problem.get_domain()->get_constants());
    auto objects = translate_common(problem.get_objects());
    objects.insert(objects.end(), constants.begin(), constants.end());

    // Derive static and fluent initial literals
    auto static_initial_literals = GroundLiteralList<Static> {};
    auto fluent_initial_literals = GroundLiteralList<Fluent> {};
    auto derived_initial_literals = GroundLiteralList<Derived> {};
    for (const auto& static_or_fluent_or_derived_ground_literal : translate_grounded(problem.get_initial_literals()))
    {
        std::visit(
            [&static_initial_literals, &fluent_initial_literals, &derived_initial_literals](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, GroundLiteral<Static>>)
                {
                    static_initial_literals.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, GroundLiteral<Fluent>>)
                {
                    fluent_initial_literals.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, GroundLiteral<Derived>>)
                {
                    derived_initial_literals.push_back(arg);
                }
                else
                {
                    static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for GroundLiteral type.");
                }
            },
            static_or_fluent_or_derived_ground_literal);
    }

    auto static_function_values = GroundFunctionValueList<Static> {};
    auto fluent_function_values = GroundFunctionValueList<Fluent> {};
    auto auxiliary_function_values = GroundFunctionValueList<Auxiliary> {};
    for (const auto static_or_fluent_or_auxiliary_function_value : translate_grounded(problem.get_function_values()))
    {
        std::visit(
            [&static_function_values, &fluent_function_values, &auxiliary_function_values](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, GroundFunctionValue<Static>>)
                {
                    static_function_values.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, GroundFunctionValue<Fluent>>)
                {
                    fluent_function_values.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, GroundFunctionValue<Auxiliary>>)
                {
                    auxiliary_function_values.push_back(arg);
                }
                else
                {
                    static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for GroundFunctionValue type.");
                }
            },
            static_or_fluent_or_auxiliary_function_value);
    }
    // Initialize total cost function value if necessary
    std::visit(
        [&](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, FunctionSkeleton<Fluent>>)
            {
                get_or_create_total_cost_function_value(arg, fluent_function_values, m_pddl_repositories);
            }
            else if constexpr (std::is_same_v<T, FunctionSkeleton<Auxiliary>>)
            {
                get_or_create_total_cost_function_value(arg, auxiliary_function_values, m_pddl_repositories);
            }
            else
            {
                static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for FunctionSkeleton type.");
            }
        },
        m_total_cost_function);
    // Initialize member that is used in numeric goal grounding.
    for (const auto& function_value : static_function_values)
    {
        m_static_function_to_value.emplace(function_value->get_function(), function_value->get_number());
    }

    // Add equal atoms, e.g., (= object1 object1)
    // This must occur after parsing the domain
    if (m_equal_predicate)
    {
        for (const auto& object : objects)
        {
            const auto equal_literal = m_pddl_repositories.get_or_create_ground_literal(
                false,
                m_pddl_repositories.get_or_create_ground_atom(m_equal_predicate, ObjectList { object, object }));

            static_initial_literals.push_back(equal_literal);
        }
    }

    const auto func_create_default_metric = [&]()
    {
        return std::visit(
            [&](auto&& arg)
            {
                const auto metric = m_pddl_repositories.get_or_create_optimization_metric(
                    loki::OptimizationMetricEnum::MINIMIZE,
                    m_pddl_repositories.get_or_create_ground_function_expression(m_pddl_repositories.get_or_create_ground_function_expression_function(
                        m_pddl_repositories.get_or_create_ground_function(arg, ObjectList {}))));

                std::cout << "[ToMimir] Creating default " << metric << std::endl;

                return metric;
            },
            m_total_cost_function);
    };

    auto static_goal_literals = GroundLiteralList<Static> {};
    auto fluent_goal_literals = GroundLiteralList<Fluent> {};
    auto derived_goal_literals = GroundLiteralList<Derived> {};
    auto numeric_goal_constraints = GroundNumericConstraintList {};
    if (problem.get_goal_condition().has_value())
    {
        const auto [static_goal_literals_, fluent_goal_literals_, derived_goal_literals_, numeric_goal_constraints_] =
            translate_grounded(*problem.get_goal_condition().value());

        static_goal_literals = static_goal_literals_;
        fluent_goal_literals = fluent_goal_literals_;
        derived_goal_literals = derived_goal_literals_;
        numeric_goal_constraints = numeric_goal_constraints_;
    }

    const auto metric =
        (problem.get_optimization_metric().has_value() ? translate_grounded(*problem.get_optimization_metric().value()) : func_create_default_metric());

    return m_pddl_repositories.get_or_create_problem(problem.get_filepath(),
                                                     translated_domain,
                                                     problem.get_name(),
                                                     translate_common(*problem.get_requirements()),
                                                     uniquify_elements(objects),
                                                     uniquify_elements(derived_predicates),
                                                     uniquify_elements(static_initial_literals),
                                                     uniquify_elements(fluent_initial_literals),
                                                     uniquify_elements(static_function_values),
                                                     uniquify_elements(fluent_function_values),
                                                     uniquify_elements(auxiliary_function_values),
                                                     uniquify_elements(static_goal_literals),
                                                     uniquify_elements(fluent_goal_literals),
                                                     uniquify_elements(derived_goal_literals),
                                                     numeric_goal_constraints,
                                                     metric,
                                                     translate_lifted(problem.get_axioms()));
}

Problem ToMimirStructures::run(const loki::ProblemImpl& problem)
{
    prepare_grounded(problem);
    return translate_grounded(problem);
}

ToMimirStructures::ToMimirStructures(PDDLRepositories& pddl_repositories) :
    m_pddl_repositories(pddl_repositories),
    m_fluent_predicates(),
    m_derived_predicates(),
    m_lifted_fexpr_functions(),
    m_grounded_fexpr_functions(),
    m_fluent_functions(),
    m_action_costs_enabled(),
    m_derived_predicates_by_name(),
    m_equal_predicate(nullptr),
    m_total_cost_function(),
    m_static_function_to_value()
{
}
}