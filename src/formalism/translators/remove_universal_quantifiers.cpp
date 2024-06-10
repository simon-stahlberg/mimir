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
RemoveUniversalQuantifiersTranslator::Scope::Scope(std::unordered_map<loki::Variable, loki::Parameter> variable_to_parameter, const Scope* parent_scope) :
    m_variable_to_parameter(std::move(variable_to_parameter)),
    m_parent_scope(parent_scope)
{
}

std::optional<loki::Parameter> RemoveUniversalQuantifiersTranslator::Scope::get_parameter(const loki::Variable& variable) const
{
    auto it = m_variable_to_parameter.find(variable);
    if (it != m_variable_to_parameter.end())
    {
        return it->second;
    }
    if (m_parent_scope)
    {
        return m_parent_scope->get_parameter(variable);
    }
    return std::nullopt;
}

const RemoveUniversalQuantifiersTranslator::Scope& RemoveUniversalQuantifiersTranslator::ScopeStack::open_scope(const loki::ParameterList& parameters)
{
    auto variable_to_parameter = std::unordered_map<loki::Variable, loki::Parameter> {};
    for (const auto& parameter : parameters)
    {
        variable_to_parameter.emplace(parameter->get_variable(), parameter);
    }
    m_stack.empty() ? m_stack.push_back(std::make_unique<Scope>(std::move(variable_to_parameter))) :
                      m_stack.push_back(std::make_unique<Scope>(std::move(variable_to_parameter), &get()));
    return get();
}

void RemoveUniversalQuantifiersTranslator::ScopeStack::close_scope()
{
    assert(!m_stack.empty());
    m_stack.pop_back();
}

const RemoveUniversalQuantifiersTranslator::Scope& RemoveUniversalQuantifiersTranslator::ScopeStack::get() const { return *m_stack.back(); }

void RemoveUniversalQuantifiersTranslator::prepare_impl(const loki::PredicateImpl& predicate)
{
    m_simple_and_derived_predicate_names.insert(predicate.get_name());
}

loki::Condition RemoveUniversalQuantifiersTranslator::translate_impl(const loki::ConditionExistsImpl& condition)
{
    m_scopes.open_scope(condition.get_parameters());

    auto result =
        this->m_pddl_factories.get_or_create_condition_exists(this->translate(condition.get_parameters()), this->translate(*condition.get_condition()));

    m_scopes.close_scope();
    return result;
}

loki::Condition RemoveUniversalQuantifiersTranslator::translate_impl(const loki::ConditionForallImpl& condition)
{
    // Access already computed axioms
    auto it = m_condition_to_substituted_condition.find(&condition);
    if (it != m_condition_to_substituted_condition.end())
    {
        return it->second;
    }

    const auto& scope = m_scopes.open_scope(condition.get_parameters());

    // Free(exists(vars, phi)) become parameters. We obtain their types from the parameters in the parent scope.
    auto head_parameters = loki::ParameterList {};
    auto terms = loki::TermList {};
    for (const auto free_variable :
         collect_free_variables(*this->m_pddl_factories.get_or_create_condition_forall(condition.get_parameters(), condition.get_condition())))
    {
        const auto optional_parameter = scope.get_parameter(free_variable);
        assert(optional_parameter.has_value());
        const auto parameter = this->m_pddl_factories.get_or_create_parameter(free_variable, optional_parameter.value()->get_bases());
        head_parameters.push_back(parameter);
        terms.push_back(this->m_pddl_factories.get_or_create_term_variable(free_variable));
    }
    // Important: all other parameters are appended to the axiom parameters
    auto axiom_parameters = head_parameters;
    for (const auto& parameter : condition.get_parameters())
    {
        axiom_parameters.push_back(parameter);
    }
    head_parameters.shrink_to_fit();
    axiom_parameters.shrink_to_fit();

    const auto axiom_condition = this->translate(*this->m_pddl_factories.get_or_create_condition_exists(
        axiom_parameters,
        m_to_nnf_translator.translate(*this->m_pddl_factories.get_or_create_condition_not(condition.get_condition()))));

    const auto axiom_name = create_unique_axiom_name(this->m_next_axiom_id, this->m_simple_and_derived_predicate_names);
    const auto predicate = this->m_pddl_factories.get_or_create_predicate(axiom_name, head_parameters);
    m_derived_predicates.insert(predicate);
    const auto atom = this->m_pddl_factories.get_or_create_atom(predicate, terms);
    const auto substituted_condition = this->m_pddl_factories.get_or_create_condition_literal(this->m_pddl_factories.get_or_create_literal(true, atom));
    const auto axiom = this->m_pddl_factories.get_or_create_axiom(axiom_name, axiom_parameters, axiom_condition, head_parameters.size());

    m_axioms.insert(axiom);

    m_condition_to_substituted_condition.emplace(&condition, substituted_condition);

    m_scopes.close_scope();

    return substituted_condition;
}

loki::Action RemoveUniversalQuantifiersTranslator::translate_impl(const loki::ActionImpl& action)
{
    this->m_scopes.open_scope(action.get_parameters());

    // Translate condition and effect
    auto translated_condition =
        (action.get_condition().has_value() ? std::optional<loki::Condition>(this->translate(*action.get_condition().value())) : std::nullopt);
    auto translated_effect = (action.get_effect().has_value() ? std::optional<loki::Effect>(this->translate(*action.get_effect().value())) : std::nullopt);

    // Turn free variables into parameters
    auto translated_parameters = this->translate(action.get_parameters());

    auto translated_action = this->m_pddl_factories.get_or_create_action(action.get_name(),
                                                                         action.get_original_arity(),
                                                                         translated_parameters,
                                                                         translated_condition,
                                                                         translated_effect);

    this->m_scopes.close_scope();

    return translated_action;
}

loki::Axiom RemoveUniversalQuantifiersTranslator::translate_impl(const loki::AxiomImpl& axiom)
{
    this->m_scopes.open_scope(axiom.get_parameters());

    // Translate condition and literal
    auto translated_condition = this->translate(*axiom.get_condition());

    // Turn free variables into parameters
    auto translated_parameters = this->translate(axiom.get_parameters());

    auto translated_axiom = this->m_pddl_factories.get_or_create_axiom(axiom.get_derived_predicate_name(),
                                                                       translated_parameters,
                                                                       translated_condition,
                                                                       axiom.get_num_parameters_to_ground_head());

    this->m_scopes.close_scope();

    return translated_axiom;
}

loki::Domain RemoveUniversalQuantifiersTranslator::translate_impl(const loki::DomainImpl& domain)
{
    // Clear containers that store derived predicates and axioms obtained during translation.
    m_axioms.clear();
    m_derived_predicates.clear();

    // Translate existing axioms.
    auto translated_axioms = this->translate(domain.get_axioms());

    // Translate universal quantifiers in conditions to axioms.
    auto translated_actions = this->translate(domain.get_actions());

    // Combine all axioms.
    translated_axioms.insert(translated_axioms.end(), m_axioms.begin(), m_axioms.end());
    translated_axioms = uniquify_elements(translated_axioms);

    auto translated_predicates = this->translate(domain.get_predicates());
    translated_predicates.insert(translated_predicates.end(), m_derived_predicates.begin(), m_derived_predicates.end());

    return this->m_pddl_factories.get_or_create_domain(domain.get_name(),
                                                       this->translate(*domain.get_requirements()),
                                                       this->translate(domain.get_types()),
                                                       this->translate(domain.get_constants()),
                                                       translated_predicates,
                                                       this->translate(domain.get_functions()),
                                                       translated_actions,
                                                       translated_axioms);
}

loki::Problem RemoveUniversalQuantifiersTranslator::translate_impl(const loki::ProblemImpl& problem)
{
    // Translate the domain
    auto translated_domain = this->translate(*problem.get_domain());

    // Clear containers that store derived predicates and axioms obtained during translation.
    m_axioms.clear();
    m_derived_predicates.clear();

    // Translate existing derived predicates and axioms.
    auto translated_derived_predicates = this->translate(problem.get_derived_predicates());
    auto translated_axioms = this->translate(problem.get_axioms());

    // Translate the goal condition
    auto translated_goal =
        (problem.get_goal_condition().has_value() ? std::optional<loki::Condition>(this->translate(*problem.get_goal_condition().value())) : std::nullopt);

    // Combine all derived predicates and axioms.
    translated_derived_predicates.insert(translated_derived_predicates.end(), m_derived_predicates.begin(), m_derived_predicates.end());
    translated_derived_predicates = uniquify_elements(translated_derived_predicates);
    translated_axioms.insert(translated_axioms.end(), m_axioms.begin(), m_axioms.end());
    translated_axioms = uniquify_elements(translated_axioms);

    return this->m_pddl_factories.get_or_create_problem(
        translated_domain,
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

loki::Problem RemoveUniversalQuantifiersTranslator::run_impl(const loki::ProblemImpl& problem)
{
    this->prepare(problem);
    return this->translate(*this->m_to_nnf_translator.run(problem));
}

RemoveUniversalQuantifiersTranslator::RemoveUniversalQuantifiersTranslator(loki::PDDLFactories& pddl_factories, ToNNFTranslator& to_nnf_translator) :
    BaseCachedRecurseTranslator(pddl_factories),
    m_to_nnf_translator(to_nnf_translator),
    m_next_axiom_id(0)
{
}
}