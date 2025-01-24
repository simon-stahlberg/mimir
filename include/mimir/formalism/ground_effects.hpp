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
#include "mimir/search/declarations.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>

namespace mimir
{

template<DynamicFunctionTag F>
class GroundEffectNumeric
{
private:
    loki::AssignOperatorEnum m_assign_operator = loki::AssignOperatorEnum::ASSIGN;
    FlatExternalPtr<const GroundFunctionImpl<F>> m_function = nullptr;
    FlatExternalPtr<const GroundFunctionExpressionImpl> m_function_expression = nullptr;

public:
    GroundEffectNumeric() = default;
    GroundEffectNumeric(loki::AssignOperatorEnum assign_operator, GroundFunction<F> function, GroundFunctionExpression function_expression);

    loki::AssignOperatorEnum& get_assign_operator();
    loki::AssignOperatorEnum get_assign_operator() const;

    FlatExternalPtr<const GroundFunctionImpl<F>>& get_function();
    FlatExternalPtr<const GroundFunctionImpl<F>> get_function() const;

    FlatExternalPtr<const GroundFunctionExpressionImpl>& get_function_expression();
    FlatExternalPtr<const GroundFunctionExpressionImpl> get_function_expression() const;

    auto cista_members() noexcept { return std::tie(m_assign_operator, m_function, m_function_expression); }
};

template<DynamicFunctionTag F>
using GroundEffectNumericList = cista::offset::vector<GroundEffectNumeric<F>>;

class GroundEffectStrips
{
private:
    FlatIndexList m_positive_effects = FlatIndexList();
    FlatIndexList m_negative_effects = FlatIndexList();
    GroundEffectNumericList<Fluent> m_fluent_numeric_effects = GroundEffectNumericList<Fluent>();
    GroundEffectNumericList<Auxiliary> m_auxiliary_numeric_effects = GroundEffectNumericList<Auxiliary>();

public:
    /* Propositional effects */
    FlatIndexList& get_positive_effects();
    const FlatIndexList& get_positive_effects() const;

    FlatIndexList& get_negative_effects();
    const FlatIndexList& get_negative_effects() const;

    /* Numeric effects */
    template<DynamicFunctionTag F>
    GroundEffectNumericList<F>& get_numeric_effects();
    template<DynamicFunctionTag F>
    const GroundEffectNumericList<F>& get_numeric_effects() const;

    auto cista_members() noexcept { return std::tie(m_positive_effects, m_negative_effects, m_fluent_numeric_effects, m_auxiliary_numeric_effects); }
};

class GroundEffectConditional
{
private:
    GroundConjunctiveCondition m_strips_condition = GroundConjunctiveCondition();
    GroundEffectStrips m_strips_effect = GroundEffectStrips();

public:
    /* Precondition */
    GroundConjunctiveCondition& get_strips_precondition();
    const GroundConjunctiveCondition& get_strips_precondition() const;

    /* Effect */
    GroundEffectStrips& get_strips_effect();
    const GroundEffectStrips& get_strips_effect() const;

    /* Utility */
    template<PredicateTag P>
    bool is_applicable(const FlatBitset& atoms) const;

    bool is_applicable(const FlatDoubleList& fluent_numeric_variables) const;

    bool is_dynamically_applicable(const DenseState& dense_state) const;

    bool is_statically_applicable(Problem problem) const;

    bool is_applicable(Problem problem, const DenseState& dense_state) const;

    auto cista_members() noexcept { return std::tie(m_strips_condition, m_strips_effect); }
};

using GroundEffectConditionalList = cista::offset::vector<GroundEffectConditional>;

/**
 * Utils
 */

template<DynamicFunctionTag F>
extern ContinuousCost
evaluate(GroundEffectNumeric<F> effect, const FlatDoubleList& fluent_numeric_variables, const FlatDoubleList& auxiliary_numeric_variables);

template<DynamicFunctionTag F>
extern ContinuousCost
evaluate(GroundEffectNumeric<F> effect, const FlatDoubleList& fluent_numeric_variables, const FlatDoubleList& auxiliary_numeric_variables);

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundEffectStrips, const PDDLRepositories&>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundEffectConditional, const PDDLRepositories&>& data);

}

#endif
