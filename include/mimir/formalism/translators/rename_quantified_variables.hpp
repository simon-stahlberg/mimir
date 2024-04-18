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

#ifndef MIMIR_FORMALISM_TRANSLATORS_RENAME_QUANTIFIED_VARIABLES_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_RENAME_QUANTIFIED_VARIABLES_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{

/**
 * Rename all variables by adding suffixes _<i>_<j>
 * where i is the variable identifier and j is the number
 * of occurences of the variable in a quantification during
 * depth-first traversal.
 */
class RenameQuantifiedVariablesTranslator : public BaseTranslator<RenameQuantifiedVariablesTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<RenameQuantifiedVariablesTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    // Collect all variables in preparation phase.
    std::unordered_set<loki::Variable> m_variables;
    // Track the number of times that each variable was quantified during the translation phase.
    // Increment num_quantifications[var] when encountering a quantifier during the translation phase.
    std::unordered_map<loki::Variable, size_t> m_num_quantifications;

    class Scope
    {
    private:
        std::unordered_map<loki::Variable, loki::Variable> m_renaming;

        const Scope* m_parent_scope;

    public:
        Scope(std::unordered_map<loki::Variable, loki::Variable> renaming, const Scope* parent_scope = nullptr);

        const std::unordered_map<loki::Variable, loki::Variable>& get_renaming() const;
    };

    class ScopeStack
    {
    private:
        std::deque<std::unique_ptr<Scope>> m_stack;

    public:
        /**
         * Open the first scope
         */
        const Scope&
        open_scope(const loki::VariableList& variables, std::unordered_map<loki::Variable, size_t>& num_quantifications, loki::PDDLFactories& pddl_factories);

        /**
         * Open successive scope.
         */
        const Scope&
        open_scope(const loki::ParameterList& parameters, std::unordered_map<loki::Variable, size_t>& num_quantifications, loki::PDDLFactories& pddl_factories);

        void close_scope();

        const Scope& get() const;
    };

    ScopeStack m_scopes;

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