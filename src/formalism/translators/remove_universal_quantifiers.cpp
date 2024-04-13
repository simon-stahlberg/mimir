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

#include "mimir/formalism/translators/remove_universal_quantifiers.hpp"

#include "mimir/formalism/translators/utils.hpp"

using namespace std::string_literals;

namespace mimir
{
Condition RemoveUniversalQuantifiersTranslator::translate_impl(const ConditionExistsImpl& condition)
{
    auto scope = m_scopes.open_scope();

    for (const auto& parameter : condition.get_parameters())
    {
        scope.insert(parameter);
    }
    auto result = this->translate(*condition.get_condition());

    m_scopes.close_scope();
    return result;
}

Condition RemoveUniversalQuantifiersTranslator::translate_impl(const ConditionForallImpl& condition)
{
    // Access already computed axioms
    auto it = m_condition_to_substituted_condition.find(&condition);
    if (it != m_condition_to_substituted_condition.end())
    {
        return it->second;
    }

    auto scope = m_scopes.open_scope();

    for (const auto& parameter : condition.get_parameters())
    {
        scope.insert(parameter);
    }

    // Note: axiom_condition may contain conjunctions or disjunctions
    const auto axiom_condition = this->translate(*this->m_pddl_factories.get_or_create_condition_exists(
        condition.get_parameters(),
        m_to_nnf_translator.translate(*this->m_pddl_factories.get_or_create_condition_not(condition.get_condition()))));

    // Free(exists(vars, phi)) become parameters. We obtain their types from the parameters in the parent scope.
    auto parameters = ParameterList {};
    auto terms = TermList {};
    for (const auto free_variable : collect_free_variables(*axiom_condition))
    {
        const auto optional_parameter = scope.get_parameter(free_variable);
        assert(optional_parameter.has_value());
        const auto parameter = optional_parameter.value();

        parameters.push_back(this->m_pddl_factories.get_or_create_parameter(free_variable, parameter->get_bases()));
        terms.push_back(this->m_pddl_factories.get_or_create_term_variable(free_variable));
    }
    parameters.shrink_to_fit();

    const auto axiom_name = "@axiom["s + std::visit([](const auto& arg) { return arg.str(); }, *axiom_condition) + "]";
    const auto predicate = this->m_pddl_factories.get_or_create_predicate(axiom_name, parameters);
    m_derived_predicates.insert(predicate);
    const auto literal = this->m_pddl_factories.get_or_create_literal(true, this->m_pddl_factories.get_or_create_atom(predicate, terms));
    const auto substituted_condition = this->m_pddl_factories.get_or_create_condition_literal(
        this->m_pddl_factories.get_or_create_literal(true, this->m_pddl_factories.get_or_create_atom(predicate, terms)));
    const auto axiom = this->m_pddl_factories.get_or_create_axiom(literal, axiom_condition);
    m_axioms.insert(axiom);

    m_condition_to_substituted_condition.emplace(&condition, substituted_condition);

    m_scopes.close_scope();

    return substituted_condition;
}

Action RemoveUniversalQuantifiersTranslator::translate_impl(const ActionImpl& action)
{
    return this->cached_translated_impl(
        action,
        m_translated_actions,
        [this](const ActionImpl& arg)
        {
            auto& scope = this->m_scopes.open_scope();

            for (const auto& parameter : arg.get_parameters())
            {
                scope.insert(parameter);
            }

            auto translated_action = this->m_pddl_factories.get_or_create_action(
                arg.get_name(),
                arg.get_parameters(),
                (arg.get_condition().has_value() ? std::optional<Condition>(this->translate(*arg.get_condition().value())) : std::nullopt),
                arg.get_effect());

            this->m_scopes.close_scope();

            return translated_action;
        });
}

Problem RemoveUniversalQuantifiersTranslator::run_impl(const ProblemImpl& problem)
{
    m_axioms.clear();
    // Translate domain, create new domain with axioms
    auto translated_domain = this->translate(*problem.get_domain());
    auto translated_derived_predicates = this->translate(problem.get_domain()->get_derived_predicates());
    auto translated_axioms = this->translate(problem.get_domain()->get_axioms());
    translated_derived_predicates.insert(translated_derived_predicates.end(), m_derived_predicates.begin(), m_derived_predicates.end());
    translated_derived_predicates = uniquify_elements(translated_derived_predicates);
    translated_axioms.insert(translated_axioms.end(), m_axioms.begin(), m_axioms.end());
    translated_axioms = uniquify_elements(translated_axioms);
    translated_domain = m_pddl_factories.get_or_create_domain(translated_domain->get_name(),
                                                              translated_domain->get_requirements(),
                                                              translated_domain->get_types(),
                                                              translated_domain->get_constants(),
                                                              translated_domain->get_predicates(),
                                                              translated_derived_predicates,
                                                              translated_domain->get_functions(),
                                                              translated_domain->get_actions(),
                                                              translated_axioms);
    // Translate problem, create new problem with all axioms
    auto translated_problem = this->translate(problem);
    translated_derived_predicates.insert(translated_derived_predicates.end(), m_derived_predicates.begin(), m_derived_predicates.end());
    translated_derived_predicates = uniquify_elements(translated_derived_predicates);
    translated_axioms.insert(translated_axioms.end(), m_axioms.begin(), m_axioms.end());
    translated_axioms = uniquify_elements(translated_axioms);
    translated_problem = m_pddl_factories.get_or_create_problem(translated_domain,
                                                                translated_problem->get_name(),
                                                                translated_problem->get_requirements(),
                                                                translated_problem->get_objects(),
                                                                translated_derived_predicates,
                                                                translated_problem->get_initial_literals(),
                                                                translated_problem->get_numeric_fluents(),
                                                                translated_problem->get_goal_condition(),
                                                                translated_problem->get_optimization_metric(),
                                                                translated_axioms);
    return translated_problem;
}

RemoveUniversalQuantifiersTranslator::RemoveUniversalQuantifiersTranslator(PDDLFactories& pddl_factories, ToNNFTranslator& to_nnf_translator) :
    BaseTranslator(pddl_factories),
    m_to_nnf_translator(to_nnf_translator)
{
}
}