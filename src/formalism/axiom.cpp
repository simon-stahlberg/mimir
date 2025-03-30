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

#include "mimir/formalism/axiom.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/conjunctive_condition.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/variable.hpp"

#include <cassert>

namespace mimir::formalism
{
AxiomImpl::AxiomImpl(Index index, ConjunctiveCondition conjunctive_condition, Literal<DerivedTag> literal) :
    m_index(index),
    m_conjunctive_condition(std::move(conjunctive_condition)),
    m_literal(std::move(literal))
{
    assert(conjunctive_condition);
    assert(literal->get_polarity());
}

Index AxiomImpl::get_index() const { return m_index; }

const VariableList& AxiomImpl::get_parameters() const { return m_conjunctive_condition->get_parameters(); }

Literal<DerivedTag> AxiomImpl::get_literal() const { return m_literal; }

ConjunctiveCondition AxiomImpl::get_conjunctive_condition() const { return m_conjunctive_condition; }

size_t AxiomImpl::get_arity() const { return get_parameters().size(); }

std::ostream& operator<<(std::ostream& out, const AxiomImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Axiom element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

}
