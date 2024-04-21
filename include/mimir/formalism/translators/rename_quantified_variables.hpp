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

#ifndef MIMIR_FORMALISM_TRANSLATORS_RENAME_QUANTIFIED_VARIABLES_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_RENAME_QUANTIFIED_VARIABLES_HPP_

#include "mimir/formalism/translators/base_recurse.hpp"

namespace mimir
{

/**
 * Rename all variables by adding suffixes _<i>_<j>
 * where i is the variable identifier and j is the number
 * of occurences of the variable in a quantification during
 * depth-first traversal.
 */
class RenameQuantifiedVariablesTranslator : public BaseRecurseTranslator<RenameQuantifiedVariablesTranslator>
{
private:
    /* Implement BaseRecurseTranslator interface. */
    friend class BaseRecurseTranslator<RenameQuantifiedVariablesTranslator>;

    // Provide default implementations
    using BaseRecurseTranslator::prepare_impl;
    using BaseRecurseTranslator::translate_impl;

    // Collect all variables in preparation phase.
    std::unordered_set<loki::Variable> m_variables;
    // Track the number of times that each variable was quantified during the translation phase.
    // Increment num_quantifications[var] when encountering a quantifier during the translation phase.
    std::unordered_map<loki::Variable, size_t> m_num_quantifications;
    std::unordered_map<loki::Variable, loki::Variable> m_renamings;

    void rename_variables(const loki::ParameterList& parameters);

    void prepare_impl(const loki::VariableImpl& variable);

    loki::Variable translate_impl(const loki::VariableImpl& variable);
    loki::Condition translate_impl(const loki::ConditionExistsImpl& condition);
    loki::Condition translate_impl(const loki::ConditionForallImpl& condition);
    loki::Effect translate_impl(const loki::EffectConditionalForallImpl& effect);
    loki::Action translate_impl(const loki::ActionImpl& action);

    loki::Problem run_impl(const loki::ProblemImpl& problem);

public:
    explicit RenameQuantifiedVariablesTranslator(loki::PDDLFactories& pddl_factories);
};
}

#endif