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
    ExistentiallyQuantifiedConjunctiveCondition m_precondition;
    Literal<Derived> m_literal;

    // Below: add additional members if needed and initialize them in the constructor

    AxiomImpl(Index index, ExistentiallyQuantifiedConjunctiveCondition precondition, Literal<Derived> literal);

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
    const ExistentiallyQuantifiedConjunctiveCondition& get_precondition() const;
    const Literal<Derived>& get_literal() const;

    size_t get_arity() const;
};

extern std::ostream& operator<<(std::ostream& out, const AxiomImpl& element);

extern std::ostream& operator<<(std::ostream& out, Axiom element);

}

#endif
