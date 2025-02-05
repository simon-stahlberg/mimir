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

#include "mimir/formalism/translators/initialize_equality.hpp"

#include "mimir/common/collections.hpp"

namespace mimir
{
static loki::Predicate get_or_create_equality_predicate(loki::PDDLRepositories& pddl_repositories)
{
    const auto object_type = pddl_repositories.get_or_create_type("object", loki::TypeList {});

    // Attention: this must match the way it is instantiated in Loki or otherwise, we may not fetch an existing = predicate, which loki currently instantiates.
    return pddl_repositories.get_or_create_predicate(
        "=",
        loki::ParameterList {
            pddl_repositories.get_or_create_parameter(pddl_repositories.get_or_create_variable("?left_arg"), loki::TypeList { object_type }),
            pddl_repositories.get_or_create_parameter(pddl_repositories.get_or_create_variable("?right_arg"), loki::TypeList { object_type }) });
}

static loki::Literal get_or_create_equality_literal(loki::Predicate equality_predicate, loki::Object object, loki::PDDLRepositories& pddl_repositories)
{
    const auto term_object = pddl_repositories.get_or_create_term(object);

    return pddl_repositories.get_or_create_literal(false,
                                                   pddl_repositories.get_or_create_atom(equality_predicate, loki::TermList { term_object, term_object }));
}

loki::Domain InitializeEqualityTranslator::translate_impl(loki::Domain domain)
{
    const auto translated_requirements = this->translate(domain->get_requirements());

    auto translated_predicates = this->translate(domain->get_predicates());

    if (translated_requirements->test(loki::RequirementEnum::EQUALITY))
    {
        translated_predicates.push_back(get_or_create_equality_predicate(m_pddl_repositories));
    }

    return this->m_pddl_repositories.get_or_create_domain(domain->get_filepath(),
                                                          domain->get_name(),
                                                          translated_requirements,
                                                          this->translate(domain->get_types()),
                                                          this->translate(domain->get_constants()),
                                                          uniquify_elements(translated_predicates),
                                                          this->translate(domain->get_functions()),
                                                          this->translate(domain->get_actions()),
                                                          this->translate(domain->get_axioms()));
}

loki::Problem InitializeEqualityTranslator::translate_impl(loki::Problem problem)
{
    auto translated_domain = this->translate(problem->get_domain());

    auto translated_objects = this->translate(problem->get_objects());

    auto translated_initial_literals = this->translate(problem->get_initial_literals());

    if (translated_domain->get_requirements()->test(loki::RequirementEnum::EQUALITY))
    {
        auto equality_predicate = get_or_create_equality_predicate(m_pddl_repositories);  ///< just instantiate it once here.
        for (const auto& constant : translated_domain->get_constants())
        {
            translated_initial_literals.push_back(get_or_create_equality_literal(equality_predicate, constant, m_pddl_repositories));
        }
        for (const auto& object : translated_objects)
        {
            translated_initial_literals.push_back(get_or_create_equality_literal(equality_predicate, object, m_pddl_repositories));
        }
    }

    return this->m_pddl_repositories.get_or_create_problem(
        problem->get_filepath(),
        translated_domain,
        problem->get_name(),
        this->translate(problem->get_requirements()),
        translated_objects,
        this->translate(problem->get_derived_predicates()),
        uniquify_elements(translated_initial_literals),
        this->translate(problem->get_function_values()),
        (problem->get_goal_condition().has_value() ? std::optional<loki::Condition>(this->translate(problem->get_goal_condition().value())) : std::nullopt),
        (problem->get_optimization_metric().has_value() ? std::optional<loki::OptimizationMetric>(this->translate(problem->get_optimization_metric().value())) :
                                                          std::nullopt),
        this->translate(problem->get_axioms()));
}

loki::Problem InitializeEqualityTranslator::run_impl(loki::Problem problem) { return this->translate(problem); }

InitializeEqualityTranslator::InitializeEqualityTranslator(loki::PDDLRepositories& pddl_repositories) : BaseCachedRecurseTranslator(pddl_repositories) {}
}