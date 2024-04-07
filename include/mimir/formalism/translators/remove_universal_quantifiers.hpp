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

#ifndef MIMIR_FORMALISM_TRANSLATORS_UNIVERSAL_QUANTIFIERS_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_UNIVERSAL_QUANTIFIERS_HPP_

#include "mimir/formalism/translators/base.hpp"
#include "mimir/formalism/translators/scopes.hpp"
#include "mimir/formalism/translators/to_negation_normal_form.hpp"

#include <deque>

namespace mimir
{

/**
 * Compile away universal quantifiers by introducing axioms.
 */
class RemoveUniversalQuantifierTranslator : public BaseTranslator<RemoveUniversalQuantifierTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<RemoveUniversalQuantifierTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    ToNNFTranslator m_nnf_translator;
    ScopeStack m_scopes;

    AxiomSet m_axioms;
    std::unordered_map<const ConditionForallImpl*, Condition> m_condition_to_substituted_condition;

    /// @brief Translate the condition while keeping track of scopes
    Condition translate_impl(const ConditionExistsImpl& condition);
    Condition translate_impl(const ConditionForallImpl& condition);
    Action translate_impl(const ActionImpl& action);

public:
    explicit RemoveUniversalQuantifierTranslator(PDDLFactories& pddl_factories);
};
}

#endif