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

#include <loki/common/collections.hpp>
#include <loki/common/hash.hpp>
#include <loki/common/pddl/visitors.hpp>
#include <mimir/formalism/domain/action.hpp>
#include <mimir/formalism/domain/object.hpp>
#include <mimir/formalism/problem/ground_action.hpp>

namespace mimir
{
GroundActionImpl::GroundActionImpl(int identifier, Action action, ObjectList terms, LiteralList condition, LiteralList effect) :
    Base(identifier),
    m_action(std::move(action)),
    m_terms(std::move(terms)),
    m_condition(std::move(condition)),
    m_effect(std::move(effect))
{
}

bool GroundActionImpl::is_structurally_equivalent_to_impl(const GroundActionImpl& other) const
{
    return (m_action == other.m_action) && (m_terms == other.m_terms);
}

size_t GroundActionImpl::hash_impl() const { return loki::hash_combine(m_action, loki::hash_container(m_terms)); }

void GroundActionImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& /*options*/) const
{
    out << "(" << m_action->get_name();
    for (size_t i = 0; i < m_terms.size(); ++i)
    {
        out << " " << *m_terms[i];
    }
    out << ")";
}

const Action& GroundActionImpl::get_action() const { return m_action; }

const ObjectList& GroundActionImpl::get_terms() const { return m_terms; }

const LiteralList& GroundActionImpl::get_condition() const { return m_condition; }

const LiteralList& GroundActionImpl::get_effect() const { return m_effect; }

size_t GroundActionImpl::get_arity() const { return m_terms.size(); }

}

namespace std
{
bool less<mimir::GroundAction>::operator()(const mimir::GroundAction& left_action, const mimir::GroundAction& right_action) const
{
    return *left_action < *right_action;
}

std::size_t hash<mimir::GroundActionImpl>::operator()(const mimir::GroundActionImpl& action) const { return action.hash(); }
}
