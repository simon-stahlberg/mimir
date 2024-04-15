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

#ifndef MIMIR_FORMALISM_TRANSLATORS_REMOVE_UNIVERSAL_QUANTIFIERS_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_REMOVE_UNIVERSAL_QUANTIFIERS_HPP_

#include "mimir/formalism/translators/base.hpp"
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

    class Scope
    {
    private:
        const Scope* m_parent_scope;

        std::unordered_map<loki::Variable, loki::Parameter> m_variable_to_parameter;

    public:
        explicit Scope(const Scope* parent_scope = nullptr) : m_parent_scope(parent_scope) {}

        std::optional<loki::Parameter> get_parameter(const loki::Variable& variable) const
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

        void insert(const loki::Parameter& parameter)
        {
            assert(!m_variable_to_parameter.count(parameter->get_variable()));
            m_variable_to_parameter.emplace(parameter->get_variable(), parameter);
        }
    };

    class ScopeStack
    {
    private:
        std::deque<std::unique_ptr<Scope>> m_stack;

    public:
        Scope& open_scope()
        {
            m_stack.empty() ? m_stack.push_back(std::make_unique<Scope>()) : m_stack.push_back(std::make_unique<Scope>(&get()));
            return get();
        }

        void close_scope()
        {
            assert(!m_stack.empty());
            m_stack.pop_back();
        }

        Scope& get() { return *m_stack.back(); }
    };

    ScopeStack m_scopes;

    // Translation might introduce additional derived predicates and axioms.
    std::unordered_set<loki::Predicate> m_derived_predicates;
    std::unordered_set<loki::Axiom> m_axioms;
    // Cache translations
    std::unordered_map<const loki::ConditionForallImpl*, loki::Condition> m_condition_to_substituted_condition;

    /// @brief Translate the condition while keeping track of scopes
    loki::Condition translate_impl(const loki::ConditionExistsImpl& condition);
    loki::Condition translate_impl(const loki::ConditionForallImpl& condition);
    loki::Action translate_impl(const loki::ActionImpl& action);
    loki::Domain translate_impl(const loki::DomainImpl& domain);
    loki::Problem translate_impl(const loki::ProblemImpl& problem);

    loki::Problem run_impl(const loki::ProblemImpl& problem);

public:
    RemoveUniversalQuantifiersTranslator(loki::PDDLFactories& pddl_factories, ToNNFTranslator& to_nnf_translator);
};
}

#endif