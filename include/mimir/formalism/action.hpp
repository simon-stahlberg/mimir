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

#ifndef MIMIR_FORMALISM_ACTION_HPP_
#define MIMIR_FORMALISM_ACTION_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/variable.hpp"

#include <functional>
#include <loki/loki.hpp>
#include <optional>
#include <string>

namespace mimir
{
class ActionImpl : public loki::Base<ActionImpl>
{
private:
    std::string m_name;
    size_t m_original_arity;
    VariableList m_parameters;
    LiteralList<Static> m_static_conditions;
    LiteralList<Fluent> m_fluent_conditions;
    LiteralList<Derived> m_derived_conditions;
    EffectSimpleList m_simple_effects;
    EffectConditionalList m_conditional_effects;
    EffectUniversalList m_universal_effects;
    // More expressive than the definition of action costs:
    // We allow arithmetic functions of ground functions
    // and not just a single ground function (<numeric-term>).
    FunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    ActionImpl(int identifier,
               std::string name,
               size_t original_arity,
               VariableList parameters,
               LiteralList<Static> static_conditions,
               LiteralList<Fluent> fluent_conditions,
               LiteralList<Derived> derived_conditions,
               EffectSimpleList simple_effects,
               EffectConditionalList conditional_effects,
               EffectUniversalList universal_effects,
               FunctionExpression function_expression);

    // Give access to the constructor.
    friend class loki::PDDLFactory<ActionImpl, loki::Hash<ActionImpl*>, loki::EqualTo<ActionImpl*>>;

    /// @brief Test for structural equivalence
    bool is_structurally_equivalent_to_impl(const ActionImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<ActionImpl>;

public:
    const std::string& get_name() const;
    size_t get_original_arity() const;
    const VariableList& get_parameters() const;
    template<PredicateCategory P>
    const LiteralList<P>& get_conditions() const;
    const EffectSimpleList& get_simple_effects() const;
    const EffectConditionalList& get_conditional_effects() const;
    const EffectUniversalList& get_universal_effects() const;
    const FunctionExpression& get_function_expression() const;

    using loki::Base<ActionImpl>::str;
    void str(std::ostream& out, const loki::FormattingOptions& options, bool action_costs) const;

    size_t get_arity() const;
};

/**
 * Type aliases
 */

using Action = const ActionImpl*;
using ActionList = std::vector<Action>;

/**
 * Implementations
 */

template<PredicateCategory P>
const LiteralList<P>& ActionImpl::get_conditions() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_static_conditions;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_conditions;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_conditions;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

}

#endif
