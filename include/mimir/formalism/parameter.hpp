/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_FORMALISM_PARAMETER_HPP_
#define MIMIR_FORMALISM_PARAMETER_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{

class ParameterImpl
{
private:
    Index m_index;
    Variable m_variable;
    TypeList m_types;

    ParameterImpl(Index index, Variable variable, TypeList types);

    static auto identifying_args(Variable variable, const TypeList& types) noexcept { return std::tuple(variable, std::cref(types)); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    ParameterImpl(const ParameterImpl& other) = delete;
    ParameterImpl& operator=(const ParameterImpl& other) = delete;
    ParameterImpl(ParameterImpl&& other) = default;
    ParameterImpl& operator=(ParameterImpl&& other) = default;

    Index get_index() const;
    Variable get_variable() const;
    const TypeList& get_bases() const;

    auto identifying_members() const noexcept { return std::tuple(get_variable(), std::cref(get_bases())); }
};
}

#endif
