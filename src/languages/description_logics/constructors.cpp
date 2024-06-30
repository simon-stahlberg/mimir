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

#include <mimir/languages/description_logics/constructors.hpp>

namespace mimir::dl
{

/**
 * ConceptAnd
 */

ConceptAnd::ConceptAnd(size_t id, const Concept& concept_left, const Concept& concept_right) :
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

bool ConceptAnd::is_equal(const Concept& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const ConceptAnd&>(other);
    return (*this == otherDerived);
}

size_t ConceptAnd::hash() const { return loki::hash_combine(&m_concept_left, &m_concept_right); }

void ConceptAnd::evaluate(EvaluationContext& context) const
{
    // TODO
}

bool ConceptAnd::accept(const ConceptVisitor& visitor) const { return visitor.visit(*this); }

size_t ConceptAnd::get_id() const { return m_id; }

const Concept& ConceptAnd::get_concept_left() const { return m_concept_left; }

const Concept& ConceptAnd::get_concept_right() const { return m_concept_right; }

/**
 * RoleAnd
 */

RoleAnd::RoleAnd(size_t id, const Role& role_left, const Role& role_right) : m_id(id), m_role_left(role_left), m_role_right(role_right) {}

bool RoleAnd::operator==(const RoleAnd& other) const
{
    if (this != &other)
    {
        return (&m_role_left == &other.m_role_left) && (&m_role_right == &other.m_role_right);
    }
    return true;
}

bool RoleAnd::is_equal(const Role& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const RoleAnd&>(other);
    return (*this == otherDerived);
}

size_t RoleAnd::hash() const { return loki::hash_combine(&m_role_left, &m_role_right); }

void RoleAnd::evaluate(EvaluationContext& context) const
{
    // TODO
}

bool RoleAnd::accept(const RoleVisitor& visitor) const { return visitor.visit(*this); }

size_t RoleAnd::get_id() const { return m_id; }

const Role& RoleAnd::get_role_left() const { return m_role_left; }

const Role& RoleAnd::get_role_right() const { return m_role_right; }

}
