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

#ifndef MIMIR_FORMALISM_GROUND_EFFECTS_HPP_
#define MIMIR_FORMALISM_GROUND_EFFECTS_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/ground_conjunctive_condition.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>

namespace mimir::formalism
{

template<IsFluentOrAuxiliaryTag F>
class GroundNumericEffectImpl
{
private:
    Index m_index;
    loki::AssignOperatorEnum m_assign_operator;
    GroundFunction<F> m_function;
    GroundFunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    GroundNumericEffectImpl(Index index, loki::AssignOperatorEnum assign_operator, GroundFunction<F> function, GroundFunctionExpression function_expression);

    static auto identifying_args(loki::AssignOperatorEnum assign_operator, GroundFunction<F> function, GroundFunctionExpression function_expression) noexcept
    {
        return std::tuple(assign_operator, function, function_expression);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    GroundNumericEffectImpl(const GroundNumericEffectImpl& other) = delete;
    GroundNumericEffectImpl& operator=(const GroundNumericEffectImpl& other) = delete;
    GroundNumericEffectImpl(GroundNumericEffectImpl&& other) = default;
    GroundNumericEffectImpl& operator=(GroundNumericEffectImpl&& other) = default;

    Index get_index() const;
    loki::AssignOperatorEnum get_assign_operator() const;
    GroundFunction<F> get_function() const;
    GroundFunctionExpression get_function_expression() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_assign_operator(), get_function(), get_function_expression()); }
};

class GroundConjunctiveEffectImpl
{
private:
    Index m_index;
    HanaContainer<const FlatIndexList*, PositiveTag, NegativeTag> m_propositional_effects;
    GroundNumericEffectList<FluentTag> m_fluent_numeric_effects;
    std::optional<GroundNumericEffect<AuxiliaryTag>> m_auxiliary_numeric_effect;

    GroundConjunctiveEffectImpl(Index index,
                                HanaContainer<const FlatIndexList*, PositiveTag, NegativeTag> propositional_effects,
                                GroundNumericEffectList<FluentTag> fluent_numeric_effects,
                                std::optional<GroundNumericEffect<AuxiliaryTag>> auxiliary_numeric_effect);

    static auto identifying_args(const HanaContainer<const FlatIndexList*, PositiveTag, NegativeTag>& propositional_effects,
                                 const GroundNumericEffectList<FluentTag>& fluent_numeric_effects,
                                 std::optional<GroundNumericEffect<AuxiliaryTag>> auxiliary_numeric_effect) noexcept
    {
        return std::tuple(boost::hana::at_key(propositional_effects, boost::hana::type<PositiveTag> {}),
                          boost::hana::at_key(propositional_effects, boost::hana::type<NegativeTag> {}),
                          std::cref(fluent_numeric_effects),
                          auxiliary_numeric_effect);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    GroundConjunctiveEffectImpl(const GroundConjunctiveEffectImpl& other) = delete;
    GroundConjunctiveEffectImpl& operator=(const GroundConjunctiveEffectImpl& other) = delete;
    GroundConjunctiveEffectImpl(GroundConjunctiveEffectImpl&& other) = default;
    GroundConjunctiveEffectImpl& operator=(GroundConjunctiveEffectImpl&& other) = default;

    Index get_index() const;
    template<IsPolarity R>
    const FlatIndexList& get_compressed_propositional_effects() const
    {
        return *boost::hana::at_key(m_propositional_effects, boost::hana::type<R> {});
    }
    template<IsPolarity R>
    auto get_propositional_effects() const
    {
        return boost::hana::at_key(m_propositional_effects, boost::hana::type<R> {})->compressed_range();
    }
    const GroundNumericEffectList<FluentTag>& get_fluent_numeric_effects() const;
    const std::optional<GroundNumericEffect<AuxiliaryTag>>& get_auxiliary_numeric_effect() const;

    auto identifying_members() const noexcept
    {
        return std::tuple(&get_compressed_propositional_effects<PositiveTag>(),
                          &get_compressed_propositional_effects<NegativeTag>(),
                          std::cref(m_fluent_numeric_effects),
                          m_auxiliary_numeric_effect);
    }
};

class GroundConditionalEffectImpl
{
private:
    Index m_index;
    GroundConjunctiveCondition m_conjunctive_condition;
    GroundConjunctiveEffect m_conjunctive_effect;

    GroundConditionalEffectImpl(Index index, GroundConjunctiveCondition conjunctive_condition, GroundConjunctiveEffect conjunctive_effect);

    static auto identifying_args(GroundConjunctiveCondition conjunctive_condition, GroundConjunctiveEffect conjunctive_effect) noexcept
    {
        return std::tuple(conjunctive_condition, conjunctive_effect);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    GroundConditionalEffectImpl(const GroundConditionalEffectImpl& other) = delete;
    GroundConditionalEffectImpl& operator=(const GroundConditionalEffectImpl& other) = delete;
    GroundConditionalEffectImpl(GroundConditionalEffectImpl&& other) = default;
    GroundConditionalEffectImpl& operator=(GroundConditionalEffectImpl&& other) = default;

    Index get_index() const;
    GroundConjunctiveCondition get_conjunctive_condition() const;
    GroundConjunctiveEffect get_conjunctive_effect() const;

    auto identifying_members() const noexcept { return std::tuple(get_conjunctive_condition(), get_conjunctive_effect()); }
};

/**
 * Utils
 */

template<IsFluentOrAuxiliaryTag F>
extern std::pair<loki::AssignOperatorEnum, ContinuousCost>
evaluate(GroundNumericEffect<F> effect, const FlatDoubleList& static_numeric_variables, const FlatDoubleList& fluent_numeric_variables);

extern bool is_compatible_numeric_effect(loki::AssignOperatorEnum lhs, loki::AssignOperatorEnum rhs);

/**
 * Pretty printing
 */

template<IsFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const GroundNumericEffectImpl<F>& element);

template<IsFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, GroundNumericEffect<F> element);

}

namespace mimir
{
template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundConjunctiveEffect, const formalism::ProblemImpl&>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundConditionalEffect, const formalism::ProblemImpl&>& data);
}

#endif