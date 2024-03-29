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
/**
 * Compile away types.
 */
class TypeTranslator : public BaseTranslator<TypeTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<TypeTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    /**
     * Utility functions
     */

    /// @brief Translate a type to a predicate of arity 1.
    Predicate translate_type_to_predicate(const TypeImpl& type);

    /// @brief Translate an typed object to an untyped objects.
    Object translate_typed_object_to_untyped_object(const ObjectImpl& object);

    /// @brief Translate an typed object to a list of ground literal for all its original types.
    GroundLiteralList translate_typed_object_to_ground_literals(const ObjectImpl& object);

    /// @brief Translate a typed parameter to an untyped parameter.
    Parameter translate_typed_parameter_to_untyped_parameter(const ParameterImpl& parameter);

    /// @brief Helper to compute all types from a hierarchy of types.
    void collect_types_from_type_hierarchy_recursively(const Type& type, std::unordered_set<Type>& ref_type_list);

    /// @brief Translate an typed parameter to a list of condition literal for all its original types.
    ConditionList translate_typed_parameter_to_condition_literals(const ParameterImpl& parameter);

    /// @brief Compute all types from a hierarchy of types.
    TypeList collect_types_from_type_hierarchy(const TypeList& type_list);

    /**
     * Translate
     */
    Object translate_impl(const ObjectImpl& object);
    Parameter translate_impl(const ParameterImpl& parameter);
    Condition translate_impl(const ConditionExistsImpl& condition);
    Condition translate_impl(const ConditionForallImpl& condition);
    Effect translate_impl(const EffectConditionalForallImpl& effect);
    Action translate_impl(const ActionImpl& action);
    Domain translate_impl(const DomainImpl& domain);
    Problem translate_impl(const ProblemImpl& problem);

    Problem run_impl(const ProblemImpl& problem);

public:
    explicit TypeTranslator(PDDLFactories& pddl_factories);
};

}
#endif
