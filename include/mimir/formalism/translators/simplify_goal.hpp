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

#ifndef MIMIR_FORMALISM_TRANSLATORS_SIMPLIFY_GOAL_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_SIMPLIFY_GOAL_HPP_

#include "mimir/formalism/translators/base_cached_recurse.hpp"

namespace mimir
{

/**
 * Introduce an axiom for complicated goals, i.e., goals that are not conjunctions of literals.
 */
class SimplifyGoalTranslator : public BaseCachedRecurseTranslator<SimplifyGoalTranslator>
{
private:
    /* Implement BaseCachedRecurseTranslator interface. */
    friend class BaseCachedRecurseTranslator<SimplifyGoalTranslator>;

    // Provide default implementations
    using BaseCachedRecurseTranslator::prepare_impl;
    using BaseCachedRecurseTranslator::translate_impl;

    // Track simple and derived predicate names to give unique names
    std::unordered_set<std::string> m_simple_and_derived_predicate_names;
    uint64_t m_next_axiom_id;

    // Translation might introduce additional derived predicates and axioms.
    std::unordered_set<loki::Predicate> m_derived_predicates;
    std::unordered_set<loki::Axiom> m_axioms;

    /// @brief Collect all existing simple and derived predicate names.
    void prepare_impl(const loki::PredicateImpl& predicate);

    loki::Problem translate_impl(const loki::ProblemImpl& problem);

public:
    explicit SimplifyGoalTranslator(loki::PDDLFactories& pddl_factories);
};
}

#endif