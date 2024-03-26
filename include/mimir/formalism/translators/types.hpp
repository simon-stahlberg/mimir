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

#ifndef MIMIR_FORMALISM_TRANSLATORS_TYPES_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_TYPES_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{
class TypeTranslator : public BaseTranslator<TypeTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<TypeTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    /// @brief Translate a type to a predicate of arity 1.
    loki::pddl::Predicate translate_type_to_predicate(const loki::pddl::TypeImpl& type)
    {
        return this->m_pddl_factories.predicates.template get_or_create<loki::pddl::PredicateImpl>(
            type.get_name(),
            loki::pddl::ParameterList { this->m_pddl_factories.parameters.template get_or_create<loki::pddl::ParameterImpl>(
                this->m_pddl_factories.variables.template get_or_create<loki::pddl::VariableImpl>("?arg"),
                loki::pddl::TypeList {}) });
    }

    /// @brief Translate an typed object to an untyped objects.
    loki::pddl::Object translate_typed_object_to_untyped_object(const loki::pddl::ObjectImpl& object)
    {
        return this->m_pddl_factories.objects.template get_or_create<loki::pddl::ObjectImpl>(object.get_name(), loki::pddl::TypeList {});
    }

    /// @brief Translate an typed object to a list of ground literal for all its original types.
    loki::pddl::GroundLiteralList translate_typed_object_to_ground_literals(const loki::pddl::ObjectImpl& object)
    {
        auto additional_literals = loki::pddl::GroundLiteralList {};
        auto translated_object = translate_typed_object_to_untyped_object(object);
        auto types = collect_types_from_type_hierarchy(object.get_bases());
        for (const auto& type : types)
        {
            auto additional_literal = this->m_pddl_factories.ground_literals.template get_or_create<loki::pddl::GroundLiteralImpl>(
                false,
                this->m_pddl_factories.ground_atoms.template get_or_create<loki::pddl::GroundAtomImpl>(translate_type_to_predicate(*type),
                                                                                                       loki::pddl::ObjectList { translated_object }));
            additional_literals.push_back(additional_literal);
        }
        return additional_literals;
    }

    /// @brief Translate a typed parameter to an untyped parameter.
    loki::pddl::Parameter translate_typed_parameter_to_untyped_parameter(const loki::pddl::ParameterImpl& parameter)
    {
        auto translated_parameter =
            this->m_pddl_factories.parameters.template get_or_create<loki::pddl::ParameterImpl>(this->translate(*parameter.get_variable()),
                                                                                                loki::pddl::TypeList {});
        return translated_parameter;
    }

    /// @brief Helper to compute all types from a hierarchy of types.
    void collect_types_from_type_hierarchy_recursively(const loki::pddl::Type& type, std::unordered_set<loki::pddl::Type>& ref_type_list)
    {
        ref_type_list.insert(type);
        for (const auto& base_type : type->get_bases())
        {
            collect_types_from_type_hierarchy_recursively(base_type, ref_type_list);
        }
    }

    /// @brief Translate an typed parameter to a list of condition literal for all its original types.
    loki::pddl::ConditionList translate_typed_parameter_to_condition_literals(const loki::pddl::ParameterImpl& parameter)
    {
        auto conditions = loki::pddl::ConditionList {};
        auto translated_variable = this->translate(*parameter.get_variable());
        auto types = collect_types_from_type_hierarchy(parameter.get_bases());
        for (const auto& type : types)
        {
            auto condition = this->m_pddl_factories.conditions.template get_or_create<loki::pddl::ConditionLiteralImpl>(
                this->m_pddl_factories.literals.template get_or_create<loki::pddl::LiteralImpl>(
                    false,
                    this->m_pddl_factories.atoms.template get_or_create<loki::pddl::AtomImpl>(
                        translate_type_to_predicate(*type),
                        loki::pddl::TermList { this->m_pddl_factories.terms.template get_or_create<loki::pddl::TermVariableImpl>(translated_variable) })));
            conditions.push_back(condition);
        }
        return conditions;
    }

    /// @brief Compute all types from a hierarchy of types.
    loki::pddl::TypeList collect_types_from_type_hierarchy(const loki::pddl::TypeList& type_list)
    {
        std::unordered_set<loki::pddl::Type> flat_type_set;
        for (const auto& type : type_list)
        {
            collect_types_from_type_hierarchy_recursively(type, flat_type_set);
        }
        return loki::pddl::TypeList(flat_type_set.begin(), flat_type_set.end());
    }

    /**
     * Translate
     */
    loki::pddl::Object translate_impl(const loki::pddl::ObjectImpl& object) { return translate_typed_object_to_untyped_object(object); }

    loki::pddl::Parameter translate_impl(const loki::pddl::ParameterImpl& parameter) { return translate_typed_parameter_to_untyped_parameter(parameter); }

    loki::pddl::Condition translate_impl(const loki::pddl::ConditionExistsImpl& condition)
    {
        // Translate parameters
        auto translated_parameters = this->translate(condition.get_parameters());

        // Translate condition
        auto conditions = loki::pddl::ConditionList {};
        for (const auto& parameter : condition.get_parameters())
        {
            auto additional_conditions = translate_typed_parameter_to_condition_literals(*parameter);
            conditions.insert(conditions.end(), additional_conditions.begin(), additional_conditions.end());
        }
        conditions.push_back(this->translate(*condition.get_condition()));

        auto translated_condition = this->m_pddl_factories.conditions.template get_or_create<loki::pddl::ConditionAndImpl>(conditions);

        return this->m_pddl_factories.conditions.template get_or_create<loki::pddl::ConditionExistsImpl>(translated_parameters, translated_condition);
    }

    loki::pddl::Condition translate_impl(const loki::pddl::ConditionForallImpl& condition)
    {
        // Translate parameters
        auto translated_parameters = this->translate(condition.get_parameters());

        // Translate condition
        auto conditions = loki::pddl::ConditionList {};
        for (const auto& parameter : condition.get_parameters())
        {
            auto additional_conditions = translate_typed_parameter_to_condition_literals(*parameter);
            conditions.insert(conditions.end(), additional_conditions.begin(), additional_conditions.end());
        }
        conditions.push_back(this->translate(*condition.get_condition()));

        auto translated_condition = this->m_pddl_factories.conditions.template get_or_create<loki::pddl::ConditionAndImpl>(conditions);

        return this->m_pddl_factories.conditions.template get_or_create<loki::pddl::ConditionForallImpl>(translated_parameters, translated_condition);
    }

    loki::pddl::Effect translate_impl(const loki::pddl::EffectConditionalForallImpl& effect)
    {
        // Translate parameters
        auto translated_parameters = this->translate(effect.get_parameters());

        // Translate condition
        auto conditions = loki::pddl::ConditionList {};
        for (const auto& parameter : effect.get_parameters())
        {
            auto additional_conditions = translate_typed_parameter_to_condition_literals(*parameter);
            conditions.insert(conditions.end(), additional_conditions.begin(), additional_conditions.end());
        }
        auto translated_condition = this->m_pddl_factories.conditions.template get_or_create<loki::pddl::ConditionAndImpl>(conditions);

        // Translate effect
        auto translated_effect = this->translate(*effect.get_effect());

        return this->m_pddl_factories.effects.template get_or_create<loki::pddl::EffectConditionalForallImpl>(
            translated_parameters,
            this->m_pddl_factories.effects.template get_or_create<loki::pddl::EffectConditionalWhenImpl>(translated_condition, translated_effect));
    }

    loki::pddl::Action translate_impl(const loki::pddl::ActionImpl& action)
    {
        // Translate parameters
        auto translated_parameters = this->translate(action.get_parameters());

        // Translate condition
        auto conditions = loki::pddl::ConditionList {};
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
            conditions.empty() ?
                std::nullopt :
                std::optional<loki::pddl::Condition>(this->m_pddl_factories.conditions.template get_or_create<loki::pddl::ConditionAndImpl>(conditions));

        return this->m_pddl_factories.actions.template get_or_create<loki::pddl::ActionImpl>(
            action.get_name(),
            translated_parameters,
            translated_condition,
            (action.get_effect().has_value() ? std::optional<loki::pddl::Effect>(this->translate(*action.get_effect().value())) : std::nullopt));
    }

    loki::pddl::Domain translate_impl(const loki::pddl::DomainImpl& domain)
    {
        // Remove :typing requirement
        auto requirements_enum_set = domain.get_requirements()->get_requirements();
        requirements_enum_set.erase(loki::pddl::RequirementEnum::TYPING);
        auto translated_requirements = this->m_pddl_factories.requirements.template get_or_create<loki::pddl::RequirementsImpl>(requirements_enum_set);

        // Make constants untyped
        auto translated_constants = loki::pddl::ObjectList {};
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
        return this->m_pddl_factories.domains.template get_or_create<loki::pddl::DomainImpl>(domain.get_name(),
                                                                                             translated_requirements,
                                                                                             loki::pddl::TypeList {},
                                                                                             translated_constants,
                                                                                             translated_predicates,
                                                                                             this->translate(domain.get_functions()),
                                                                                             this->translate(domain.get_actions()),
                                                                                             this->translate(domain.get_derived_predicates()));
    }

    loki::pddl::Problem translate_impl(const loki::pddl::ProblemImpl& problem)
    {
        // Remove :typing requirement
        auto requirements_enum_set = problem.get_requirements()->get_requirements();
        requirements_enum_set.erase(loki::pddl::RequirementEnum::TYPING);
        auto translated_requirements = this->m_pddl_factories.requirements.template get_or_create<loki::pddl::RequirementsImpl>(requirements_enum_set);

        // Make objects untyped
        auto translated_objects = loki::pddl::ObjectList {};
        translated_objects.reserve(problem.get_objects().size());
        auto additional_initial_literals = loki::pddl::GroundLiteralList {};
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

        return this->m_pddl_factories.problems.template get_or_create<loki::pddl::ProblemImpl>(
            this->translate(*problem.get_domain()),
            problem.get_name(),
            translated_requirements,
            translated_objects,
            translated_initial_literals,
            this->translate(problem.get_numeric_fluents()),
            this->translate(*problem.get_goal_condition()),
            (problem.get_optimization_metric().has_value() ?
                 std::optional<loki::pddl::OptimizationMetric>(this->translate(*problem.get_optimization_metric().value())) :
                 std::nullopt));
    }

    loki::pddl::Problem run_impl(const loki::pddl::ProblemImpl& problem) { return self().translate(problem); }

public:
    explicit TypeTranslator(loki::PDDLFactories& pddl_factories) : BaseTranslator<TypeTranslator>(pddl_factories) {}
};

}
#endif
