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

#include "mimir/formalism/ground_atom.hpp"

#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"

#include <loki/loki.hpp>

namespace mimir
{
GroundAtomImpl::GroundAtomImpl(int identifier, Predicate predicate, ObjectList objects) :
    Base(identifier),
    m_predicate(std::move(predicate)),
    m_objects(std::move(objects))
{
}

bool GroundAtomImpl::is_structurally_equivalent_to_impl(const GroundAtomImpl& other) const
{
    if (this != &other)
    {
        return (m_predicate == other.m_predicate) && (m_objects == other.m_objects);
    }
    return true;
}

size_t GroundAtomImpl::hash_impl() const { return loki::hash_combine(m_predicate, loki::hash_container(m_objects)); }

void GroundAtomImpl::str_impl(std::ostream& out, const loki::FormattingOptions& /*options*/) const
{
    out << "(" << m_predicate->get_name();
    for (size_t i = 0; i < m_objects.size(); ++i)
    {
        out << " " << m_objects[i]->get_name();
    }
    out << ")";
}

const Predicate& GroundAtomImpl::get_predicate() const { return m_predicate; }

const ObjectList& GroundAtomImpl::get_objects() const { return m_objects; }

bool GroundAtomImpl::is_static() const { return m_predicate->is_static(); }

size_t GroundAtomImpl::get_arity() const { return m_objects.size(); }

}
