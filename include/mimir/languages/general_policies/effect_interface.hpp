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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_EFFECT_INTERFACE_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_EFFECT_INTERFACE_HPP_

#include "mimir/languages/general_policies/declarations.hpp"

namespace mimir::languages::general_policies
{
/// @brief `IEffect` represents a Boolean feature effect.
class IEffect
{
protected:
public:
    virtual ~IEffect() = default;

    virtual Index get_index() const = 0;

    virtual bool evaluate(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const = 0;

    /// @brief Accept a `IVisitor`.
    /// @param visitor the `IVisitor`.
    virtual void accept(IVisitor& visitor) const = 0;
};

}

#endif