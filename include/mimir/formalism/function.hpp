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

namespace mimir
{
class FunctionImpl
{
private:
    Index m_index;
    FunctionSkeleton m_function_skeleton;
    TermList m_terms;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionImpl(Index index, FunctionSkeleton function_skeleton, TermList terms);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    FunctionImpl(const FunctionImpl& other) = delete;
    FunctionImpl& operator=(const FunctionImpl& other) = delete;
    FunctionImpl(FunctionImpl&& other) = default;
    FunctionImpl& operator=(FunctionImpl&& other) = default;

    Index get_index() const;
    const FunctionSkeleton& get_function_skeleton() const;
    const TermList& get_terms() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const { return std::forward_as_tuple(std::as_const(m_function_skeleton), std::as_const(m_terms)); }
};

extern std::ostream& operator<<(std::ostream& out, const FunctionImpl& element);

extern std::ostream& operator<<(std::ostream& out, Function element);

}

#endif
