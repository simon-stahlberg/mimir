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

#ifndef MIMIR_FORMALISM_FUNCTION_HPP_
#define MIMIR_FORMALISM_FUNCTION_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{
template<IsStaticOrFluentOrAuxiliaryTag F>
class FunctionImpl
{
private:
    Index m_index;
    FunctionSkeleton<F> m_function_skeleton;
    TermList m_terms;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionImpl(Index index, FunctionSkeleton<F> function_skeleton, TermList terms);

    static auto identifying_args(FunctionSkeleton<F> function_skeleton, const TermList& terms) noexcept
    {
        return std::tuple(function_skeleton, std::cref(terms));
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    FunctionImpl(const FunctionImpl& other) = delete;
    FunctionImpl& operator=(const FunctionImpl& other) = delete;
    FunctionImpl(FunctionImpl&& other) = default;
    FunctionImpl& operator=(FunctionImpl&& other) = default;

    Index get_index() const;
    FunctionSkeleton<F> get_function_skeleton() const;
    const TermList& get_terms() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_function_skeleton(), std::cref(get_terms())); }
};

static_assert(loki::HasIdentifyingMembers<FunctionImpl<FluentTag>>);
}

#endif