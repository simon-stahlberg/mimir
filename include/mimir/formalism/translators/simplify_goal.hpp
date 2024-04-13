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

#ifndef MIMIR_FORMALISM_TRANSLATORS_SIMPLIFY_GOAL_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_SIMPLIFY_GOAL_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{

class SimplifyGoalTranslator : public BaseTranslator<SimplifyGoalTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<SimplifyGoalTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    // Translation might introduce additional derived predicates and axioms.
    std::unordered_set<loki::Predicate> m_derived_predicates;
    std::unordered_set<loki::Axiom> m_axioms;

    loki::Problem translate_impl(const loki::ProblemImpl& problem);

    loki::Problem run_impl(const loki::ProblemImpl& problem);

public:
    explicit SimplifyGoalTranslator(loki::PDDLFactories& pddl_factories);
};
}

#endif