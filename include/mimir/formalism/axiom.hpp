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

namespace mimir
{
class AxiomImpl
{
private:
    Index m_index;
    VariableList m_parameters;
    Literal<Derived> m_literal;
    LiteralList<Static> m_static_conditions;
    LiteralList<Fluent> m_fluent_conditions;
    LiteralList<Derived> m_derived_conditions;

    // Below: add additional members if needed and initialize them in the constructor

    AxiomImpl(Index index,
              VariableList parameters,
              Literal<Derived> literal,
              LiteralList<Static> static_conditions,
              LiteralList<Fluent> fluent_conditions,
              LiteralList<Derived> derived_conditions);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    AxiomImpl(const AxiomImpl& other) = delete;
    AxiomImpl& operator=(const AxiomImpl& other) = delete;
    AxiomImpl(AxiomImpl&& other) = default;
    AxiomImpl& operator=(AxiomImpl&& other) = default;

    Index get_index() const;
    const VariableList& get_parameters() const;
    const Literal<Derived>& get_literal() const;
    template<PredicateTag P>
    const LiteralList<P>& get_conditions() const;

    size_t get_arity() const;
};

extern std::ostream& operator<<(std::ostream& out, const AxiomImpl& element);

extern std::ostream& operator<<(std::ostream& out, Axiom element);

}

#endif
