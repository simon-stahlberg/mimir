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

#include "mimir/formalism/declarations.hpp"

namespace mimir
{
class ActionImpl
{
private:
    Index m_index;
    std::string m_name;
    size_t m_original_arity;
    VariableList m_parameters;
    LiteralList<Static> m_static_conditions;
    LiteralList<Fluent> m_fluent_conditions;
    LiteralList<Derived> m_derived_conditions;
    EffectSimpleList m_simple_effects;
    EffectComplexList m_complex_effects;
    // More expressive than the definition of action costs:
    // We allow arithmetic functions of ground functions
    // and not just a single ground function (<numeric-term>).
    FunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    ActionImpl(Index index,
               std::string name,
               size_t original_arity,
               VariableList parameters,
               LiteralList<Static> static_conditions,
               LiteralList<Fluent> fluent_conditions,
               LiteralList<Derived> derived_conditions,
               EffectSimpleList simple_effects,
               EffectComplexList complex_effects,
               FunctionExpression function_expression);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ActionImpl(const ActionImpl& other) = delete;
    ActionImpl& operator=(const ActionImpl& other) = delete;
    ActionImpl(ActionImpl&& other) = default;
    ActionImpl& operator=(ActionImpl&& other) = default;

    std::string str() const;

    Index get_index() const;
    const std::string& get_name() const;
    size_t get_original_arity() const;
    const VariableList& get_parameters() const;
    template<PredicateTag P>
    const LiteralList<P>& get_conditions() const;
    const EffectSimpleList& get_simple_effects() const;
    const EffectComplexList& get_complex_effects() const;
    const FunctionExpression& get_function_expression() const;

    size_t get_arity() const;
};

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out, const ActionImpl& element);

extern std::ostream& operator<<(std::ostream& out, Action element);

}

#endif
