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

namespace mimir
{

Predicate TypeTranslator::translate_type_to_predicate(const TypeImpl& type)
{
    return this->m_pddl_factories.predicates.template get_or_create<PredicateImpl>(
        type.get_name(),
        ParameterList { this->m_pddl_factories.parameters.template get_or_create<ParameterImpl>(
            this->m_pddl_factories.variables.template get_or_create<VariableImpl>("?arg"),
            TypeList {}) });
}

Object TypeTranslator::translate_typed_object_to_untyped_object(const ObjectImpl& object)
{
    return this->m_pddl_factories.objects.template get_or_create<ObjectImpl>(object.get_name(), TypeList {});
}

GroundLiteralList TypeTranslator::translate_typed_object_to_ground_literals(const ObjectImpl& object)
{
    auto additional_literals = GroundLiteralList {};
    auto translated_object = translate_typed_object_to_untyped_object(object);
    auto types = collect_types_from_type_hierarchy(object.get_bases());
    for (const auto& type : types)
    {
        auto additional_literal = this->m_pddl_factories.ground_literals.template get_or_create<GroundLiteralImpl>(
            false,
            this->m_pddl_factories.ground_atoms.template get_or_create<GroundAtomImpl>(translate_type_to_predicate(*type), ObjectList { translated_object }));
        additional_literals.push_back(additional_literal);
    }
    return additional_literals;
}

Parameter TypeTranslator::translate_typed_parameter_to_untyped_parameter(const ParameterImpl& parameter)
{
    auto translated_parameter =
        this->m_pddl_factories.parameters.template get_or_create<ParameterImpl>(this->translate(*parameter.get_variable()), TypeList {});
    return translated_parameter;
}

void TypeTranslator::collect_types_from_type_hierarchy_recursively(const Type& type, std::unordered_set<Type>& ref_type_list)
{
    ref_type_list.insert(type);
    for (const auto& base_type : type->get_bases())
    {
        collect_types_from_type_hierarchy_recursively(base_type, ref_type_list);
    }
}

ConditionList TypeTranslator::translate_typed_parameter_to_condition_literals(const ParameterImpl& parameter)
{
    auto conditions = ConditionList {};
    auto translated_variable = this->translate(*parameter.get_variable());
    auto types = collect_types_from_type_hierarchy(parameter.get_bases());
    for (const auto& type : types)
    {
        auto condition =
            this->m_pddl_factories.conditions.template get_or_create<ConditionLiteralImpl>(this->m_pddl_factories.literals.template get_or_create<LiteralImpl>(
                false,
                this->m_pddl_factories.atoms.template get_or_create<AtomImpl>(
                    translate_type_to_predicate(*type),
                    TermList { this->m_pddl_factories.terms.template get_or_create<TermVariableImpl>(translated_variable) })));
        conditions.push_back(condition);
    }
    return conditions;
}

TypeList TypeTranslator::collect_types_from_type_hierarchy(const TypeList& type_list)
{
    std::unordered_set<Type> flat_type_set;
    for (const auto& type : type_list)
    {
        collect_types_from_type_hierarchy_recursively(type, flat_type_set);
    }
    return TypeList(flat_type_set.begin(), flat_type_set.end());
}

Object TypeTranslator::translate_impl(const ObjectImpl& object) { return translate_typed_object_to_untyped_object(object); }

Parameter TypeTranslator::translate_impl(const ParameterImpl& parameter) { return translate_typed_parameter_to_untyped_parameter(parameter); }

Condition TypeTranslator::translate_impl(const ConditionExistsImpl& condition)
{
    // Translate parameters
    auto translated_parameters = this->translate(condition.get_parameters());

    // Translate condition
    auto conditions = ConditionList {};
    for (const auto& parameter : condition.get_parameters())
    {
        auto additional_conditions = translate_typed_parameter_to_condition_literals(*parameter);
        conditions.insert(conditions.end(), additional_conditions.begin(), additional_conditions.end());
    }
    conditions.push_back(this->translate(*condition.get_condition()));

    auto translated_condition = this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(conditions);

    return this->m_pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(translated_parameters, translated_condition);
}

Condition TypeTranslator::translate_impl(const ConditionForallImpl& condition)
{
    // Translate parameters
    auto translated_parameters = this->translate(condition.get_parameters());

    // Translate condition
    auto conditions = ConditionList {};
    for (const auto& parameter : condition.get_parameters())
    {
        auto additional_conditions = translate_typed_parameter_to_condition_literals(*parameter);
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
        auto additional_conditions = translate_typed_parameter_to_condition_literals(*parameter);
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
        auto additional_conditions = translate_typed_parameter_to_condition_literals(*parameter);
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
        auto translated_object = translate_typed_object_to_untyped_object(*object);
        translated_constants.push_back(translated_object);
    }

    // Add type predicates
    auto translated_predicates = this->translate(domain.get_predicates());
    for (const auto type : collect_types_from_type_hierarchy(domain.get_types()))
    {
        translated_predicates.push_back(translate_type_to_predicate(*type));
    }

    return this->m_pddl_factories.domains.template get_or_create<DomainImpl>(domain.get_name(),
                                                                             translated_requirements,
                                                                             TypeList {},
                                                                             translated_constants,
                                                                             translated_predicates,
                                                                             this->translate(domain.get_functions()),
                                                                             this->translate(domain.get_actions()),
                                                                             this->translate(domain.get_derived_predicates()));
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
        auto translated_object = translate_typed_object_to_untyped_object(*object);
        auto additional_literals = translate_typed_object_to_ground_literals(*object);
        translated_objects.push_back(translated_object);
        additional_initial_literals.insert(additional_initial_literals.end(), additional_literals.begin(), additional_literals.end());
    }

    // Make constants untyped
    for (const auto& object : problem.get_domain()->get_constants())
    {
        auto additional_literals = translate_typed_object_to_ground_literals(*object);
        additional_initial_literals.insert(additional_initial_literals.end(), additional_literals.begin(), additional_literals.end());
    }

    // Translate other initial literals and add additional literals
    auto translated_initial_literals = this->translate(problem.get_initial_literals());
    translated_initial_literals.insert(translated_initial_literals.end(), additional_initial_literals.begin(), additional_initial_literals.end());

    return this->m_pddl_factories.problems.template get_or_create<ProblemImpl>(
        this->translate(*problem.get_domain()),
        problem.get_name(),
        translated_requirements,
        translated_objects,
        translated_initial_literals,
        this->translate(problem.get_numeric_fluents()),
        this->translate(*problem.get_goal_condition()),
        (problem.get_optimization_metric().has_value() ? std::optional<OptimizationMetric>(this->translate(*problem.get_optimization_metric().value())) :
                                                         std::nullopt));
}

Problem TypeTranslator::run_impl(const ProblemImpl& problem) { return self().translate(problem); }

TypeTranslator::TypeTranslator(PDDLFactories& pddl_factories) : BaseTranslator<TypeTranslator>(pddl_factories) {}

}
