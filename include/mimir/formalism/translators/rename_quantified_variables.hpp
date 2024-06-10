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

#include <deque>

namespace mimir
{

/**
 * Rename all variables to be able to move quantifiers around.
 */
class RenameQuantifiedVariablesTranslator : public BaseRecurseTranslator<RenameQuantifiedVariablesTranslator>
{
private:
    /* Implement BaseRecurseTranslator interface. */
    friend class BaseRecurseTranslator<RenameQuantifiedVariablesTranslator>;

    // Provide default implementations
    using BaseRecurseTranslator::prepare_impl;
    using BaseRecurseTranslator::translate_impl;

    /* Prepare step */

    // Collect all variables in preparation phase.
    std::unordered_set<loki::Variable> m_variables;

    class PrepareScope
    {
    private:
        PrepareScope* m_parent;

        std::unordered_set<loki::Variable> m_quantified_in_scope_or_child_scopes;

        bool m_name_conflict_detected;

    public:
        explicit PrepareScope(PrepareScope* parent = nullptr);

        /// @brief Insert a variable.
        /// Set m_name_conflict_detected to true if quantified variable already exists.
        /// The meaning is that the variable is quantified over in one of the sibling scopes.
        void insert(const loki::Variable& variable);
        void on_conflict_detected();

        PrepareScope* get_parent();
        const std::unordered_set<loki::Variable>& get_quantified_in_scope_or_child_scopes() const;
        bool get_name_conflict_detected() const;
    };

    class PrepareScopeStack
    {
    private:
        std::deque<std::unique_ptr<PrepareScope>> m_scopes;

    public:
        const PrepareScope& open_scope(const loki::ParameterList& parameters = loki::ParameterList {});

        /// @brief Insert all variables in the parent scope, and
        /// propagate a potential conflict to the parent scope.
        void close_scope();

        /// @brief Only propagate a detected conflict.
        /// We use this when jumping to the next action or axiom since their variables cannot conflict.
        void close_scope_soft();

        const PrepareScope& top() const;
    };

    PrepareScopeStack m_scopes;

    /// @brief Collect all existing variables.
    void prepare_impl(const loki::VariableImpl& variable);
    // Open scopes for each of the parameter list to begin tracking for conflicts
    void prepare_impl(const loki::ConditionExistsImpl& condition);
    void prepare_impl(const loki::ConditionForallImpl& condition);
    void prepare_impl(const loki::EffectConditionalForallImpl& effect);
    void prepare_impl(const loki::ActionImpl& action);
    void prepare_impl(const loki::AxiomImpl& axiom);

    /* Translate step */

    std::unordered_map<loki::Variable, size_t> m_num_quantifications;

    bool m_renaming_enabled;

    void increment_num_quantifications(const loki::ParameterList& parameters);
    void decrement_num_quantifications(const loki::ParameterList& parameters);

    loki::Variable translate_impl(const loki::VariableImpl& variable);

    /**
     * Keep variable names of parameters in these constructs, i.e., turn off renaming
     */
    loki::Predicate translate_impl(const loki::PredicateImpl& predicate);
    loki::FunctionSkeleton translate_impl(const loki::FunctionSkeletonImpl& function_skeleton);

    /**
     * Rename variables names of parameters in these constructs, i.e., turn on renaming
     */
    loki::Condition translate_impl(const loki::ConditionExistsImpl& condition);
    loki::Condition translate_impl(const loki::ConditionForallImpl& condition);
    loki::Effect translate_impl(const loki::EffectConditionalForallImpl& effect);
    loki::Action translate_impl(const loki::ActionImpl& action);
    loki::Axiom translate_impl(const loki::AxiomImpl& axiom);

    loki::Problem run_impl(const loki::ProblemImpl& problem);

public:
    explicit RenameQuantifiedVariablesTranslator(loki::PDDLFactories& pddl_factories);
};
}

#endif