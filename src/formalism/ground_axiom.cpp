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

#include "mimir/formalism/ground_axiom.hpp"

#include "formatter.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/formalism/problem.hpp"

namespace mimir::formalism
{

/* GroundAxiom */

GroundAxiomImpl::GroundAxiomImpl(Index index,
                                 Axiom axiom,
                                 ObjectList objects,
                                 GroundConjunctiveCondition conjunctive_condition,
                                 GroundLiteral<DerivedTag> literal) :
    m_index(index),
    m_axiom(axiom),
    m_objects(std::move(objects)),
    m_conjunctive_condition(conjunctive_condition),
    m_literal(literal)
{
}

Index GroundAxiomImpl::get_index() const { return m_index; }

Axiom GroundAxiomImpl::get_axiom() const { return m_axiom; }

const ObjectList& GroundAxiomImpl::get_objects() const { return m_objects; }

GroundConjunctiveCondition GroundAxiomImpl::get_conjunctive_condition() const { return m_conjunctive_condition; }

GroundLiteral<DerivedTag> GroundAxiomImpl::get_literal() const { return m_literal; }

}

namespace mimir
{

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundAxiom, const formalism::ProblemImpl&>& data)
{
    formalism::write(data, formalism::StringFormatter(), os);
    return os;
}
}
