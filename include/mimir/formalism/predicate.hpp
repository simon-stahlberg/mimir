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

#ifndef MIMIR_FORMALISM_PREDICATE_HPP_
#define MIMIR_FORMALISM_PREDICATE_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{

template<IsStaticOrFluentOrDerivedTag P>
class PredicateImpl
{
private:
    Index m_index;
    std::string m_name;
    VariableList m_parameters;

    // Below: add additional members if needed and initialize them in the constructor

    PredicateImpl(Index index, std::string name, VariableList parameters);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;
    using Type = P;

    // moveable but not copyable
    PredicateImpl(const PredicateImpl& other) = delete;
    PredicateImpl& operator=(const PredicateImpl& other) = delete;
    PredicateImpl(PredicateImpl&& other) = default;
    PredicateImpl& operator=(PredicateImpl&& other) = default;

    Index get_index() const;
    const std::string& get_name() const;
    const VariableList& get_parameters() const;
    size_t get_arity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(std::cref(get_name()), std::cref(get_parameters())); }
};

template<IsStaticOrFluentOrDerivedTag P>
extern std::ostream& operator<<(std::ostream& out, const PredicateImpl<P>& element);

template<IsStaticOrFluentOrDerivedTag P>
extern std::ostream& operator<<(std::ostream& out, Predicate<P> element);

}

#endif
