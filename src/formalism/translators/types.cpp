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

#include "mimir/formalism/translators/types.hpp"

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

Object TypeTranslator::translate_impl(const ObjectImpl& object) { return translate_typed_object_to_untyped_object(object, this->m_pddl_factories); }

Parameter TypeTranslator::translate_impl(const ParameterImpl& parameter)
{
    return translate_typed_parameter_to_untyped_parameter(parameter, this->m_pddl_factories);
}

Condition TypeTranslator::translate_impl(const ConditionExistsImpl& condition)
{
    // Translate parameters
    auto translated_parameters = this->translate(condition.get_parameters());

    // Translate condition
    auto conditions = ConditionList {};
    for (const auto& parameter : condition.get_parameters())
    {
        auto additional_conditions = translate_typed_parameter_to_condition_literals(*parameter, this->m_pddl_factories);
        conditions.insert(conditions.end(), additional_conditions.begin(), additional_conditions.end());
    }
    conditions.push_back(this->translate(*condition.get_condition()));

    auto translated_condition = this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(conditions);

    return translated_condition;
}

Condition TypeTranslator::translate_impl(const ConditionForallImpl& condition)
{
    // Translate parameters
    auto translated_parameters = this->translate(condition.get_parameters());

    // Translate condition
    auto conditions = ConditionList {};
    for (const auto& parameter : condition.get_parameters())
    {
        auto additional_conditions = translate_typed_parameter_to_condition_literals(*parameter, this->m_pddl_factories);
        conditions.insert(conditions.end(), additional_conditions.begin(), additional_conditions.end());
    }
    conditions.push_back(this->translate(*condition.get_condition()));

    auto translated_condition = this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(conditions);

    return this->m_pddl_factories.conditions.template get_or_create<ConditionForallImpl>(translated_parameters, translated_condition);
}

Effect TypeTranslator::translate_impl(const EffectConditionalForallImpl& effect)
{
    // Translate parameters
    auto translated_parameters = this->translate(effect.get_parameters());

    // Translate condition
    auto conditions = ConditionList {};
    for (const auto& parameter : effect.get_parameters())
    {
        auto additional_conditions = translate_typed_parameter_to_condition_literals(*parameter, this->m_pddl_factories);
        conditions.insert(conditions.end(), additional_conditions.begin(), additional_conditions.end());
    }
    auto translated_condition = this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(conditions);

    // Translate effect
    auto translated_effect = this->translate(*effect.get_effect());

    return this->m_pddl_factories.effects.template get_or_create<EffectConditionalForallImpl>(
        translated_parameters,
        this->m_pddl_factories.effects.template get_or_create<EffectConditionalWhenImpl>(translated_condition, translated_effect));
}

Action TypeTranslator::translate_impl(const ActionImpl& action)
{
    // Translate parameters
    auto translated_parameters = this->translate(action.get_parameters());

    // Translate condition
    auto conditions = ConditionList {};
    for (const auto& parameter : action.get_parameters())
    {
        auto additional_conditions = translate_typed_parameter_to_condition_literals(*parameter, this->m_pddl_factories);
        conditions.insert(conditions.end(), additional_conditions.begin(), additional_conditions.end());
    }
    if (action.get_condition().has_value())
    {
        conditions.push_back(this->translate(*action.get_condition().value()));
    }
    auto translated_condition =
        conditions.empty() ? std::nullopt : std::optional<Condition>(this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(conditions));

    return this->m_pddl_factories.actions.template get_or_create<ActionImpl>(
        action.get_name(),
        translated_parameters,
        translated_condition,
        (action.get_effect().has_value() ? std::optional<Effect>(this->translate(*action.get_effect().value())) : std::nullopt));
}

Domain TypeTranslator::translate_impl(const DomainImpl& domain)
{
    // Remove :typing requirement
    auto requirements_enum_set = domain.get_requirements()->get_requirements();
    requirements_enum_set.erase(loki::pddl::RequirementEnum::TYPING);
    auto translated_requirements = this->m_pddl_factories.requirements.template get_or_create<RequirementsImpl>(requirements_enum_set);

    // Make constants untyped
    auto translated_constants = ObjectList {};
    translated_constants.reserve(domain.get_constants().size());
    for (const auto& object : domain.get_constants())
    {
        auto translated_object = translate_typed_object_to_untyped_object(*object, this->m_pddl_factories);
        translated_constants.push_back(translated_object);
    }

    // Add type predicates
    auto translated_predicates = this->translate(domain.get_predicates());
    for (const auto type : collect_types_from_type_hierarchy(domain.get_types()))
    {
        translated_predicates.push_back(translate_type_to_predicate(*type, this->m_pddl_factories));
    }

    auto translated_domain = this->m_pddl_factories.domains.template get_or_create<DomainImpl>(domain.get_name(),
                                                                                               translated_requirements,
                                                                                               TypeList {},
                                                                                               translated_constants,
                                                                                               translated_predicates,
                                                                                               this->translate(domain.get_functions()),
                                                                                               this->translate(domain.get_actions()),
                                                                                               this->translate(domain.get_derived_predicates()));
    return translated_domain;
}

Problem TypeTranslator::translate_impl(const ProblemImpl& problem)
{
    // Remove :typing requirement
    auto requirements_enum_set = problem.get_requirements()->get_requirements();
    requirements_enum_set.erase(loki::pddl::RequirementEnum::TYPING);
    auto translated_requirements = this->m_pddl_factories.requirements.template get_or_create<RequirementsImpl>(requirements_enum_set);

    // Make objects untyped
    auto translated_objects = ObjectList {};
    translated_objects.reserve(problem.get_objects().size());
    auto additional_initial_literals = GroundLiteralList {};
    for (const auto& object : problem.get_objects())
    {
        auto translated_object = translate_typed_object_to_untyped_object(*object, this->m_pddl_factories);
        auto additional_literals = translate_typed_object_to_ground_literals(*object, this->m_pddl_factories);
        translated_objects.push_back(translated_object);
        additional_initial_literals.insert(additional_initial_literals.end(), additional_literals.begin(), additional_literals.end());
    }

    // Make constants untyped
    for (const auto& object : problem.get_domain()->get_constants())
    {
        auto additional_literals = translate_typed_object_to_ground_literals(*object, this->m_pddl_factories);
        additional_initial_literals.insert(additional_initial_literals.end(), additional_literals.begin(), additional_literals.end());
    }

    // Translate other initial literals and add additional literals
    auto translated_initial_literals = this->translate(problem.get_initial_literals());
    translated_initial_literals.insert(translated_initial_literals.end(), additional_initial_literals.begin(), additional_initial_literals.end());

    auto translated_problem = this->m_pddl_factories.problems.template get_or_create<ProblemImpl>(
        this->translate(*problem.get_domain()),
        problem.get_name(),
        translated_requirements,
        translated_objects,
        translated_initial_literals,
        this->translate(problem.get_numeric_fluents()),
        this->translate(*problem.get_goal_condition()),
        (problem.get_optimization_metric().has_value() ? std::optional<OptimizationMetric>(this->translate(*problem.get_optimization_metric().value())) :
                                                         std::nullopt));
    return translated_problem;
}

Problem TypeTranslator::run_impl(const ProblemImpl& problem) { return self().translate(problem); }

TypeTranslator::TypeTranslator(PDDLFactories& pddl_factories) : BaseTranslator<TypeTranslator>(pddl_factories) {}

}
