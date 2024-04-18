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

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

Requirements ToMimirStructures::translate(const loki::RequirementsImpl& requirements)
{
    return m_pddl_factories.get_or_create_requirements(requirements.get_requirements());
}

Variable ToMimirStructures::translate(const loki::VariableImpl& variable) { return m_pddl_factories.get_or_create_variable(variable.get_name()); }

Object ToMimirStructures::translate(const loki::ObjectImpl& object)
{
    if (!object.get_bases().empty())
    {
        throw std::logic_error("Expected types to be empty.");
    }
    return m_pddl_factories.get_or_create_object(object.get_name());
}

Term ToMimirStructures::translate(const loki::TermVariableImpl& term) { return m_pddl_factories.get_or_create_term_variable(translate(*term.get_variable())); }

Term ToMimirStructures::translate(const loki::TermObjectImpl& term) { return m_pddl_factories.get_or_create_term_object(translate(*term.get_object())); }

Term ToMimirStructures::translate(const loki::TermImpl& term)
{
    return std::visit([this](auto&& arg) { return this->translate(arg); }, term);
}

Parameter ToMimirStructures::translate(const loki::ParameterImpl& parameter)
{
    if (!parameter.get_bases().empty())
    {
        throw std::logic_error("Expected types to be empty.");
    }
    return m_pddl_factories.get_or_create_parameter(translate(*parameter.get_variable()));
}

Predicate ToMimirStructures::translate(const loki::PredicateImpl& predicate)
{
    return m_pddl_factories.get_or_create_predicate(predicate.get_name(), translate(predicate.get_parameters()));
}

Atom ToMimirStructures::translate(const loki::AtomImpl& atom)
{
    return m_pddl_factories.get_or_create_atom(translate(*atom.get_predicate()), translate(atom.get_terms()));
}

GroundAtom ToMimirStructures::translate(const loki::GroundAtomImpl& atom)
{
    return m_pddl_factories.get_or_create_ground_atom(translate(*atom.get_predicate()), translate(atom.get_objects()));
}

Literal ToMimirStructures::translate(const loki::LiteralImpl& literal)
{
    return m_pddl_factories.get_or_create_literal(literal.is_negated(), translate(*literal.get_atom()));
}

GroundLiteral ToMimirStructures::translate(const loki::GroundLiteralImpl& literal)
{
    return m_pddl_factories.get_or_create_ground_literal(literal.is_negated(), translate(*literal.get_atom()));
}

NumericFluent ToMimirStructures::translate(const loki::NumericFluentImpl& numeric_fluent)
{
    return m_pddl_factories.get_or_create_numeric_fluent(translate(*numeric_fluent.get_function()), numeric_fluent.get_number());
}

FunctionExpression ToMimirStructures::translate(const loki::FunctionExpressionNumberImpl& function_expression)
{
    return m_pddl_factories.get_or_create_function_expression_number(function_expression.get_number());
}

FunctionExpression ToMimirStructures::translate(const loki::FunctionExpressionBinaryOperatorImpl& function_expression)
{
    return m_pddl_factories.get_or_create_function_expression_binary_operator(function_expression.get_binary_operator(),
                                                                              translate(*function_expression.get_left_function_expression()),
                                                                              translate(*function_expression.get_right_function_expression()));
}

FunctionExpression ToMimirStructures::translate(const loki::FunctionExpressionMultiOperatorImpl& function_expression)
{
    return m_pddl_factories.get_or_create_function_expression_multi_operator(function_expression.get_multi_operator(),
                                                                             translate(function_expression.get_function_expressions()));
}

FunctionExpression ToMimirStructures::translate(const loki::FunctionExpressionMinusImpl& function_expression)
{
    return m_pddl_factories.get_or_create_function_expression_minus(translate(*function_expression.get_function_expression()));
}

FunctionExpression ToMimirStructures::translate(const loki::FunctionExpressionFunctionImpl& function_expression)
{
    return m_pddl_factories.get_or_create_function_expression_function(translate(*function_expression.get_function()));
}

FunctionExpression ToMimirStructures::translate(const loki::FunctionExpressionImpl& function_expression)
{
    return std::visit([this](auto&& arg) { return this->translate(arg); }, function_expression);
}

FunctionSkeleton ToMimirStructures::translate(const loki::FunctionSkeletonImpl& function_skeleton)
{
    return m_pddl_factories.get_or_create_function_skeleton(function_skeleton.get_name(), translate(function_skeleton.get_parameters()));
}

Function ToMimirStructures::translate(const loki::FunctionImpl& function)
{
    return m_pddl_factories.get_or_create_function(translate(*function.get_function_skeleton()), translate(function.get_terms()));
}

std::pair<ParameterList, LiteralList> ToMimirStructures::translate(const loki::ConditionImpl& condition)
{
    auto condition_ptr = &condition;

    auto parameters = ParameterList {};
    if (const auto condition_exists = std::get_if<loki::ConditionExistsImpl>(condition_ptr))
    {
        parameters = translate(condition_exists->get_parameters());

        condition_ptr = condition_exists->get_condition();
    }

    if (const auto condition_and = std::get_if<loki::ConditionAndImpl>(condition_ptr))
    {
        auto literals = LiteralList {};
        for (const auto& part : condition_and->get_conditions())
        {
            if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(part))
            {
                literals.push_back(translate(*condition_literal->get_literal()));
            }
            else
            {
                std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                throw std::logic_error("Expected literal in conjunctive condition.");
            }
        }
        return std::make_pair(parameters, literals);
    }
    else if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(condition_ptr))
    {
        return std::make_pair(parameters, LiteralList { translate(*condition_literal->get_literal()) });
    }

    std::cout << std::visit([](auto&& arg) { return arg.str(); }, *condition_ptr) << std::endl;

    throw std::logic_error("Expected conjunctive condition.");
}

EffectList ToMimirStructures::translate(const loki::EffectImpl& effect)
{
    auto effect_ptr = &effect;

    // 1. Parse conjunctive part
    if (const auto& effect_and = std::get_if<loki::EffectAndImpl>(effect_ptr))
    {
        auto result = EffectList {};
        for (const auto& nested_effect : effect_and->get_effects())
        {
            auto tmp_effect = nested_effect;

            // 2. Parse universal part
            auto parameters = ParameterList {};
            if (const auto& tmp_effect_forall = std::get_if<loki::EffectConditionalForallImpl>(tmp_effect))
            {
                parameters = translate(tmp_effect_forall->get_parameters());

                tmp_effect = tmp_effect_forall->get_effect();
            }

            // 3. Parse conditional part
            auto conditions = LiteralList {};
            if (const auto& tmp_effect_when = std::get_if<loki::EffectConditionalWhenImpl>(tmp_effect))
            {
                if (const auto condition_and = std::get_if<loki::ConditionAndImpl>(tmp_effect_when->get_condition()))
                {
                    for (const auto& part : condition_and->get_conditions())
                    {
                        if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(part))
                        {
                            conditions.push_back(translate(*condition_literal->get_literal()));
                        }
                        else
                        {
                            std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                            throw std::logic_error("Expected literal in conjunctive condition.");
                        }
                    }
                }
                else if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(tmp_effect_when->get_condition()))
                {
                    conditions = LiteralList { translate(*condition_literal->get_literal()) };
                }

                tmp_effect = tmp_effect_when->get_effect();
            }

            // 4. Parse simple effect
            if (const auto& effect_literal = std::get_if<loki::EffectLiteralImpl>(tmp_effect))
            {
                result.push_back(m_pddl_factories.get_or_create_simple_effect(parameters, conditions, translate(*effect_literal->get_literal())));
            }
            else if (const auto& effect_numeric = std::get_if<loki::EffectNumericImpl>(tmp_effect))
            {
                // TODO: implement how we should handle numeric effect
            }
            else
            {
                std::cout << std::visit([](auto&& arg) { return arg.str(); }, *tmp_effect) << std::endl;

                throw std::logic_error("Expected simple effect.");
            }
        }

        return result;
    }

    std::cout << std::visit([](auto&& arg) { return arg.str(); }, *effect_ptr) << std::endl;

    throw std::logic_error("Expected conjunctive effect.");
}

Action ToMimirStructures::translate(const loki::ActionImpl& action)
{
    auto parameters = translate(action.get_parameters());
    auto literals = LiteralList {};
    if (action.get_condition().has_value())
    {
        const auto [additional_parameters, parsed_literals] = translate(*action.get_condition().value());
        literals = parsed_literals;
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
    }

    auto effects = EffectList {};
    if (action.get_effect().has_value())
    {
        effects = translate(*action.get_effect().value());
    }

    return m_pddl_factories.get_or_create_action(action.get_name(), parameters, literals, effects);
}

Axiom ToMimirStructures::translate(const loki::AxiomImpl& axiom)
{
    // Turn predicate arguments into parameters
    auto parameters = translate(axiom.get_literal()->get_atom()->get_predicate()->get_parameters());

    // Turn quantifier variables into parameters
    const auto [additional_parameters, literals] = translate(*axiom.get_condition());
    parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());

    // Turn free variables into parameters
    for (const auto& free_variable : collect_free_variables(*axiom.get_condition()))
    {
        parameters.push_back(m_pddl_factories.get_or_create_parameter(translate(*free_variable)));
    }
    parameters.shrink_to_fit();

    return m_pddl_factories.get_or_create_axiom(parameters, translate(*axiom.get_literal()), literals);
}

OptimizationMetric ToMimirStructures::translate(const loki::OptimizationMetricImpl& optimization_metric)
{
    return m_pddl_factories.get_or_create_optimization_metric(optimization_metric.get_optimization_metric(),
                                                              translate(*optimization_metric.get_function_expression()));
}

Domain ToMimirStructures::translate(const loki::DomainImpl& domain)
{
    return m_pddl_factories.get_or_create_domain(domain.get_name(),
                                                 translate(*domain.get_requirements()),
                                                 translate(domain.get_constants()),
                                                 translate(domain.get_predicates()),
                                                 translate(domain.get_derived_predicates()),
                                                 translate(domain.get_functions()),
                                                 translate(domain.get_actions()),
                                                 translate(domain.get_axioms()));
}

ToMimirStructures::ToMimirStructures(PDDLFactories& pddl_factories) : m_pddl_factories(pddl_factories) {}

Problem ToMimirStructures::translate(const loki::ProblemImpl& problem)
{
    // Add constants to objects in problem.
    const auto constants = translate(problem.get_domain()->get_constants());
    auto objects = translate(problem.get_objects());
    objects.insert(objects.end(), constants.begin(), constants.end());

    auto goal_literals = LiteralList {};
    if (problem.get_goal_condition().has_value())
    {
        const auto [parameters, literals] = translate(*problem.get_goal_condition().value());

        if (!parameters.empty())
        {
            throw std::logic_error("Expected parameters to be empty.");
        }

        goal_literals = literals;
    }

    return m_pddl_factories.get_or_create_problem(translate(*problem.get_domain()),
                                                  problem.get_name(),
                                                  translate(*problem.get_requirements()),
                                                  objects,
                                                  translate(problem.get_derived_predicates()),
                                                  translate(problem.get_initial_literals()),
                                                  translate(problem.get_numeric_fluents()),
                                                  goal_literals,
                                                  (problem.get_optimization_metric().has_value() ?
                                                       std::optional<OptimizationMetric>(translate(*problem.get_optimization_metric().value())) :
                                                       std::nullopt),
                                                  translate(problem.get_axioms()));
}

}