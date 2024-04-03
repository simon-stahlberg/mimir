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

#include "mimir/formalism/translators/universal_quantifiers.hpp"

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

Condition UniversalQuantifierTranslator::translate_impl(const ConditionForallImpl& condition)
{
    // Access already computed axioms
    auto it = m_condition_to_axiom.find(&condition);
    if (it != m_condition_to_axiom.end())
    {
        return m_axiom_to_literal.at(it->second);
    }

    // const auto axiom_condition = m_nnf_translator.translate(*this->m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(&condition));
    //  parameters are free variables
}

UniversalQuantifierTranslator::UniversalQuantifierTranslator(PDDLFactories& pddl_factories) :
    BaseTranslator(pddl_factories),
    m_nnf_translator(NNFTranslator(pddl_factories))
{
}

}
