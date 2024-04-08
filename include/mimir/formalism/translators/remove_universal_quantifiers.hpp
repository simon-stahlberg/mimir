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
class RemoveUniversalQuantifiersTranslator : public BaseTranslator<RemoveUniversalQuantifiersTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<RemoveUniversalQuantifiersTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    ToNNFTranslator& m_to_nnf_translator;
    ScopeStack m_scopes;

    DerivedPredicateSet m_axioms;
    std::unordered_map<const ConditionForallImpl*, Condition> m_condition_to_substituted_condition;

    /// @brief Translate the condition while keeping track of scopes
    Condition translate_impl(const ConditionExistsImpl& condition);
    Condition translate_impl(const ConditionForallImpl& condition);
    Action translate_impl(const ActionImpl& action);

    Problem run_impl(const ProblemImpl& problem);

public:
    RemoveUniversalQuantifiersTranslator(PDDLFactories& pddl_factories, ToNNFTranslator& to_nnf_translator);

    /// @brief Return the generated axioms.
    DerivedPredicateList get_axioms() const;
};
}

#endif