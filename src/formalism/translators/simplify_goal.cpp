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

#include "mimir/formalism/translators/simplify_goal.hpp"

#include "mimir/formalism/translators/utils.hpp"

using namespace std::string_literals;

namespace mimir
{

static loki::Condition simplify_goal_condition(const loki::ConditionImpl& goal_condition,
                                               loki::PDDLFactories& pddl_factories,
                                               std::unordered_set<loki::Predicate>& derived_predicates,
                                               std::unordered_set<loki::Axiom>& axioms,
                                               uint64_t& next_axiom_id,
                                               std::unordered_set<std::string>& simple_and_derived_predicates)
{
    if (std::get_if<loki::ConditionLiteralImpl>(&goal_condition))
    {
        return &goal_condition;
    }
    else if (const auto condition_and = std::get_if<loki::ConditionAndImpl>(&goal_condition))
    {
        auto parts = loki::ConditionList {};
        parts.reserve(condition_and->get_conditions().size());
        for (const auto& part : condition_and->get_conditions())
        {
            parts.push_back(simplify_goal_condition(*part, pddl_factories, derived_predicates, axioms, next_axiom_id, simple_and_derived_predicates));
        }
        return pddl_factories.get_or_create_condition_and(parts);
    }

    const auto axiom_name = create_unique_axiom_name(next_axiom_id, simple_and_derived_predicates);
    const auto predicate = pddl_factories.get_or_create_predicate(axiom_name, loki::ParameterList {});
    derived_predicates.insert(predicate);
    const auto atom = pddl_factories.get_or_create_atom(predicate, loki::TermList {});
    const auto literal = pddl_factories.get_or_create_literal(false, atom);
    const auto substituted_condition = pddl_factories.get_or_create_condition_literal(literal);
    const auto axiom = pddl_factories.get_or_create_axiom(axiom_name, loki::ParameterList {}, &goal_condition, 0);
    axioms.insert(axiom);

    return substituted_condition;
}

void SimplifyGoalTranslator::prepare_impl(const loki::PredicateImpl& predicate) { m_simple_and_derived_predicate_names.insert(predicate.get_name()); }

loki::Problem SimplifyGoalTranslator::translate_impl(const loki::ProblemImpl& problem)
{
    // Translate existing derived predicates and axioms.
    auto translated_derived_predicates = this->translate(problem.get_derived_predicates());
    auto translated_axioms = this->translate(problem.get_axioms());

    // Translate the goal condition which might introduce additional derived predicates and axioms.
    auto derived_predicates = std::unordered_set<loki::Predicate> {};
    auto axioms = std::unordered_set<loki::Axiom> {};
    auto next_axiom_id = uint64_t { 0 };
    auto translated_goal =
        (problem.get_goal_condition().has_value() ? std::optional<loki::Condition>(simplify_goal_condition(*problem.get_goal_condition().value(),
                                                                                                           this->m_pddl_factories,
                                                                                                           derived_predicates,
                                                                                                           axioms,
                                                                                                           next_axiom_id,
                                                                                                           this->m_simple_and_derived_predicate_names)) :
                                                    std::nullopt);

    // Combine all derived derived predicates and axioms.
    translated_derived_predicates.insert(translated_derived_predicates.end(), derived_predicates.begin(), derived_predicates.end());
    translated_derived_predicates = uniquify_elements(translated_derived_predicates);
    translated_axioms.insert(translated_axioms.end(), axioms.begin(), axioms.end());
    translated_axioms = uniquify_elements(translated_axioms);

    return this->m_pddl_factories.get_or_create_problem(
        this->translate(*problem.get_domain()),
        problem.get_name(),
        this->translate(*problem.get_requirements()),
        this->translate(problem.get_objects()),
        translated_derived_predicates,
        this->translate(problem.get_initial_literals()),
        this->translate(problem.get_numeric_fluents()),
        translated_goal,
        (problem.get_optimization_metric().has_value() ? std::optional<loki::OptimizationMetric>(this->translate(*problem.get_optimization_metric().value())) :
                                                         std::nullopt),
        translated_axioms);
}

SimplifyGoalTranslator::SimplifyGoalTranslator(loki::PDDLFactories& pddl_factories) :
    BaseCachedRecurseTranslator<SimplifyGoalTranslator>(pddl_factories),
    m_simple_and_derived_predicate_names()
{
}

}
