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

#ifndef MIMIR_FORMALISM_BINDING_HPP_
#define MIMIR_FORMALISM_BINDING_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{

class BindingImpl
{
private:
    Index m_index;
    ObjectList m_objects;

    // Below: add additional members if needed and initialize them in the constructor

    BindingImpl(Index index, ObjectList objects);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    BindingImpl(const BindingImpl& other) = delete;
    BindingImpl& operator=(const BindingImpl& other) = delete;
    BindingImpl(BindingImpl&& other) = default;
    BindingImpl& operator=(BindingImpl&& other) = default;

    Index get_index() const;
    const ObjectList& get_objects() const;
    size_t get_arity() const;
    Object get_object(Index pos) const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(std::cref(get_objects())); }
};

extern std::ostream& operator<<(std::ostream& out, const BindingImpl& element);

extern std::ostream& operator<<(std::ostream& out, Binding element);

}

#endif
