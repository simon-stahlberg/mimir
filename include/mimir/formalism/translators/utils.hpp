/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg and Simon Stahlberg
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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_TRANSLATORS_UTILS_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_UTILS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/translators/scopes.hpp"

namespace mimir
{

/**
 * Types
 */

/// @brief Translate a type to a predicate of arity 1.
extern loki::Predicate type_to_predicate(const loki::TypeImpl& type, loki::PDDLFactories& pddl_factories);

/// @brief Translate an typed object to an untyped objects.
extern loki::Object typed_object_to_untyped_object(const loki::ObjectImpl& object, loki::PDDLFactories& pddl_factories);

/// @brief Translate an typed object to a list of ground literal for all its original types.
extern loki::GroundLiteralList typed_object_to_ground_literals(const loki::ObjectImpl& object, loki::PDDLFactories& pddl_factories);

/// @brief Translate a typed parameter to an untyped parameter.
extern loki::Parameter typed_parameter_to_untyped_parameter(const loki::ParameterImpl& parameter, loki::PDDLFactories& pddl_factories);

/// @brief Translate an typed parameter to a list of condition literal for all its original types.
extern loki::ConditionList typed_parameter_to_condition_literals(const loki::ParameterImpl& parameter, loki::PDDLFactories& pddl_factories);

/// @brief Compute all types from a hierarchy of types.
extern loki::TypeList collect_types_from_type_hierarchy(const loki::TypeList& type_list);

/**
 * Conditions
 */

/**
 * Flatten conjunctions.
 *
 * 9. A and (B and C)  =>  A and B and C
 */
extern loki::Condition flatten_conjunctions(const loki::ConditionAndImpl& condition, loki::PDDLFactories& pddl_factories);

extern loki::Effect flatten_conjunctions(const loki::EffectAndImpl& effect, loki::PDDLFactories& pddl_factories);

/**
 * Flatten disjunctions.
 *
 * 10. A or (B or C)  =>  A or B or C
 */
extern loki::Condition flatten_disjunctions(const loki::ConditionOrImpl& condition, loki::PDDLFactories& pddl_factories);

/**
 * Flatten existential quantifiers.
 *
 * 11. exists(vars1, exists(vars2, A))  =>  exists(vars1+vars2, A)
 */
extern loki::Condition flatten_existential_quantifier(const loki::ConditionExistsImpl& condition, loki::PDDLFactories& pddl_factories);

/**
 * Flatten universal quantifiers.
 *
 * 12. forall(vars1, forall(vars2, A))  =>  forall(vars1+vars2, A)
 */
extern loki::Condition flatten_universal_quantifier(const loki::ConditionForallImpl& condition, loki::PDDLFactories& pddl_factories);

/**
 * Return all free variables of a condition, i.e., variables that are not quantified.
 */
extern loki::VariableList collect_free_variables(const loki::ConditionImpl& condition);

/**
 * General
 */

/**
 * Uniquify elements in a vector of elements.
 */
template<typename T>
extern std::vector<const T*> uniquify_elements(const std::vector<const T*>& vec)
{
    std::unordered_set<const T*> set(vec.begin(), vec.end());
    return std::vector<const T*>(set.begin(), set.end());
}
}

#endif
