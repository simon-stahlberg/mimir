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

#include "mimir/formalism/translators/rename_quantified_variables.hpp"

using namespace std::string_literals;

namespace mimir
{

static loki::Variable create_renamed_variable(const loki::Variable& variable, size_t num_quantification, loki::PDDLFactories& pddl_factories)
{
    return pddl_factories.get_or_create_variable(variable->get_name() + "_" + std::to_string(variable->get_identifier()) + "_"
                                                 + std::to_string(num_quantification));
}

void RenameQuantifiedVariablesTranslator::rename_variables(const loki::ParameterList& parameters)
{
    for (const auto& parameter : parameters)
    {
        // Increment number of quantifications of the variable.
        const auto renamed_variable =
            create_renamed_variable(parameter->get_variable(), ++m_num_quantifications.at(parameter->get_variable()), this->m_pddl_factories);

        m_renamings[parameter->get_variable()] = renamed_variable;
    }
}

void RenameQuantifiedVariablesTranslator::prepare_impl(const loki::VariableImpl& variable) { m_variables.insert(&variable); }

loki::Variable RenameQuantifiedVariablesTranslator::translate_impl(const loki::VariableImpl& variable) { return m_renamings.at(&variable); }

loki::Problem RenameQuantifiedVariablesTranslator::run_impl(const loki::ProblemImpl& problem)
{
    this->prepare(problem);

    for (const auto& variable : m_variables)
    {
        m_num_quantifications.emplace(variable, 0);

        const auto renamed_variable = create_renamed_variable(variable, 0, this->m_pddl_factories);

        m_renamings.emplace(variable, renamed_variable);
    }

    return this->translate(problem);
}

RenameQuantifiedVariablesTranslator::RenameQuantifiedVariablesTranslator(loki::PDDLFactories& pddl_factories) : BaseRecurseTranslator(pddl_factories) {}

}
