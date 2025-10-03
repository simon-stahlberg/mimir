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

#ifndef MIMIR_SEARCH_APPLICABILITY_UTILS_HPP_
#define MIMIR_SEARCH_APPLICABILITY_UTILS_HPP_

#include <loki/loki.hpp>

namespace mimir::search::detail
{

enum class EffectFamily
{
    NONE,
    ASSIGN,
    ADDITIVE,        // += / -=
    MULTIPLICATIVE,  // *= / /=
};

inline bool is_compatible_effect_family(EffectFamily lhs, EffectFamily rhs)
{
    if (lhs == EffectFamily::NONE || rhs == EffectFamily::NONE)
        return true;  ///< first effect

    if (lhs == rhs)
        return lhs != EffectFamily::ASSIGN;  ///< disallow double assignment.

    return false;  ///< disallow mixing assign, additive, or multiplicative
}

inline EffectFamily get_effect_family(loki::AssignOperatorEnum op)
{
    switch (op)
    {
        case loki::AssignOperatorEnum::ASSIGN:
        {
            return EffectFamily::ASSIGN;
        }
        case loki::AssignOperatorEnum::INCREASE:
        {
            return EffectFamily::ADDITIVE;
        }
        case loki::AssignOperatorEnum::DECREASE:
        {
            return EffectFamily::ADDITIVE;
        }
        case loki::AssignOperatorEnum::SCALE_UP:
        {
            return EffectFamily::MULTIPLICATIVE;
        }
        case loki::AssignOperatorEnum::SCALE_DOWN:
        {
            return EffectFamily::MULTIPLICATIVE;
        }
        default:
        {
            throw std::logic_error("Unexpected case.");
        }
    }
}

}

#endif
