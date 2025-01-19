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

#include "mimir/search/grounders/numeric_effect_grounder.hpp"

#include "mimir/formalism/repositories.hpp"
#include "mimir/search/grounders/function_expression_grounder.hpp"
#include "mimir/search/grounders/function_grounder.hpp"

namespace mimir
{
NumericEffectGrounder::NumericEffectGrounder(std::shared_ptr<FunctionGrounder> function_grounder, std::shared_ptr<FunctionExpressionGrounder> fexpr_grounder) :
    m_function_grounder(std::move(function_grounder)),
    m_fexpr_grounder(std::move(fexpr_grounder))
{
}

template<DynamicFunctionTag F>
GroundEffectNumeric<F> NumericEffectGrounder::ground(EffectNumeric<F> numeric_effect, const ObjectList& binding)
{
    return GroundEffectNumeric<F>(numeric_effect->get_assign_operator(),
                                  m_function_grounder->ground(numeric_effect->get_function(), binding),
                                  m_fexpr_grounder->ground(numeric_effect->get_function_expression(), binding));
}

template GroundEffectNumeric<Fluent> NumericEffectGrounder::ground(EffectNumeric<Fluent> numeric_effect, const ObjectList& binding);
template GroundEffectNumeric<Auxiliary> NumericEffectGrounder::ground(EffectNumeric<Auxiliary> numeric_effect, const ObjectList& binding);

/**
 * Getters
 */

const std::shared_ptr<FunctionGrounder>& NumericEffectGrounder::get_function_grounder() const { return m_function_grounder; }

const std::shared_ptr<FunctionExpressionGrounder>& NumericEffectGrounder::get_fexpr_grounder() const { return m_fexpr_grounder; }
}
