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
#include "mimir/formalism/existentially_quantified_conjunctive_condition.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/variable.hpp"

#include <cassert>

namespace mimir
{
AxiomImpl::AxiomImpl(Index index, ExistentiallyQuantifiedConjunctiveCondition precondition, Literal<Derived> literal) :
    m_index(index),
    m_precondition(std::move(precondition)),
    m_literal(std::move(literal))
{
    assert(!literal->is_negated());
}

Index AxiomImpl::get_index() const { return m_index; }

const VariableList& AxiomImpl::get_parameters() const { return m_precondition->get_parameters(); }

const Literal<Derived>& AxiomImpl::get_literal() const { return m_literal; }

const ExistentiallyQuantifiedConjunctiveCondition& AxiomImpl::get_precondition() const { return m_precondition; }

size_t AxiomImpl::get_arity() const { return get_parameters().size(); }

std::ostream& operator<<(std::ostream& out, const AxiomImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Axiom element)
{
    out << *element;
    return out;
}

}
