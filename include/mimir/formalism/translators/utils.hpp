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

namespace mimir
{

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

}

#endif
