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

#include "mimir/buffering/unordered_set.h"
#include "mimir/buffering/vector.h"
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

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;

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
    auto identifying_members() const { return std::tuple(get_assign_operator(), get_function(), get_function_expression()); }
};

template<IsFluentOrAuxiliaryTag F>
using GroundNumericEffectList = cista::offset::vector<FlatExternalPtr<const GroundNumericEffectImpl<F>>>;

class GroundConjunctiveEffect
{
private:
    FlatIndexList m_positive_effects = FlatIndexList();
    FlatIndexList m_negative_effects = FlatIndexList();
    GroundNumericEffectList<FluentTag> m_fluent_numeric_effects = GroundNumericEffectList<FluentTag>();
    cista::optional<FlatExternalPtr<const GroundNumericEffectImpl<AuxiliaryTag>>> m_auxiliary_numeric_effect = std::nullopt;

public:
    /* Propositional effects */
    FlatIndexList& get_positive_effects();
    const FlatIndexList& get_positive_effects() const;

    FlatIndexList& get_negative_effects();
    const FlatIndexList& get_negative_effects() const;

    /* Numeric effects */
    GroundNumericEffectList<FluentTag>& get_fluent_numeric_effects();
    const GroundNumericEffectList<FluentTag>& get_fluent_numeric_effects() const;

    cista::optional<FlatExternalPtr<const GroundNumericEffectImpl<AuxiliaryTag>>>& get_auxiliary_numeric_effect();
    const cista::optional<FlatExternalPtr<const GroundNumericEffectImpl<AuxiliaryTag>>>& get_auxiliary_numeric_effect() const;

    auto cista_members() noexcept { return std::tie(m_positive_effects, m_negative_effects, m_fluent_numeric_effects, m_auxiliary_numeric_effect); }
};

class GroundConditionalEffect
{
private:
    GroundConjunctiveCondition m_conjunctive_condition = GroundConjunctiveCondition();
    GroundConjunctiveEffect m_conjunctive_effect = GroundConjunctiveEffect();

public:
    /* Precondition */
    GroundConjunctiveCondition& get_conjunctive_condition();
    const GroundConjunctiveCondition& get_conjunctive_condition() const;

    /* Effect */
    GroundConjunctiveEffect& get_conjunctive_effect();
    const GroundConjunctiveEffect& get_conjunctive_effect() const;

    /* Utility */
    auto cista_members() noexcept { return std::tie(m_conjunctive_condition, m_conjunctive_effect); }
};

using GroundEffectConditionalList = cista::offset::vector<GroundConditionalEffect>;

/**
 * Utils
 */

template<IsFluentOrAuxiliaryTag F>
extern std::pair<loki::AssignOperatorEnum, ContinuousCost>
evaluate(GroundNumericEffect<F> effect, const FlatDoubleList& static_numeric_variables, const FlatDoubleList& fluent_numeric_variables);

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
