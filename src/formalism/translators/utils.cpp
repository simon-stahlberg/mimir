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

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

/**
 * Types
 */
Predicate type_to_predicate(const TypeImpl& type, PDDLFactories& pddl_factories)
{
    return pddl_factories.predicates.template get_or_create<PredicateImpl>(
        type.get_name(),
        ParameterList { pddl_factories.parameters.template get_or_create<ParameterImpl>(pddl_factories.variables.template get_or_create<VariableImpl>("?arg"),
                                                                                        TypeList {}) });
}

Object typed_object_to_untyped_object(const ObjectImpl& object, PDDLFactories& pddl_factories)
{
    return pddl_factories.objects.template get_or_create<ObjectImpl>(object.get_name(), TypeList {});
}

GroundLiteralList typed_object_to_ground_literals(const ObjectImpl& object, PDDLFactories& pddl_factories)
{
    auto additional_literals = GroundLiteralList {};
    auto translated_object = typed_object_to_untyped_object(object, pddl_factories);
    auto types = collect_types_from_type_hierarchy(object.get_bases());
    for (const auto& type : types)
    {
        auto additional_literal = pddl_factories.ground_literals.template get_or_create<GroundLiteralImpl>(
            false,
            pddl_factories.ground_atoms.template get_or_create<GroundAtomImpl>(type_to_predicate(*type, pddl_factories), ObjectList { translated_object }));
        additional_literals.push_back(additional_literal);
    }
    return additional_literals;
}

Parameter typed_parameter_to_untyped_parameter(const ParameterImpl& parameter, PDDLFactories& pddl_factories)
{
    auto translated_parameter = pddl_factories.parameters.template get_or_create<ParameterImpl>(parameter.get_variable(), TypeList {});
    return translated_parameter;
}

ConditionList typed_parameter_to_condition_literals(const ParameterImpl& parameter, PDDLFactories& pddl_factories)
{
    auto conditions = ConditionList {};
    auto types = collect_types_from_type_hierarchy(parameter.get_bases());
    for (const auto& type : types)
    {
        auto condition = pddl_factories.conditions.template get_or_create<ConditionLiteralImpl>(pddl_factories.literals.template get_or_create<LiteralImpl>(
            false,
            pddl_factories.atoms.template get_or_create<AtomImpl>(
                type_to_predicate(*type, pddl_factories),
                TermList { pddl_factories.terms.template get_or_create<TermVariableImpl>(parameter.get_variable()) })));
        conditions.push_back(condition);
    }
    return conditions;
}

static void collect_types_from_type_hierarchy_recursively(const Type& type, std::unordered_set<Type>& ref_type_list)
{
    ref_type_list.insert(type);
    for (const auto& base_type : type->get_bases())
    {
        collect_types_from_type_hierarchy_recursively(base_type, ref_type_list);
    }
}

TypeList collect_types_from_type_hierarchy(const TypeList& type_list)
{
    std::unordered_set<Type> flat_type_set;
    for (const auto& type : type_list)
    {
        collect_types_from_type_hierarchy_recursively(type, flat_type_set);
    }
    return TypeList(flat_type_set.begin(), flat_type_set.end());
}

/**
 * Conditions
 */

Condition flatten_conjunctions(const ConditionAndImpl& condition, PDDLFactories& pddl_factories)
{
    auto parts = ConditionSet {};
    for (const auto& nested_condition : condition.get_conditions())
    {
        if (const auto and_condition = std::get_if<ConditionAndImpl>(nested_condition))
        {
            const auto nested_parts = std::get_if<ConditionAndImpl>(flatten_conjunctions(*and_condition, pddl_factories));

            parts.insert(nested_parts->get_conditions().begin(), nested_parts->get_conditions().end());
        }
        else
        {
            parts.insert(nested_condition);
        }
    }
    return pddl_factories.conditions.template get_or_create<ConditionAndImpl>(ConditionList(parts.begin(), parts.end()));
}

Condition flatten_disjunctions(const ConditionOrImpl& condition, PDDLFactories& pddl_factories)
{
    auto parts = ConditionSet {};
    for (const auto& nested_condition : condition.get_conditions())
    {
        if (const auto or_condition = std::get_if<ConditionOrImpl>(nested_condition))
        {
            const auto nested_parts = std::get_if<ConditionOrImpl>(flatten_disjunctions(*or_condition, pddl_factories));

            parts.insert(nested_parts->get_conditions().begin(), nested_parts->get_conditions().end());
        }
        else
        {
            parts.insert(nested_condition);
        }
    }
    return pddl_factories.conditions.template get_or_create<ConditionOrImpl>(ConditionList(parts.begin(), parts.end()));
}

Condition flatten_existential_quantifier(const ConditionExistsImpl& condition, PDDLFactories& pddl_factories)
{
    if (const auto condition_exists = std::get_if<ConditionExistsImpl>(condition.get_condition()))
    {
        const auto nested_condition = std::get_if<ConditionExistsImpl>(flatten_existential_quantifier(*condition_exists, pddl_factories));
        auto parameters = condition.get_parameters();
        const auto additional_parameters = nested_condition->get_parameters();
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
        return pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(parameters, nested_condition->get_condition());
    }
    return pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(condition.get_parameters(), condition.get_condition());
}

Condition flatten_universal_quantifier(const ConditionForallImpl& condition, PDDLFactories& pddl_factories)
{
    if (const auto condition_forall = std::get_if<ConditionForallImpl>(condition.get_condition()))
    {
        const auto nested_condition = std::get_if<ConditionForallImpl>(flatten_universal_quantifier(*condition_forall, pddl_factories));
        auto parameters = condition.get_parameters();
        const auto additional_parameters = nested_condition->get_parameters();
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
        return pddl_factories.conditions.template get_or_create<ConditionForallImpl>(parameters, nested_condition->get_condition());
    }
    return pddl_factories.conditions.template get_or_create<ConditionForallImpl>(condition.get_parameters(), condition.get_condition());
}

void collect_free_variables_recursively(const ConditionImpl& condition, VariableSet& ref_quantified_variables, VariableSet& ref_free_variables)
{
    if (const auto condition_imply = std::get_if<ConditionImplyImpl>(&condition))
    {
        collect_free_variables_recursively(*condition_imply->get_condition_left(), ref_quantified_variables, ref_free_variables);
        collect_free_variables_recursively(*condition_imply->get_condition_right(), ref_quantified_variables, ref_free_variables);
    }
    else if (const auto condition_and = std::get_if<ConditionAndImpl>(&condition))
    {
        for (const auto& part : condition_and->get_conditions())
        {
            collect_free_variables_recursively(*part, ref_quantified_variables, ref_free_variables);
        }
    }
    // TODO
}

VariableList collect_free_variables(const ConditionImpl& condition)
{
    auto quantified_variables = VariableSet {};
    auto free_variables = VariableSet {};

    collect_free_variables_recursively(condition, quantified_variables, free_variables);

    return VariableList(free_variables.begin(), free_variables.end());
}
}