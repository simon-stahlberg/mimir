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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_EQUAL_TO_HPP_
#define MIMIR_FORMALISM_EQUAL_TO_HPP_

#include "mimir/formalism/declarations.hpp"

#include <functional>
#include <variant>

namespace mimir
{

/// @brief `UniquePDDLEqualTo` is used to compare newly created PDDL objects for uniqueness.
/// Since the children are unique, it suffices to compare nested pointers.
template<typename T>
struct UniquePDDLEqualTo
{
    bool operator()(const T& l, const T& r) const { return std::equal_to<T>()(l, r); }
};

/// Spezialization for std::variant.
template<typename... Ts>
struct UniquePDDLEqualTo<std::variant<Ts...>>
{
    bool operator()(const std::variant<Ts...>& l, const std::variant<Ts...>& r) const
    {
        if (l.index() != r.index())
        {
            return false;  // Different types held
        }
        // Compare the held values, but only if they are of the same type
        return std::visit(
            [](const auto& lhs, const auto& rhs) -> bool
            {
                if constexpr (std::is_same_v<decltype(lhs), decltype(rhs)>)
                {
                    return UniquePDDLEqualTo<decltype(lhs)>()(lhs, rhs);
                }
                else
                {
                    return false;  // Different types can't be equal
                }
            },
            l,
            r);
    }
};

template<>
struct UniquePDDLEqualTo<const ActionImpl*>
{
    bool operator()(const ActionImpl* l, const ActionImpl* r) const;
};

template<PredicateCategory P>
struct UniquePDDLEqualTo<const AtomImpl<P>*>
{
    bool operator()(const AtomImpl<P>* l, const AtomImpl<P>* r) const;
};

}

#endif
