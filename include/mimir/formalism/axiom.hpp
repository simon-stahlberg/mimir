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

#ifndef MIMIR_FORMALISM_AXIOM_HPP_
#define MIMIR_FORMALISM_AXIOM_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{
class AxiomImpl
{
private:
    Index m_index;
    ConjunctiveCondition m_conjunctive_condition;
    Literal<DerivedTag> m_literal;

    // Below: add additional members if needed and initialize them in the constructor

    AxiomImpl(Index index, ConjunctiveCondition conjunctive_condition, Literal<DerivedTag> literal);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    AxiomImpl(const AxiomImpl& other) = delete;
    AxiomImpl& operator=(const AxiomImpl& other) = delete;
    AxiomImpl(AxiomImpl&& other) = default;
    AxiomImpl& operator=(AxiomImpl&& other) = default;

    Index get_index() const;
    const VariableList& get_parameters() const;
    ConjunctiveCondition get_conjunctive_condition() const;
    Literal<DerivedTag> get_literal() const;

    size_t get_arity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_conjunctive_condition(), get_literal()); }
};

extern std::ostream& operator<<(std::ostream& out, const AxiomImpl& element);

extern std::ostream& operator<<(std::ostream& out, Axiom element);

}

#endif
