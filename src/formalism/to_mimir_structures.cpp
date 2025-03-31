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

#include "to_mimir_structures.hpp"

#include "mimir/common/collections.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/domain_builder.hpp"
#include "mimir/formalism/problem_builder.hpp"
#include "mimir/formalism/repositories.hpp"

#include <loki/details/utils/hash.hpp>
#include <numeric>
#include <typeinfo>
#include <unordered_map>

namespace mimir::formalism
{

/**
 * Prepare common
 */
void ToMimirStructures::prepare(loki::FunctionSkeleton function_skeleton)
{
    prepare(function_skeleton->get_parameters());
    prepare(function_skeleton->get_type());
}
void ToMimirStructures::prepare(loki::Object object) { prepare(object->get_bases()); }
void ToMimirStructures::prepare(loki::Parameter parameter) { prepare(parameter->get_variable()); }
void ToMimirStructures::prepare(loki::Predicate predicate) { prepare(predicate->get_parameters()); }
void ToMimirStructures::prepare(loki::Requirements requirements) {}
void ToMimirStructures::prepare(loki::Type type) { prepare(type->get_bases()); }
void ToMimirStructures::prepare(loki::Variable variabl) {}

/**
 * Prepare lifted
 */

void ToMimirStructures::prepare(loki::Term term)
{
    std::visit([&](auto&& arg) { return this->prepare(arg); }, term->get_object_or_variable());
}

void ToMimirStructures::prepare(loki::Atom atom)
{
    prepare(atom->get_predicate());
    prepare(atom->get_terms());
}
void ToMimirStructures::prepare(loki::Literal literal) { prepare(literal->get_atom()); }
void ToMimirStructures::prepare(loki::FunctionExpressionNumber function_expression) {}
void ToMimirStructures::prepare(loki::FunctionExpressionBinaryOperator function_expression)
{
    prepare(function_expression->get_left_function_expression());
    prepare(function_expression->get_right_function_expression());
}
void ToMimirStructures::prepare(loki::FunctionExpressionMultiOperator function_expression) { this->prepare(function_expression->get_function_expressions()); }
void ToMimirStructures::prepare(loki::FunctionExpressionMinus function_expression) { this->prepare(function_expression->get_function_expression()); }
void ToMimirStructures::prepare(loki::FunctionExpressionFunction function_expression)
{
    m_fexpr_functions.insert(function_expression->get_function()->get_function_skeleton()->get_name());

    this->prepare(function_expression->get_function());
}
void ToMimirStructures::prepare(loki::FunctionExpression function_expression)
{
    std::visit([&](auto&& arg) { return this->prepare(arg); }, function_expression->get_function_expression());
}
void ToMimirStructures::prepare(loki::Function function)
{
    prepare(function->get_function_skeleton());
    prepare(function->get_terms());
}
void ToMimirStructures::prepare(loki::Condition condition)
{
    if (const auto condition_and = std::get_if<loki::ConditionAnd>(&condition->get_condition()))
    {
        for (const auto& part : (*condition_and)->get_conditions())
        {
            if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&part->get_condition()))
            {
                prepare((*condition_literal)->get_literal());
            }
            else if (const auto condition_numeric = std::get_if<loki::ConditionNumericConstraint>(&part->get_condition()))
            {
                prepare((*condition_numeric)->get_left_function_expression());
                prepare((*condition_numeric)->get_right_function_expression());
            }
            else
            {
                // std::visit([](auto&& arg) { std::cout << arg << std::endl; }, part->get_condition());

                throw std::logic_error("Expected literal in conjunctive condition.");
            }
        }
    }
    else if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&condition->get_condition()))
    {
        prepare((*condition_literal)->get_literal());
    }
    else
    {
        // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *condition_ptr) << std::endl;

        throw std::logic_error("Expected conjunctive condition.");
    }
}

void ToMimirStructures::prepare(loki::Effect effect)
{
    const auto prepare_effect_func = [&](loki::Effect arg_effect)
    {
        auto tmp_effect = arg_effect;

        // 2. Prepare universal part
        if (const auto& tmp_effect_forall = std::get_if<loki::EffectCompositeForall>(&tmp_effect->get_effect()))
        {
            prepare((*tmp_effect_forall)->get_parameters());

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
                        prepare((*condition_literal)->get_literal());
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
                prepare((*condition_literal)->get_literal());
            }

            tmp_effect = (*tmp_effect_when)->get_effect();
        }

        // 4. Parse simple effect
        if (const auto& effect_literal = std::get_if<loki::EffectLiteral>(&tmp_effect->get_effect()))
        {
            prepare((*effect_literal)->get_literal());

            // Found predicate affected by an effect
            m_fluent_predicates.insert((*effect_literal)->get_literal()->get_atom()->get_predicate()->get_name());
        }
        else if (const auto& effect_numeric = std::get_if<loki::EffectNumeric>(&tmp_effect->get_effect()))
        {
            // Found function affected by an effect
            m_effect_function_skeletons.insert((*effect_numeric)->get_function()->get_function_skeleton()->get_name());

            prepare((*effect_numeric)->get_function_expression());
        }
        else
        {
            // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *tmp_effect) << std::endl;

            throw std::logic_error("Expected simple effect.");
        }
    };

    // 1. Prepare conjunctive part
    if (const auto& effect_and = std::get_if<loki::EffectAnd>(&effect->get_effect()))
    {
        for (const auto& nested_effect : (*effect_and)->get_effects())
        {
            prepare_effect_func(nested_effect);
        }
    }
    else
    {
        prepare_effect_func(effect);
    }
}
void ToMimirStructures::prepare(loki::Action action)
{
    prepare(action->get_parameters());
    prepare(action->get_condition());
    prepare(action->get_effect());
}
void ToMimirStructures::prepare(loki::Axiom axiom)
{
    prepare(axiom->get_parameters());
    prepare(axiom->get_literal());
    prepare(axiom->get_condition());

    m_derived_predicates.insert(axiom->get_literal()->get_atom()->get_predicate()->get_name());
}
void ToMimirStructures::prepare(loki::FunctionValue function_value) { prepare(function_value->get_function()); }
void ToMimirStructures::prepare(loki::OptimizationMetric metric) { prepare(metric->get_function_expression()); }

void ToMimirStructures::prepare(loki::Domain domain)
{
    prepare(domain->get_requirements());
    prepare(domain->get_types());
    prepare(domain->get_constants());
    prepare(domain->get_predicates());
    prepare(domain->get_function_skeletons());
    prepare(domain->get_actions());
    prepare(domain->get_axioms());
}

void ToMimirStructures::prepare(loki::Problem problem)
{
    prepare(problem->get_domain());
    prepare(problem->get_requirements());
    prepare(problem->get_objects());
    prepare(problem->get_predicates());
    prepare(problem->get_initial_literals());
    prepare(problem->get_initial_function_values());
    prepare(problem->get_goal_condition());
    prepare(problem->get_optimization_metric());
    prepare(problem->get_axioms());

    for (const auto& derived_predicate : problem->get_predicates())
    {
        m_derived_predicates.insert(derived_predicate->get_name());
    }
}

/**
 * Common
 */

StaticOrFluentOrAuxiliaryFunctionSkeleton ToMimirStructures::translate_common(loki::FunctionSkeleton function_skeleton, Repositories& repositories)
{
    // Now that we cannot analyze the metric function anymore, we can only work with total cost as auxiliary...
    if (function_skeleton->get_name() == "total-cost")
    {
        return repositories.template get_or_create_function_skeleton<AuxiliaryTag>(function_skeleton->get_name(),
                                                                                   translate_common(function_skeleton->get_parameters(), repositories));
    }
    else if (m_effect_function_skeletons.contains(function_skeleton->get_name()))
    {
        return repositories.template get_or_create_function_skeleton<FluentTag>(function_skeleton->get_name(),
                                                                                translate_common(function_skeleton->get_parameters(), repositories));
    }
    else
    {
        return repositories.template get_or_create_function_skeleton<StaticTag>(function_skeleton->get_name(),
                                                                                translate_common(function_skeleton->get_parameters(), repositories));
    }
}

Variable ToMimirStructures::translate_common(loki::Parameter parameter, Repositories& repositories)
{
    return translate_common(parameter->get_variable(), repositories);
}

Requirements ToMimirStructures::translate_common(loki::Requirements requirements, Repositories& repositories)
{
    return repositories.get_or_create_requirements(requirements->get_requirements());
}

Variable ToMimirStructures::translate_common(loki::Variable variable, Repositories& repositories)
{
    return repositories.get_or_create_variable(variable->get_name(), 0);
}

Object ToMimirStructures::translate_common(loki::Object object, Repositories& repositories)
{
    assert(object->get_bases().empty());
    return repositories.get_or_create_object(object->get_name());
}

StaticOrFluentOrDerivedPredicate ToMimirStructures::translate_common(loki::Predicate predicate, Repositories& repositories)
{
    auto parameters = translate_common(predicate->get_parameters(), repositories);

    if (m_fluent_predicates.count(predicate->get_name()) && !m_derived_predicates.count(predicate->get_name()))
    {
        return repositories.get_or_create_predicate<FluentTag>(predicate->get_name(), parameters);
    }
    else if (m_derived_predicates.count(predicate->get_name()))
    {
        return repositories.get_or_create_predicate<DerivedTag>(predicate->get_name(), parameters);
    }
    else
    {
        return repositories.get_or_create_predicate<StaticTag>(predicate->get_name(), parameters);
    }
}

/**
 * Lifted
 */

Term ToMimirStructures::translate_lifted(loki::Term term, Repositories& repositories)
{
    return std::visit([&](auto&& arg) -> Term { return repositories.get_or_create_term(this->translate_common(arg, repositories)); },
                      term->get_object_or_variable());
}

StaticOrFluentOrDerivedAtom ToMimirStructures::translate_lifted(loki::Atom atom, Repositories& repositories)
{
    auto static_or_fluent_or_derived_predicate = translate_common(atom->get_predicate(), repositories);

    return std::visit([&](auto&& arg) -> StaticOrFluentOrDerivedAtom
                      { return repositories.get_or_create_atom(arg, translate_lifted(atom->get_terms(), repositories)); },
                      static_or_fluent_or_derived_predicate);
}

StaticOrFluentOrDerivedLiteral ToMimirStructures::translate_lifted(loki::Literal literal, Repositories& repositories)
{
    auto static_or_fluent_or_derived_atom = translate_lifted(literal->get_atom(), repositories);

    return std::visit([&](auto&& arg) -> StaticOrFluentOrDerivedLiteral { return repositories.get_or_create_literal(literal->get_polarity(), arg); },
                      static_or_fluent_or_derived_atom);
}

FunctionExpression ToMimirStructures::translate_lifted(loki::FunctionExpressionNumber function_expression, Repositories& repositories)
{
    return repositories.get_or_create_function_expression(repositories.get_or_create_function_expression_number(function_expression->get_number()));
}

FunctionExpression ToMimirStructures::translate_lifted(loki::FunctionExpressionBinaryOperator function_expression, Repositories& repositories)
{
    return repositories.get_or_create_function_expression(
        repositories.get_or_create_function_expression_binary_operator(function_expression->get_binary_operator(),
                                                                       translate_lifted(function_expression->get_left_function_expression(), repositories),
                                                                       translate_lifted(function_expression->get_right_function_expression(), repositories)));
}

FunctionExpression ToMimirStructures::translate_lifted(loki::FunctionExpressionMultiOperator function_expression, Repositories& repositories)
{
    return repositories.get_or_create_function_expression(
        repositories.get_or_create_function_expression_multi_operator(function_expression->get_multi_operator(),
                                                                      translate_lifted(function_expression->get_function_expressions(), repositories)));
}

FunctionExpression ToMimirStructures::translate_lifted(loki::FunctionExpressionMinus function_expression, Repositories& repositories)
{
    return repositories.get_or_create_function_expression(
        repositories.get_or_create_function_expression_minus(translate_lifted(function_expression->get_function_expression(), repositories)));
}

FunctionExpression ToMimirStructures::translate_lifted(loki::FunctionExpressionFunction function_expression, Repositories& repositories)
{
    return std::visit(
        [&](auto&& function) -> FunctionExpression
        {
            using T = std::decay_t<decltype(function)>;
            if constexpr (std::is_same_v<T, Function<StaticTag>>)
            {
                return repositories.get_or_create_function_expression(repositories.get_or_create_function_expression_function(function));
            }
            else if constexpr (std::is_same_v<T, Function<FluentTag>>)
            {
                return repositories.get_or_create_function_expression(repositories.get_or_create_function_expression_function(function));
            }
            else if constexpr (std::is_same_v<T, Function<AuxiliaryTag>>)
            {
                throw std::logic_error("ToMimirStructures::translate_lifted: FunctionExpressionFunction<AuxiliaryTag> must not exist.");
            }
            else
            {
                static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for Function type.");
            }
        },
        translate_lifted(function_expression->get_function(), repositories));
}

FunctionExpression ToMimirStructures::translate_lifted(loki::FunctionExpression function_expression, Repositories& repositories)
{
    return std::visit([&](auto&& arg) { return this->translate_lifted(arg, repositories); }, function_expression->get_function_expression());
}

NumericConstraint ToMimirStructures::translate_lifted(loki::ConditionNumericConstraint condition, Repositories& repositories)
{
    return repositories.get_or_create_numeric_constraint(condition->get_binary_comparator(),
                                                         translate_lifted(condition->get_left_function_expression(), repositories),
                                                         translate_lifted(condition->get_right_function_expression(), repositories),
                                                         TermList {});
}

StaticOrFluentOrAuxiliaryFunction ToMimirStructures::translate_lifted(loki::Function function, Repositories& repositories)
{
    return std::visit([&](auto&& function_skeleton) -> StaticOrFluentOrAuxiliaryFunction
                      { return repositories.get_or_create_function(function_skeleton, translate_lifted(function->get_terms(), repositories), IndexList {}); },
                      translate_common(function->get_function_skeleton(), repositories));
}

ConjunctiveCondition ToMimirStructures::translate_lifted(loki::Condition condition, const VariableList& parameters, Repositories& repositories)
{
    const auto func_insert_literal =
        [](const StaticOrFluentOrDerivedLiteral& static_or_fluent_or_derived_literal, LiteralLists<StaticTag, FluentTag, DerivedTag>& ref_literals)
    {
        std::visit(
            [&ref_literals](auto&& arg)
            {
                using Type = typename std::decay_t<decltype(*arg)>::Type;

                boost::hana::at_key(ref_literals, boost::hana::type<Type> {}).push_back(arg);
            },
            static_or_fluent_or_derived_literal);
    };

    if (const auto condition_and = std::get_if<loki::ConditionAnd>(&condition->get_condition()))
    {
        auto literals = LiteralLists<StaticTag, FluentTag, DerivedTag> {};
        auto numeric_constraints = NumericConstraintList {};

        for (const auto& part : (*condition_and)->get_conditions())
        {
            if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&part->get_condition()))
            {
                const auto static_or_fluent_literal = translate_lifted((*condition_literal)->get_literal(), repositories);

                func_insert_literal(static_or_fluent_literal, literals);
            }
            else if (const auto condition_numeric_constraint = std::get_if<loki::ConditionNumericConstraint>(&part->get_condition()))
            {
                const auto numeric_constraint = translate_lifted((*condition_numeric_constraint), repositories);

                numeric_constraints.push_back(numeric_constraint);
            }
            else
            {
                // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                throw std::logic_error("Expected literal in conjunctive condition.");
            }
        }
        return repositories.get_or_create_conjunctive_condition(parameters, literals, numeric_constraints);
    }
    else if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&condition->get_condition()))
    {
        auto literals = LiteralLists<StaticTag, FluentTag, DerivedTag> {};
        auto numeric_constraints = NumericConstraintList {};

        const auto static_or_fluent_or_derived_literal = translate_lifted((*condition_literal)->get_literal(), repositories);

        func_insert_literal(static_or_fluent_or_derived_literal, literals);

        return repositories.get_or_create_conjunctive_condition(parameters, literals, numeric_constraints);
    }

    // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *condition_ptr) << std::endl;

    throw std::logic_error("Expected conjunctive condition.");
}

std::tuple<ConjunctiveEffect, ConditionalEffectList>
ToMimirStructures::translate_lifted(loki::Effect effect, const VariableList& parameters, Repositories& repositories)
{
    using ConjunctiveEffectData = std::tuple<LiteralList<FluentTag>, NumericEffectList<FluentTag>, std::optional<NumericEffect<AuxiliaryTag>>>;
    using ConditionalEffectData =
        std::unordered_map<ConjunctiveCondition, std::tuple<LiteralList<FluentTag>, NumericEffectList<FluentTag>, std::optional<NumericEffect<AuxiliaryTag>>>>;

    const auto translate_effect_func =
        [&](loki::Effect effect, ConjunctiveEffectData& ref_conjunctive_effect_data, ConditionalEffectData& ref_conditional_effect_data)
    {
        auto tmp_effect = effect;

        /* 1. Parse universal part. */
        auto parameters = VariableList {};
        if (const auto& tmp_effect_forall = std::get_if<loki::EffectCompositeForall>(&tmp_effect->get_effect()))
        {
            parameters = translate_common((*tmp_effect_forall)->get_parameters(), repositories);

            tmp_effect = (*tmp_effect_forall)->get_effect();
        }

        /* 2. Parse conditional part */
        auto conjunctive_condition = ConjunctiveCondition { nullptr };
        if (const auto tmp_effect_when = std::get_if<loki::EffectCompositeWhen>(&tmp_effect->get_effect()))
        {
            conjunctive_condition = translate_lifted((*tmp_effect_when)->get_condition(), parameters, repositories);

            tmp_effect = (*tmp_effect_when)->get_effect();
        }

        // Fetch container to store the effects
        auto& effect_data = (conjunctive_condition) ? ref_conditional_effect_data[conjunctive_condition] : ref_conjunctive_effect_data;
        auto& data_fluent_literals = std::get<0>(effect_data);
        auto& data_fluent_numeric_effects = std::get<1>(effect_data);
        auto& data_auxiliary_numeric_effect = std::get<2>(effect_data);

        /* 3. Parse effect part */
        if (const auto& effect_literal = std::get_if<loki::EffectLiteral>(&tmp_effect->get_effect()))
        {
            const auto static_or_fluent_or_derived_effect = translate_lifted((*effect_literal)->get_literal(), repositories);

            if (std::get_if<Literal<DerivedTag>>(&static_or_fluent_or_derived_effect))
            {
                throw std::runtime_error("Only fluent literals are allowed in effects!");
            }
            else if (std::get_if<Literal<StaticTag>>(&static_or_fluent_or_derived_effect))
            {
                throw std::logic_error("Expected fluent effect literal but it was static!");
            }

            const auto fluent_effect = std::get<Literal<FluentTag>>(static_or_fluent_or_derived_effect);

            data_fluent_literals.push_back(fluent_effect);
        }
        else if (const auto& effect_numeric = std::get_if<loki::EffectNumeric>(&tmp_effect->get_effect()))
        {
            const auto static_fluent_or_auxiliary_function = translate_lifted((*effect_numeric)->get_function(), repositories);
            const auto function_expression = translate_lifted((*effect_numeric)->get_function_expression(), repositories);

            std::visit(
                [&](auto&& function)
                {
                    using T = std::decay_t<decltype(function)>;
                    if constexpr (std::is_same_v<T, Function<FluentTag>>)
                    {
                        data_fluent_numeric_effects.push_back(
                            repositories.get_or_create_numeric_effect<FluentTag>((*effect_numeric)->get_assign_operator(), function, function_expression));
                    }
                    else if constexpr (std::is_same_v<T, Function<AuxiliaryTag>>)
                    {
                        assert(!data_auxiliary_numeric_effect.has_value());
                        data_auxiliary_numeric_effect =
                            repositories.get_or_create_numeric_effect<AuxiliaryTag>((*effect_numeric)->get_assign_operator(), function, function_expression);
                    }
                    else if constexpr (std::is_same_v<T, Function<StaticTag>>)
                    {
                        throw std::logic_error("ToMimirStructures::translate_lifted: Function<StaticTag> must not exist.");
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
    auto conjunctive_effect_data = ConjunctiveEffectData {};
    auto conditional_effect_data = ConditionalEffectData {};
    // Parse conjunctive part
    if (const auto& effect_and = std::get_if<loki::EffectAnd>(&effect->get_effect()))
    {
        for (const auto& nested_effect : (*effect_and)->get_effects())
        {
            translate_effect_func(nested_effect, conjunctive_effect_data, conditional_effect_data);
        }
    }
    else
    {
        // Parse non conjunctive
        translate_effect_func(effect, conjunctive_effect_data, conditional_effect_data);
    }

    /* Instantiate conjunctive effect. */
    auto& [conjunctive_effect_fluent_literals, conjunctive_effect_fluent_numeric_effects, conjunctive_effect_auxiliary_numeric_effects] =
        conjunctive_effect_data;

    const auto conjunctive_effect = repositories.get_or_create_conjunctive_effect(parameters,
                                                                                  conjunctive_effect_fluent_literals,
                                                                                  conjunctive_effect_fluent_numeric_effects,
                                                                                  conjunctive_effect_auxiliary_numeric_effects);

    /* Instantiate conditional effects. */
    auto conditional_effects = ConditionalEffectList {};
    for (const auto& [cond_conjunctive_condition, value] : conditional_effect_data)
    {
        const auto& [cond_effect_fluent_literals, cond_effect_fluent_numeric_effects, cond_effect_auxiliary_numeric_effects] = value;

        conditional_effects.push_back(
            repositories.get_or_create_conditional_effect(cond_conjunctive_condition,
                                                          repositories.get_or_create_conjunctive_effect(parameters,
                                                                                                        cond_effect_fluent_literals,
                                                                                                        cond_effect_fluent_numeric_effects,
                                                                                                        cond_effect_auxiliary_numeric_effects)));
    }

    return { conjunctive_effect, conditional_effects };
}

Action ToMimirStructures::translate_lifted(loki::Action action, Repositories& repositories)
{
    // We sort the additional parameters to enforce some additional approximate syntactic equivalence.
    auto translated_parameters = translate_common(action->get_parameters(), repositories);
    std::sort(translated_parameters.begin() + action->get_original_arity(), translated_parameters.end());

    // 1. Translate conditions
    auto conjunctive_condition = ConjunctiveCondition { nullptr };
    if (action->get_condition().has_value())
    {
        conjunctive_condition = translate_lifted(action->get_condition().value(), translated_parameters, repositories);
    }

    // 2. Translate effects
    auto conjunctive_effect = ConjunctiveEffect { nullptr };
    auto conditional_effects = ConditionalEffectList {};
    if (action->get_effect().has_value())
    {
        const auto [conjunctive_effect_, conditional_effects_] = translate_lifted(action->get_effect().value(), translated_parameters, repositories);
        conjunctive_effect = conjunctive_effect_;
        conditional_effects = conditional_effects_;
    }
    else
    {
        // TODO: actions without effects are useless....
        conjunctive_effect =
            repositories.get_or_create_conjunctive_effect(translated_parameters, LiteralList<FluentTag> {}, NumericEffectList<FluentTag> {}, std::nullopt);
    }

    return repositories.get_or_create_action(action->get_name(), action->get_original_arity(), conjunctive_condition, conjunctive_effect, conditional_effects);
}

Axiom ToMimirStructures::translate_lifted(loki::Axiom axiom, Repositories& repositories)
{
    auto parameters = translate_common(axiom->get_parameters(), repositories);

    const auto conjunctive_condition = translate_lifted(axiom->get_condition(), parameters, repositories);
    const auto literal = translate_lifted(axiom->get_literal(), repositories);

    return std::visit(
        [&](auto&& arg) -> Axiom
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Literal<DerivedTag>>)
            {
                return repositories.get_or_create_axiom(conjunctive_condition, arg);
            }
            else
            {
                throw std::runtime_error("ToMimirStructures::translate_lifted: Expected Literal<DerivedTag> in axiom head.");
            }
        },
        literal);
}

/**
 * Grounded
 */

Object ToMimirStructures::translate_grounded(loki::Term term, Repositories& repositories)
{
    if (const auto object = std::get_if<loki::Object>(&term->get_object_or_variable()))
    {
        return translate_common(*object, repositories);
    }

    throw std::logic_error("Expected ground term.");
}

StaticOrFluentOrDerivedGroundAtom ToMimirStructures::translate_grounded(loki::Atom atom, Repositories& repositories)
{
    auto static_or_fluent_or_derived_predicate = translate_common(atom->get_predicate(), repositories);

    return std::visit([&](auto&& arg) -> StaticOrFluentOrDerivedGroundAtom
                      { return repositories.get_or_create_ground_atom(arg, translate_grounded(atom->get_terms(), repositories)); },
                      static_or_fluent_or_derived_predicate);
}

StaticOrFluentOrDerivedGroundLiteral ToMimirStructures::translate_grounded(loki::Literal literal, Repositories& repositories)
{
    auto static_or_fluent_or_derived_ground_atom = translate_grounded(literal->get_atom(), repositories);

    return std::visit([&literal, &repositories](auto&& arg) -> StaticOrFluentOrDerivedGroundLiteral
                      { return repositories.get_or_create_ground_literal(literal->get_polarity(), arg); },
                      static_or_fluent_or_derived_ground_atom);
}

StaticOrFluentOrAuxiliaryGroundFunctionValue ToMimirStructures::translate_grounded(loki::FunctionValue function_value, Repositories& repositories)
{
    const auto static_or_fluent_or_auxiliary_ground_function = translate_grounded(function_value->get_function(), repositories);

    return std::visit([&](auto&& ground_function) -> StaticOrFluentOrAuxiliaryGroundFunctionValue
                      { return repositories.get_or_create_ground_function_value(ground_function, function_value->get_number()); },
                      static_or_fluent_or_auxiliary_ground_function);
}

GroundFunctionExpression ToMimirStructures::translate_grounded(loki::FunctionExpressionNumber function_expression, Repositories& repositories)
{
    return repositories.get_or_create_ground_function_expression(
        repositories.get_or_create_ground_function_expression_number(function_expression->get_number()));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(loki::FunctionExpressionBinaryOperator function_expression, Repositories& repositories)
{
    return repositories.get_or_create_ground_function_expression(repositories.get_or_create_ground_function_expression_binary_operator(
        function_expression->get_binary_operator(),
        translate_grounded(function_expression->get_left_function_expression(), repositories),
        translate_grounded(function_expression->get_right_function_expression(), repositories)));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(loki::FunctionExpressionMultiOperator function_expression, Repositories& repositories)
{
    return repositories.get_or_create_ground_function_expression(repositories.get_or_create_ground_function_expression_multi_operator(
        function_expression->get_multi_operator(),
        translate_grounded(function_expression->get_function_expressions(), repositories)));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(loki::FunctionExpressionMinus function_expression, Repositories& repositories)
{
    return repositories.get_or_create_ground_function_expression(
        repositories.get_or_create_ground_function_expression_minus(translate_grounded(function_expression->get_function_expression(), repositories)));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(loki::FunctionExpressionFunction function_expression, Repositories& repositories)
{
    return std::visit(
        [&](auto&& ground_function)
        { return repositories.get_or_create_ground_function_expression(repositories.get_or_create_ground_function_expression_function(ground_function)); },
        translate_grounded(function_expression->get_function(), repositories));
}

GroundFunctionExpression ToMimirStructures::translate_grounded(loki::FunctionExpression function_expression, Repositories& repositories)
{
    return std::visit([&](auto&& arg) { return this->translate_grounded(arg, repositories); }, function_expression->get_function_expression());
}

StaticOrFluentOrAuxiliaryGroundFunction ToMimirStructures::translate_grounded(loki::Function function, Repositories& repositories)
{
    const auto static_or_fluent_or_auxiliary_function_skeleton = translate_common(function->get_function_skeleton(), repositories);
    const auto objects = translate_grounded(function->get_terms(), repositories);

    return std::visit([&](auto&& function_skeleton) -> StaticOrFluentOrAuxiliaryGroundFunction
                      { return repositories.get_or_create_ground_function(function_skeleton, objects); },
                      static_or_fluent_or_auxiliary_function_skeleton);
}

GroundNumericConstraint ToMimirStructures::translate_grounded(loki::ConditionNumericConstraint condition, Repositories& repositories)
{
    return repositories.get_or_create_ground_numeric_constraint(condition->get_binary_comparator(),
                                                                translate_grounded(condition->get_left_function_expression(), repositories),
                                                                translate_grounded(condition->get_right_function_expression(), repositories));
}

std::tuple<GroundLiteralList<StaticTag>, GroundLiteralList<FluentTag>, GroundLiteralList<DerivedTag>, GroundNumericConstraintList>
ToMimirStructures::translate_grounded(loki::Condition condition, Repositories& repositories)
{
    const auto func_insert_ground_literal = [](const StaticOrFluentOrDerivedGroundLiteral& static_or_fluent_or_derived_literal,
                                               GroundLiteralList<StaticTag>& ref_static_ground_literals,
                                               GroundLiteralList<FluentTag>& ref_fluent_ground_literals,
                                               GroundLiteralList<DerivedTag>& ref_derived_ground_literals)
    {
        std::visit(
            [&ref_static_ground_literals, &ref_fluent_ground_literals, &ref_derived_ground_literals](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, GroundLiteral<StaticTag>>)
                {
                    ref_static_ground_literals.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, GroundLiteral<FluentTag>>)
                {
                    ref_fluent_ground_literals.push_back(arg);
                }
                else if constexpr (std::is_same_v<T, GroundLiteral<DerivedTag>>)
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

    if (const auto condition_and = std::get_if<loki::ConditionAnd>(&condition->get_condition()))
    {
        auto static_ground_literals = GroundLiteralList<StaticTag> {};
        auto fluent_ground_literals = GroundLiteralList<FluentTag> {};
        auto derived_ground_literals = GroundLiteralList<DerivedTag> {};
        auto numeric_constraints = GroundNumericConstraintList {};

        for (const auto& part : (*condition_and)->get_conditions())
        {
            if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&part->get_condition()))
            {
                const auto static_or_fluent_ground_literal = translate_grounded((*condition_literal)->get_literal(), repositories);

                func_insert_ground_literal(static_or_fluent_ground_literal, static_ground_literals, fluent_ground_literals, derived_ground_literals);
            }
            else if (const auto condition_numeric = std::get_if<loki::ConditionNumericConstraint>(&part->get_condition()))
            {
                numeric_constraints.push_back(translate_grounded(*condition_numeric, repositories));
            }
            else
            {
                // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                throw std::logic_error("Expected literal in conjunctive condition.");
            }
        }
        return std::make_tuple(static_ground_literals, fluent_ground_literals, derived_ground_literals, numeric_constraints);
    }
    else if (const auto condition_literal = std::get_if<loki::ConditionLiteral>(&condition->get_condition()))
    {
        auto static_ground_literals = GroundLiteralList<StaticTag> {};
        auto fluent_ground_literals = GroundLiteralList<FluentTag> {};
        auto derived_ground_literals = GroundLiteralList<DerivedTag> {};
        auto numeric_constraints = GroundNumericConstraintList {};

        const auto static_or_fluent_or_derived_ground_literal = translate_grounded((*condition_literal)->get_literal(), repositories);

        func_insert_ground_literal(static_or_fluent_or_derived_ground_literal, static_ground_literals, fluent_ground_literals, derived_ground_literals);

        return std::make_tuple(static_ground_literals, fluent_ground_literals, derived_ground_literals, numeric_constraints);
    }
    else if (const auto& condition_numeric = std::get_if<loki::ConditionNumericConstraint>(&condition->get_condition()))
    {
        auto static_ground_literals = GroundLiteralList<StaticTag> {};
        auto fluent_ground_literals = GroundLiteralList<FluentTag> {};
        auto derived_ground_literals = GroundLiteralList<DerivedTag> {};
        auto numeric_constraints = GroundNumericConstraintList {};

        numeric_constraints.push_back(translate_grounded(*condition_numeric, repositories));

        return std::make_tuple(static_ground_literals, fluent_ground_literals, derived_ground_literals, numeric_constraints);
    }

    // std::cout << std::visit([](auto&& arg) { return arg.str(); }, *condition_ptr) << std::endl;

    throw std::logic_error("Expected conjunctive condition.");
}

OptimizationMetric ToMimirStructures::translate_grounded(loki::OptimizationMetric optimization_metric, Repositories& repositories)
{
    return repositories.get_or_create_optimization_metric(optimization_metric->get_optimization_metric(),
                                                          translate_grounded(optimization_metric->get_function_expression(), repositories));
}

Domain ToMimirStructures::translate(const loki::Domain& domain, DomainBuilder& builder)
{
    /* Perform static type analysis */
    prepare(domain);

    auto& repositories = builder.get_repositories();

    /* Requirements section */
    const auto requirements = translate_common(domain->get_requirements(), repositories);

    /* Constants section */
    const auto constants = translate_common(domain->get_constants(), repositories);

    /* Predicates section */
    auto predicates = PredicateLists<StaticTag, FluentTag, DerivedTag> {};
    for (const auto& static_or_fluent_or_derived_predicate : translate_common(domain->get_predicates(), repositories))
    {
        std::visit(
            [&predicates](auto&& arg)
            {
                using Type = typename std::decay_t<decltype(*arg)>::Type;

                boost::hana::at_key(predicates, boost::hana::type<Type> {}).push_back(arg);
            },
            static_or_fluent_or_derived_predicate);
    }

    /* Functions section */
    auto function_skeletons = FunctionSkeletonLists<StaticTag, FluentTag> {};
    auto auxiliary_function = std::optional<FunctionSkeleton<AuxiliaryTag>> { std::nullopt };
    for (const auto& static_or_fluent_or_auxiliary_function : translate_common(domain->get_function_skeletons(), repositories))
    {
        std::visit(
            [&function_skeletons, &auxiliary_function](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, FunctionSkeleton<StaticTag>> || std::is_same_v<T, FunctionSkeleton<FluentTag>>)
                {
                    using Type = typename std::decay_t<decltype(*arg)>::Type;

                    boost::hana::at_key(function_skeletons, boost::hana::type<Type> {}).push_back(arg);
                }
                else if constexpr (std::is_same_v<T, FunctionSkeleton<AuxiliaryTag>>)
                {
                    assert(!auxiliary_function.has_value());
                    auxiliary_function = arg;
                }
                else
                {
                    static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for FunctionSkeleton type.");
                }
            },
            static_or_fluent_or_auxiliary_function);
    }

    /* Structures section */
    const auto actions = translate_lifted(domain->get_actions(), repositories);
    const auto axioms = translate_lifted(domain->get_axioms(), repositories);

    builder.get_filepath() = domain->get_filepath();
    builder.get_name() = domain->get_name();
    builder.get_requirements() = requirements;
    builder.get_constants() = std::move(constants);
    builder.get_hana_predicates() = std::move(predicates);
    builder.get_hana_function_skeletons() = std::move(function_skeletons);
    builder.get_auxiliary_function_skeleton() = auxiliary_function;
    builder.get_actions() = std::move(actions);
    builder.get_axioms() = std::move(axioms);

    return builder.get_result();
}

Problem ToMimirStructures::translate(const loki::Problem& problem, ProblemBuilder& builder)
{
    /* Perform static type analysis */
    prepare(problem);

    auto& repositories = builder.get_repositories();

    /* Requirements section */
    const auto requirements = translate_common(problem->get_requirements(), repositories);

    /* Objects section */
    auto objects = translate_common(problem->get_objects(), repositories);

    /* Predicates section */
    auto predicates = PredicateLists<StaticTag, FluentTag, DerivedTag> {};
    for (const auto& static_or_fluent_or_derived_predicate : translate_common(problem->get_predicates(), repositories))
    {
        std::visit(
            [&predicates](auto&& arg)
            {
                using Type = typename std::decay_t<decltype(*arg)>::Type;

                boost::hana::at_key(predicates, boost::hana::type<Type> {}).push_back(arg);
            },
            static_or_fluent_or_derived_predicate);
    }
    assert(boost::hana::at_key(predicates, boost::hana::type<StaticTag> {}).empty());
    assert(boost::hana::at_key(predicates, boost::hana::type<FluentTag> {}).empty());
    auto derived_predicates = boost::hana::at_key(predicates, boost::hana::type<DerivedTag> {});

    /* Initial section */
    auto tmp_initial_literals = GroundLiteralLists<StaticTag, FluentTag, DerivedTag> {};
    for (const auto& static_or_fluent_or_derived_ground_literal : translate_grounded(problem->get_initial_literals(), repositories))
    {
        std::visit(
            [&tmp_initial_literals](auto&& arg)
            {
                using Type = typename std::decay_t<decltype(*arg)>::Type;

                boost::hana::at_key(tmp_initial_literals, boost::hana::type<Type> {}).push_back(arg);
            },
            static_or_fluent_or_derived_ground_literal);
    }
    auto initial_literals = GroundLiteralLists<StaticTag, FluentTag> {};
    boost::hana::at_key(initial_literals, boost::hana::type<StaticTag> {}) =
        uniquify_elements(boost::hana::at_key(tmp_initial_literals, boost::hana::type<StaticTag> {}));  ///< filter duplicates
    boost::hana::at_key(initial_literals, boost::hana::type<FluentTag> {}) =
        uniquify_elements(boost::hana::at_key(tmp_initial_literals, boost::hana::type<FluentTag> {}));  ///< filter duplicates
    assert(boost::hana::at_key(tmp_initial_literals, boost::hana::type<DerivedTag> {}).empty());  ///< ensure that no derived ground atom appears in initial

    auto initial_function_values = GroundFunctionValueLists<StaticTag, FluentTag> {};
    auto initial_auxiliary_function_value = std::optional<GroundFunctionValue<AuxiliaryTag>> {};
    for (const auto static_or_fluent_or_auxiliary_function_value : translate_grounded(problem->get_initial_function_values(), repositories))
    {
        std::visit(
            [&initial_function_values, &initial_auxiliary_function_value](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, GroundFunctionValue<StaticTag>> || std::is_same_v<T, GroundFunctionValue<FluentTag>>)
                {
                    using Type = typename std::decay_t<decltype(*arg)>::Type;

                    boost::hana::at_key(initial_function_values, boost::hana::type<Type> {}).push_back(arg);
                }
                else if constexpr (std::is_same_v<T, GroundFunctionValue<AuxiliaryTag>>)
                {
                    assert(!initial_auxiliary_function_value.has_value());
                    initial_auxiliary_function_value = arg;
                }
                else
                {
                    static_assert(dependent_false<T>::value, "ToMimirStructures::translate_lifted: Missing implementation for GroundFunctionValue type.");
                }
            },
            static_or_fluent_or_auxiliary_function_value);
    }

    /* Goal section */
    auto goal_literals = GroundLiteralLists<StaticTag, FluentTag, DerivedTag> {};
    auto numeric_goal_constraints = GroundNumericConstraintList {};
    if (problem->get_goal_condition().has_value())
    {
        const auto [static_goal_literals_, fluent_goal_literals_, derived_goal_literals_, numeric_goal_constraints_] =
            translate_grounded(problem->get_goal_condition().value(), repositories);

        boost::hana::at_key(goal_literals, boost::hana::type<StaticTag> {}) = static_goal_literals_;
        boost::hana::at_key(goal_literals, boost::hana::type<FluentTag> {}) = fluent_goal_literals_;
        boost::hana::at_key(goal_literals, boost::hana::type<DerivedTag> {}) = derived_goal_literals_;
        numeric_goal_constraints = numeric_goal_constraints_;
    }

    /* Metric section */
    const auto metric = (problem->get_optimization_metric().has_value()) ?
                            std::optional<OptimizationMetric>(translate_grounded(problem->get_optimization_metric().value(), repositories)) :
                            std::nullopt;

    /* Structures section */
    const auto axioms = translate_lifted(problem->get_axioms(), repositories);

    builder.get_filepath() = problem->get_filepath();
    builder.get_name() = problem->get_name();
    builder.get_requirements() = requirements;
    builder.get_objects() = std::move(objects);
    builder.get_derived_predicates() = std::move(derived_predicates);
    builder.get_hana_initial_literals() = std::move(initial_literals);
    builder.get_hana_initial_function_values() = std::move(initial_function_values);
    builder.get_auxiliary_function_value() = initial_auxiliary_function_value;
    builder.get_hana_goal_condition() = std::move(goal_literals);
    builder.get_numeric_goal_condition() = std::move(numeric_goal_constraints);
    builder.get_optimization_metric() = metric;
    builder.get_axioms() = std::move(axioms);

    return builder.get_result(problem->get_index());
}
}