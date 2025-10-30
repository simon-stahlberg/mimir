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

namespace mimir::formalism
{

template<IsFluentOrAuxiliaryTag F>
class NumericEffectImpl
{
private:
    Index m_index;
    loki::AssignOperatorEnum m_assign_operator;
    Function<F> m_function;
    FunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    NumericEffectImpl(Index index, loki::AssignOperatorEnum assign_operator, Function<F> function, FunctionExpression function_expression);

    static auto identifying_args(loki::AssignOperatorEnum assign_operator, Function<F> function, FunctionExpression function_expression) noexcept
    {
        return std::tuple(assign_operator, function, function_expression);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    NumericEffectImpl(const NumericEffectImpl& other) = delete;
    NumericEffectImpl& operator=(const NumericEffectImpl& other) = delete;
    NumericEffectImpl(NumericEffectImpl&& other) = default;
    NumericEffectImpl& operator=(NumericEffectImpl&& other) = default;

    Index get_index() const;
    loki::AssignOperatorEnum get_assign_operator() const;
    Function<F> get_function() const;
    FunctionExpression get_function_expression() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_assign_operator(), get_function(), get_function_expression()); }
};

/**
 * Type 1 effects
 */
class ConjunctiveEffectImpl
{
private:
    Index m_index;
    ParameterList m_parameters;
    LiteralList<FluentTag> m_literals;
    NumericEffectList<FluentTag> m_fluent_numeric_effects;
    std::optional<NumericEffect<AuxiliaryTag>> m_auxiliary_numeric_effect;

    // Below: add additional members if needed and initialize them in the constructor

    ConjunctiveEffectImpl(Index index,
                          ParameterList parameters,
                          LiteralList<FluentTag> literals,
                          NumericEffectList<FluentTag> fluent_numeric_effects,
                          std::optional<NumericEffect<AuxiliaryTag>> auxiliary_numeric_effect);

    static auto identifying_args(const ParameterList& parameters,
                                 const LiteralList<FluentTag>& literals,
                                 const NumericEffectList<FluentTag>& fluent_numeric_effects,
                                 std::optional<NumericEffect<AuxiliaryTag>> auxiliary_numeric_effect) noexcept
    {
        return std::tuple(std::cref(parameters), std::cref(literals), std::cref(fluent_numeric_effects), auxiliary_numeric_effect);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    ConjunctiveEffectImpl(const ConjunctiveEffectImpl& other) = delete;
    ConjunctiveEffectImpl& operator=(const ConjunctiveEffectImpl& other) = delete;
    ConjunctiveEffectImpl(ConjunctiveEffectImpl&& other) = default;
    ConjunctiveEffectImpl& operator=(ConjunctiveEffectImpl&& other) = default;

    Index get_index() const;
    const ParameterList& get_parameters() const;
    const LiteralList<FluentTag>& get_literals() const;
    const NumericEffectList<FluentTag>& get_fluent_numeric_effects() const;
    const std::optional<NumericEffect<AuxiliaryTag>>& get_auxiliary_numeric_effect() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept
    {
        return std::tuple(std::cref(get_parameters()), std::cref(get_literals()), std::cref(get_fluent_numeric_effects()), get_auxiliary_numeric_effect());
    }
};

/**
 * Type 2 effects
 */
class ConditionalEffectImpl
{
private:
    Index m_index;
    ConjunctiveCondition m_conjunctive_condition;
    ConjunctiveEffect m_conjunctive_effect;

    // Below: add additional members if needed and initialize them in the constructor

    ConditionalEffectImpl(Index index, ConjunctiveCondition conjunctive_condition, ConjunctiveEffect conjunctive_effect);

    static auto identifying_args(ConjunctiveCondition conjunctive_condition, ConjunctiveEffect conjunctive_effect) noexcept
    {
        return std::tuple(conjunctive_condition, conjunctive_effect);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    ConditionalEffectImpl(const ConditionalEffectImpl& other) = delete;
    ConditionalEffectImpl& operator=(const ConditionalEffectImpl& other) = delete;
    ConditionalEffectImpl(ConditionalEffectImpl&& other) = default;
    ConditionalEffectImpl& operator=(ConditionalEffectImpl&& other) = default;

    Index get_index() const;
    ConjunctiveCondition get_conjunctive_condition() const;
    ConjunctiveEffect get_conjunctive_effect() const;

    size_t get_arity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_conjunctive_condition(), get_conjunctive_effect()); }
};
}

#endif
