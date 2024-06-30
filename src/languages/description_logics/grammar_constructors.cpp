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

#include <mimir/languages/description_logics/grammar_constructors.hpp>

namespace mimir::dl::grammar
{

/**
 * ConceptAnd
 */
ConceptAnd::ConceptAnd(size_t id, const ConceptChoice& concept_left, const ConceptChoice& concept_right) :
    m_id(id),
    m_concept_left(concept_left),
    m_concept_right(concept_right)
{
}

bool ConceptAnd::operator==(const ConceptAnd& other) const
{
    if (this != &other)
    {
        return (&m_concept_left == &other.m_concept_left) && (&m_concept_right == &other.m_concept_right);
    }
    return true;
}

bool ConceptAnd::is_equal(const Constructor<Concept>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const ConceptAnd&>(other);
    return (*this == otherDerived);
}

size_t ConceptAnd::hash() const { return loki::hash_combine(&m_concept_left, &m_concept_right); }

bool ConceptAnd::test_match(const dl::Constructor<Concept>& constructor) const { return constructor.accept(ConceptAndVisitor(*this)); }

size_t ConceptAnd::get_id() const { return m_id; }

const ConceptChoice& ConceptAnd::get_concept_left() const { return m_concept_left; }

const ConceptChoice& ConceptAnd::get_concept_right() const { return m_concept_right; }

/**
 * RoleAnd
 */
RoleAnd::RoleAnd(size_t id, const RoleChoice& role_left, const RoleChoice& role_right) : m_id(id), m_role_left(role_left), m_role_right(role_right) {}

bool RoleAnd::operator==(const RoleAnd& other) const
{
    if (this != &other)
    {
        return (&m_role_left == &other.m_role_left) && (&m_role_right == &other.m_role_right);
    }
    return true;
}

bool RoleAnd::is_equal(const Constructor<Role>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const RoleAnd&>(other);
    return (*this == otherDerived);
}

size_t RoleAnd::hash() const { return loki::hash_combine(&m_role_left, &m_role_right); }

bool RoleAnd::test_match(const dl::Constructor<Role>& constructor) const { return constructor.accept(RoleAndVisitor(*this)); }

size_t RoleAnd::get_id() const { return m_id; }

const RoleChoice& RoleAnd::get_role_left() const { return m_role_left; }

const RoleChoice& RoleAnd::get_role_right() const { return m_role_right; }

}
