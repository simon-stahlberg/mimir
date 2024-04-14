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

#include "mimir/formalism/translators/remove_types.hpp"

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

static void collect_types_from_type_hierarchy_recursively(const loki::Type& type, std::unordered_set<loki::Type>& ref_type_list)
{
    ref_type_list.insert(type);
    for (const auto& base_type : type->get_bases())
    {
        collect_types_from_type_hierarchy_recursively(base_type, ref_type_list);
    }
}

static loki::TypeList collect_types_from_type_hierarchy(const loki::TypeList& type_list)
{
    std::unordered_set<loki::Type> flat_type_set;
    for (const auto& type : type_list)
    {
        collect_types_from_type_hierarchy_recursively(type, flat_type_set);
    }
    return loki::TypeList(flat_type_set.begin(), flat_type_set.end());
}

static loki::Predicate type_to_predicate(const loki::TypeImpl& type, loki::PDDLFactories& pddl_factories)
{
    return pddl_factories.get_or_create_predicate(
        type.get_name(),
        loki::ParameterList { pddl_factories.get_or_create_parameter(pddl_factories.get_or_create_variable("?arg"), loki::TypeList {}) });
}

static loki::Object typed_object_to_untyped_object(const loki::ObjectImpl& object, loki::PDDLFactories& pddl_factories)
{
    return pddl_factories.get_or_create_object(object.get_name(), loki::TypeList {});
}

static loki::GroundLiteralList typed_object_to_ground_literals(const loki::ObjectImpl& object, loki::PDDLFactories& pddl_factories)
{
    auto additional_literals = loki::GroundLiteralList {};
    auto translated_object = typed_object_to_untyped_object(object, pddl_factories);
    auto types = collect_types_from_type_hierarchy(object.get_bases());
    for (const auto& type : types)
    {
        auto additional_literal = pddl_factories.get_or_create_ground_literal(
            false,
            pddl_factories.get_or_create_ground_atom(type_to_predicate(*type, pddl_factories), loki::ObjectList { translated_object }));
        additional_literals.push_back(additional_literal);
    }
    return additional_literals;
}

static loki::Parameter typed_parameter_to_untyped_parameter(const loki::ParameterImpl& parameter, loki::PDDLFactories& pddl_factories)
{
    auto translated_parameter = pddl_factories.get_or_create_parameter(parameter.get_variable(), loki::TypeList {});
    return translated_parameter;
}

static loki::ConditionList typed_parameter_to_condition_literals(const loki::ParameterImpl& parameter, loki::PDDLFactories& pddl_factories)
{
    auto conditions = loki::ConditionList {};
    auto types = collect_types_from_type_hierarchy(parameter.get_bases());
    for (const auto& type : types)
    {
        auto condition = pddl_factories.get_or_create_condition_literal(pddl_factories.get_or_create_literal(
            false,
            pddl_factories.get_or_create_atom(type_to_predicate(*type, pddl_factories),
                                              loki::TermList { pddl_factories.get_or_create_term_variable(parameter.get_variable()) })));
        conditions.push_back(condition);
    }
    return conditions;
}

loki::Object RemoveTypesTranslator::translate_impl(const loki::ObjectImpl& object) { return typed_object_to_untyped_object(object, this->m_pddl_factories); }

loki::Parameter RemoveTypesTranslator::translate_impl(const loki::ParameterImpl& parameter)
{
    return typed_parameter_to_untyped_parameter(parameter, this->m_pddl_factories);
}

loki::Condition RemoveTypesTranslator::translate_impl(const loki::ConditionExistsImpl& condition)
{
    // Translate parameters
    auto translated_parameters = this->translate(condition.get_parameters());

    // Translate condition
    auto conditions = loki::ConditionList {};
    for (const auto& parameter : condition.get_parameters())
    {
        auto additional_conditions = typed_parameter_to_condition_literals(*parameter, this->m_pddl_factories);
        conditions.insert(conditions.end(), additional_conditions.begin(), additional_conditions.end());
    }
    conditions.push_back(this->translate(*condition.get_condition()));

    auto translated_condition = this->m_pddl_factories.get_or_create_condition_and(conditions);

    return translated_condition;
}

loki::Condition RemoveTypesTranslator::translate_impl(const loki::ConditionForallImpl& condition)
{
    // Translate parameters
    auto translated_parameters = this->translate(condition.get_parameters());

    // Translate condition
    auto conditions = loki::ConditionList {};
    for (const auto& parameter : condition.get_parameters())
    {
        auto additional_conditions = typed_parameter_to_condition_literals(*parameter, this->m_pddl_factories);
        conditions.insert(conditions.end(), additional_conditions.begin(), additional_conditions.end());
    }
    conditions.push_back(this->translate(*condition.get_condition()));

    auto translated_condition = this->m_pddl_factories.get_or_create_condition_and(conditions);

    return this->m_pddl_factories.get_or_create_condition_forall(translated_parameters, translated_condition);
}

loki::Effect RemoveTypesTranslator::translate_impl(const loki::EffectConditionalForallImpl& effect)
{
    // Translate parameters
    auto translated_parameters = this->translate(effect.get_parameters());

    // Translate condition
    auto conditions = loki::ConditionList {};
    for (const auto& parameter : effect.get_parameters())
    {
        auto additional_conditions = typed_parameter_to_condition_literals(*parameter, this->m_pddl_factories);
        conditions.insert(conditions.end(), additional_conditions.begin(), additional_conditions.end());
    }
    auto translated_condition = this->m_pddl_factories.get_or_create_condition_and(conditions);

    // Translate effect
    auto translated_effect = this->translate(*effect.get_effect());

    return this->m_pddl_factories.get_or_create_effect_conditional_forall(
        translated_parameters,
        this->m_pddl_factories.get_or_create_effect_conditional_when(translated_condition, translated_effect));
}

loki::Action RemoveTypesTranslator::translate_impl(const loki::ActionImpl& action)
{
    // Translate parameters
    auto translated_parameters = this->translate(action.get_parameters());

    // Translate condition
    auto conditions = loki::ConditionList {};
    for (const auto& parameter : action.get_parameters())
    {
        auto additional_conditions = typed_parameter_to_condition_literals(*parameter, this->m_pddl_factories);
        conditions.insert(conditions.end(), additional_conditions.begin(), additional_conditions.end());
    }
    if (action.get_condition().has_value())
    {
        conditions.push_back(this->translate(*action.get_condition().value()));
    }
    auto translated_condition =
        conditions.empty() ? std::nullopt : std::optional<loki::Condition>(this->m_pddl_factories.get_or_create_condition_and(conditions));
    auto translated_effect = action.get_effect().has_value() ? std::optional<loki::Effect>(this->translate(*action.get_effect().value())) : std::nullopt;

    return this->m_pddl_factories.get_or_create_action(action.get_name(), translated_parameters, translated_condition, translated_effect);
}

loki::Domain RemoveTypesTranslator::translate_impl(const loki::DomainImpl& domain)
{
    // Remove :typing requirement
    auto requirements_enum_set = domain.get_requirements()->get_requirements();
    requirements_enum_set.erase(loki::RequirementEnum::TYPING);
    auto translated_requirements = this->m_pddl_factories.get_or_create_requirements(requirements_enum_set);

    // Make constants untyped
    auto translated_constants = loki::ObjectList {};
    translated_constants.reserve(domain.get_constants().size());
    for (const auto& object : domain.get_constants())
    {
        auto translated_object = typed_object_to_untyped_object(*object, this->m_pddl_factories);
        translated_constants.push_back(translated_object);
    }

    // Add type predicates
    auto translated_predicates = this->translate(domain.get_predicates());
    for (const auto type : collect_types_from_type_hierarchy(domain.get_types()))
    {
        translated_predicates.push_back(type_to_predicate(*type, this->m_pddl_factories));
    }

    auto translated_domain = this->m_pddl_factories.get_or_create_domain(domain.get_name(),
                                                                         translated_requirements,
                                                                         loki::TypeList {},
                                                                         translated_constants,
                                                                         translated_predicates,
                                                                         this->translate(domain.get_derived_predicates()),
                                                                         this->translate(domain.get_functions()),
                                                                         this->translate(domain.get_actions()),
                                                                         this->translate(domain.get_axioms()));
    return translated_domain;
}

loki::Problem RemoveTypesTranslator::translate_impl(const loki::ProblemImpl& problem)
{
    // Remove :typing requirement
    auto requirements_enum_set = problem.get_requirements()->get_requirements();
    requirements_enum_set.erase(loki::RequirementEnum::TYPING);
    auto translated_requirements = this->m_pddl_factories.get_or_create_requirements(requirements_enum_set);

    // Make objects untyped
    auto translated_objects = loki::ObjectList {};
    translated_objects.reserve(problem.get_objects().size());
    auto additional_initial_literals = loki::GroundLiteralList {};
    for (const auto& object : problem.get_objects())
    {
        auto translated_object = typed_object_to_untyped_object(*object, this->m_pddl_factories);
        auto additional_literals = typed_object_to_ground_literals(*object, this->m_pddl_factories);
        translated_objects.push_back(translated_object);
        additional_initial_literals.insert(additional_initial_literals.end(), additional_literals.begin(), additional_literals.end());
    }

    // Make constants untyped
    for (const auto& object : problem.get_domain()->get_constants())
    {
        auto additional_literals = typed_object_to_ground_literals(*object, this->m_pddl_factories);
        additional_initial_literals.insert(additional_initial_literals.end(), additional_literals.begin(), additional_literals.end());
    }

    // Translate other initial literals and add additional literals
    auto translated_initial_literals = this->translate(problem.get_initial_literals());
    translated_initial_literals.insert(translated_initial_literals.end(), additional_initial_literals.begin(), additional_initial_literals.end());

    auto translated_problem = this->m_pddl_factories.get_or_create_problem(
        this->translate(*problem.get_domain()),
        problem.get_name(),
        translated_requirements,
        translated_objects,
        this->translate(problem.get_derived_predicates()),
        translated_initial_literals,
        this->translate(problem.get_numeric_fluents()),
        (problem.get_goal_condition().has_value() ? std::optional<loki::Condition>(this->translate(*problem.get_goal_condition().value())) : std::nullopt),
        (problem.get_optimization_metric().has_value() ? std::optional<loki::OptimizationMetric>(this->translate(*problem.get_optimization_metric().value())) :
                                                         std::nullopt),
        this->translate(problem.get_axioms()));
    return translated_problem;
}

loki::Problem RemoveTypesTranslator::run_impl(const loki::ProblemImpl& problem) { return self().translate(problem); }

RemoveTypesTranslator::RemoveTypesTranslator(loki::PDDLFactories& pddl_factories) : BaseTranslator<RemoveTypesTranslator>(pddl_factories) {}

}
