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

template<DynamicFunctionTag F>
class NumericEffectImpl
{
private:
    Index m_index;
    loki::AssignOperatorEnum m_assign_operator;
    Function<F> m_function;
    FunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    NumericEffectImpl(Index index, loki::AssignOperatorEnum assign_operator, Function<F> function, FunctionExpression function_expression);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    NumericEffectImpl(const NumericEffectImpl& other) = delete;
    NumericEffectImpl& operator=(const NumericEffectImpl& other) = delete;
    NumericEffectImpl(NumericEffectImpl&& other) = default;
    NumericEffectImpl& operator=(NumericEffectImpl&& other) = default;

    Index get_index() const;
    loki::AssignOperatorEnum get_assign_operator() const;
    const Function<F>& get_function() const;
    const FunctionExpression& get_function_expression() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const
    {
        return std::forward_as_tuple(std::as_const(m_assign_operator), std::as_const(m_function), std::as_const(m_function_expression));
    }
};

/**
 * Type 1 effects
 */
class ConjunctiveEffectImpl
{
private:
    Index m_index;
    VariableList m_parameters;
    LiteralList<Fluent> m_literals;
    NumericEffectList<Fluent> m_fluent_numeric_effects;
    NumericEffectList<Auxiliary> m_auxiliary_numeric_effects;

    // Below: add additional members if needed and initialize them in the constructor

    ConjunctiveEffectImpl(Index index,
                          VariableList parameters,
                          LiteralList<Fluent> literals,
                          NumericEffectList<Fluent> fluent_numeric_effects,
                          NumericEffectList<Auxiliary> auxiliary_numeric_effects);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConjunctiveEffectImpl(const ConjunctiveEffectImpl& other) = delete;
    ConjunctiveEffectImpl& operator=(const ConjunctiveEffectImpl& other) = delete;
    ConjunctiveEffectImpl(ConjunctiveEffectImpl&& other) = default;
    ConjunctiveEffectImpl& operator=(ConjunctiveEffectImpl&& other) = default;

    Index get_index() const;
    const VariableList& get_parameters() const;
    const LiteralList<Fluent>& get_literals() const;
    template<DynamicFunctionTag F>
    const NumericEffectList<F>& get_numeric_effects() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const
    {
        return std::forward_as_tuple(std::as_const(m_literals), std::as_const(m_fluent_numeric_effects), std::as_const(m_auxiliary_numeric_effects));
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

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConditionalEffectImpl(const ConditionalEffectImpl& other) = delete;
    ConditionalEffectImpl& operator=(const ConditionalEffectImpl& other) = delete;
    ConditionalEffectImpl(ConditionalEffectImpl&& other) = default;
    ConditionalEffectImpl& operator=(ConditionalEffectImpl&& other) = default;

    Index get_index() const;
    const ConjunctiveCondition& get_conjunctive_condition() const;
    const ConjunctiveEffect& get_conjunctive_effect() const;

    size_t get_arity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const { return std::forward_as_tuple(std::as_const(m_conjunctive_condition), std::as_const(m_conjunctive_effect)); }
};

template<DynamicFunctionTag F>
extern std::ostream& operator<<(std::ostream& out, const NumericEffectImpl<F>& element);
extern std::ostream& operator<<(std::ostream& out, const ConjunctiveEffectImpl& element);
extern std::ostream& operator<<(std::ostream& out, const ConditionalEffectImpl& element);

template<DynamicFunctionTag F>
extern std::ostream& operator<<(std::ostream& out, NumericEffect<F> element);
extern std::ostream& operator<<(std::ostream& out, ConjunctiveEffect element);
extern std::ostream& operator<<(std::ostream& out, ConditionalEffect element);

}

#endif
