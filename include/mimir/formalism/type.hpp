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

#ifndef MIMIR_FORMALISM_TYPE_HPP_
#define MIMIR_FORMALISM_TYPE_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{
class TypeImpl
{
private:
    Index m_index;
    std::string m_name;
    TypeList m_bases;

    TypeImpl(Index index, std::string name, TypeList bases = {});

    static auto identifying_args(const std::string& name, const TypeList& bases) noexcept { return std::tuple(std::cref(name), std::cref(bases)); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    TypeImpl(const TypeImpl& other) = delete;
    TypeImpl& operator=(const TypeImpl& other) = delete;
    TypeImpl(TypeImpl&& other) = default;
    TypeImpl& operator=(TypeImpl&& other) = default;

    Index get_index() const;
    const std::string& get_name() const;
    const TypeList& get_bases() const;

    auto identifying_members() const noexcept { return std::tuple(std::cref(get_name()), std::cref(get_bases())); }
};

class TypesHierarchy
{
private:
    TypeList m_types;

public:
    explicit TypesHierarchy(TypeList types);

    const TypeList& get_types() const;
};

extern bool is_subtypeeq(Type lhs, Type rhs);

extern bool is_subtypeeq(const TypeList& lhs, const TypeList& rhs);
}

#endif
