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

#ifndef MIMIR_FORMALISM_EFFECTS_HPP_
#define MIMIR_FORMALISM_EFFECTS_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{

/**
 * Type 1 effects
 */
class EffectStripsImpl
{
private:
    Index m_index;
    LiteralList<Fluent> m_effects;
    FunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    EffectStripsImpl(Index index, LiteralList<Fluent> effects, FunctionExpression function_expression);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    EffectStripsImpl(const EffectStripsImpl& other) = delete;
    EffectStripsImpl& operator=(const EffectStripsImpl& other) = delete;
    EffectStripsImpl(EffectStripsImpl&& other) = default;
    EffectStripsImpl& operator=(EffectStripsImpl&& other) = default;

    Index get_index() const;
    const LiteralList<Fluent>& get_effects() const;
    const FunctionExpression& get_function_expression() const;
};

/**
 * Type 2 effects
 */
class EffectConditionalImpl
{
private:
    Index m_index;
    VariableList m_quantified_variables;
    LiteralList<Static> m_static_conditions;
    LiteralList<Fluent> m_fluent_conditions;
    LiteralList<Derived> m_derived_conditions;
    LiteralList<Fluent> m_effects;
    FunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    EffectConditionalImpl(Index index,
                          VariableList quantified_variables,
                          LiteralList<Static> static_conditions,
                          LiteralList<Fluent> fluent_conditions,
                          LiteralList<Derived> derived_conditions,
                          LiteralList<Fluent> effects,
                          FunctionExpression function_expression);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    EffectConditionalImpl(const EffectConditionalImpl& other) = delete;
    EffectConditionalImpl& operator=(const EffectConditionalImpl& other) = delete;
    EffectConditionalImpl(EffectConditionalImpl&& other) = default;
    EffectConditionalImpl& operator=(EffectConditionalImpl&& other) = default;

    Index get_index() const;
    const VariableList& get_parameters() const;
    template<PredicateTag P>
    const LiteralList<P>& get_conditions() const;
    const LiteralList<Fluent>& get_effects() const;
    const FunctionExpression& get_function_expression() const;

    size_t get_arity() const;
};

extern std::ostream& operator<<(std::ostream& out, const EffectStripsImpl& element);
extern std::ostream& operator<<(std::ostream& out, const EffectConditionalImpl& element);

extern std::ostream& operator<<(std::ostream& out, EffectStrips element);
extern std::ostream& operator<<(std::ostream& out, EffectConditional element);

}

#endif
