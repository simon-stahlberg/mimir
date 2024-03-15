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

#include "mimir/formalism/atom.hpp"

#include "loki/pddl/visitors.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/term.hpp"

#include <loki/utils/collections.hpp>
#include <loki/utils/hash.hpp>

namespace mimir
{
AtomImpl::AtomImpl(int identifier, Predicate predicate, TermList terms) : Base(identifier), m_predicate(std::move(predicate)), m_terms(std::move(terms)) {}

bool AtomImpl::is_structurally_equivalent_to_impl(const AtomImpl& other) const { return (m_predicate == other.m_predicate) && (m_terms == other.m_terms); }

size_t AtomImpl::hash_impl() const { return loki::hash_combine(m_predicate, loki::hash_container(m_terms)); }

void AtomImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const
{
    out << "(" << m_predicate->get_name();
    for (size_t i = 0; i < m_terms.size(); ++i)
    {
        out << " ";
        std::visit(loki::pddl::StringifyVisitor(out, options), *m_terms[i]);
    }
    out << ")";
}

const Predicate& AtomImpl::get_predicate() const { return m_predicate; }

const TermList& AtomImpl::get_terms() const { return m_terms; }
}

namespace std
{
bool less<mimir::Atom>::operator()(const mimir::Atom& left_atom, const mimir::Atom& right_atom) const { return *left_atom < *right_atom; }

std::size_t hash<mimir::AtomImpl>::operator()(const mimir::AtomImpl& atom) const { return atom.hash(); }
}
