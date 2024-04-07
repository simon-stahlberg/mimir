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
extern Predicate type_to_predicate(const TypeImpl& type, PDDLFactories& pddl_factories);

/// @brief Translate an typed object to an untyped objects.
extern Object typed_object_to_untyped_object(const ObjectImpl& object, PDDLFactories& pddl_factories);

/// @brief Translate an typed object to a list of ground literal for all its original types.
extern GroundLiteralList typed_object_to_ground_literals(const ObjectImpl& object, PDDLFactories& pddl_factories);

/// @brief Translate a typed parameter to an untyped parameter.
extern Parameter typed_parameter_to_untyped_parameter(const ParameterImpl& parameter, PDDLFactories& pddl_factories);

/// @brief Translate an typed parameter to a list of condition literal for all its original types.
extern ConditionList typed_parameter_to_condition_literals(const ParameterImpl& parameter, PDDLFactories& pddl_factories);

/// @brief Compute all types from a hierarchy of types.
extern TypeList collect_types_from_type_hierarchy(const TypeList& type_list);

/**
 * Conditions
 */

/**
 * Flatten conjunctions.
 *
 * 9. A and (B and C)  =>  A and B and C
 */
extern Condition flatten_conjunctions(const ConditionAndImpl& condition, PDDLFactories& pddl_factories);

/**
 * Flatten disjunctions.
 *
 * 10. A or (B or C)  =>  A or B or C
 */
extern Condition flatten_disjunctions(const ConditionOrImpl& condition, PDDLFactories& pddl_factories);

/**
 * Flatten existential quantifiers.
 *
 * 11. exists(vars1, exists(vars2, A))  =>  exists(vars1+vars2, A)
 */
extern Condition flatten_existential_quantifier(const ConditionExistsImpl& condition, PDDLFactories& pddl_factories);

/**
 * Flatten universal quantifiers.
 *
 * 12. forall(vars1, forall(vars2, A))  =>  forall(vars1+vars2, A)
 */
extern Condition flatten_universal_quantifier(const ConditionForallImpl& condition, PDDLFactories& pddl_factories);

/**
 * Return all free variables of a condition, i.e., variables that are not quantified.
 */
extern VariableList collect_free_variables(const ConditionImpl& condition);

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
