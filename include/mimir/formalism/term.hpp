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

#ifndef MIMIR_FORMALISM_TERM_HPP_
#define MIMIR_FORMALISM_TERM_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{

class TermImpl
{
private:
    size_t m_index;
    std::variant<Object, Variable> m_object_or_variable;

    TermImpl(size_t index, std::variant<Object, Variable> object_or_variable);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    TermImpl(const TermImpl& other) = delete;
    TermImpl& operator=(const TermImpl& other) = delete;
    TermImpl(TermImpl&& other) = default;
    TermImpl& operator=(TermImpl&& other) = default;

    size_t get_index() const;
    const std::variant<Object, Variable>& get_variant() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_variant()); }
};

extern std::ostream& operator<<(std::ostream& out, const TermImpl& element);

extern std::ostream& operator<<(std::ostream& out, Term element);

}

#endif
