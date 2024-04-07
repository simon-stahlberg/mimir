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
Condition RemoveUniversalQuantifierTranslator::translate_impl(const ConditionExistsImpl& condition)
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

Condition RemoveUniversalQuantifierTranslator::translate_impl(const ConditionForallImpl& condition)
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
        m_nnf_translator.translate(*this->m_pddl_factories.get_or_create_condition_not(condition.get_condition()))));

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

    const auto axiom_name = "@axiom("s + std::visit([](const auto& arg) { return arg.str(); }, *axiom_condition) + ")";
    const auto predicate = this->m_pddl_factories.get_or_create_predicate(axiom_name, parameters);
    const auto atom = this->m_pddl_factories.get_or_create_atom(predicate, terms);
    auto substituted_condition = this->m_pddl_factories.get_or_create_condition_literal(this->m_pddl_factories.get_or_create_literal(true, atom));

    const auto axiom = this->m_pddl_factories.get_or_create_axiom(parameters, atom, axiom_condition);
    m_axioms.insert(axiom);
    m_condition_to_substituted_condition.emplace(&condition, substituted_condition);

    m_scopes.close_scope();

    return substituted_condition;
}

RemoveUniversalQuantifierTranslator::RemoveUniversalQuantifierTranslator(PDDLFactories& pddl_factories) :
    BaseTranslator(pddl_factories),
    m_nnf_translator(ToNNFTranslator(pddl_factories))
{
}
}
