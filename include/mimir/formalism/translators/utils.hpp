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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_TRANSLATORS_UTILS_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_UTILS_HPP_

#include "mimir/formalism/declarations.hpp"

#include <random>

namespace mimir
{

/**
 * Conditions
 */

/**
 * Flatten conjunctions.
 *
 * A and (B and C)  =>  A and B and C
 */
extern loki::Condition flatten(const loki::ConditionAndImpl& condition, loki::PDDLRepositories& pddl_repositories);

/**
 * Flatten conjunctions.
 *
 * e1 and (e2 and e3)  =>  e1 and e2 and e3
 */
extern loki::Effect flatten(const loki::EffectAndImpl& effect, loki::PDDLRepositories& pddl_repositories);

/**
 * Flatten disjunctions.
 *
 * A or (B or C)  =>  A or B or C
 */
extern loki::Condition flatten(const loki::ConditionOrImpl& condition, loki::PDDLRepositories& pddl_repositories);

/**
 * Flatten existential quantifiers.
 *
 * exists(vars1, exists(vars2, A))  =>  exists(vars1+vars2, A)
 */
extern loki::Condition flatten(const loki::ConditionExistsImpl& condition, loki::PDDLRepositories& pddl_repositories);

/**
 * Flatten universal quantifiers.
 *
 * forall(vars1, forall(vars2, A))  =>  forall(vars1+vars2, A)
 */
extern loki::Condition flatten(const loki::ConditionForallImpl& condition, loki::PDDLRepositories& pddl_repositories);

/**
 * Flatten conditional when
 *
 * A > (B > e)    =>  (A and B) > e
 */
extern loki::Effect flatten(const loki::EffectCompositeWhenImpl& effect, loki::PDDLRepositories& pddl_repositories);

/**
 * Flatten conditional forall
 *
 * forall(vars1, forall(vars2, e))    =>  forall(vars1+vars2, e)
 */
extern loki::Effect flatten(const loki::EffectCompositeForallImpl& effect, loki::PDDLRepositories& pddl_repositories);

/**
 * Axioms
 */

extern std::string create_unique_axiom_name(Index& next_axiom_index, std::unordered_set<std::string>& simple_and_derived_predicate_names);

}

#endif
