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

#include <cassert>
#include <loki/common/hash.hpp>
#include <mimir/formalism/requirements.hpp>

namespace mimir
{
RequirementsImpl::RequirementsImpl(int identifier, loki::pddl::RequirementEnumSet requirements) : Base(identifier), m_requirements(std::move(requirements)) {}

bool RequirementsImpl::is_structurally_equivalent_to_impl(const RequirementsImpl& other) const { return (m_requirements == other.m_requirements); }

size_t RequirementsImpl::hash_impl() const { return loki::hash_container(m_requirements); }

void RequirementsImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& /*options*/) const
{
    out << "(:requirements ";
    int i = 0;
    for (const auto& requirement : m_requirements)
    {
        if (i != 0)
            out << " ";
        out << to_string(requirement);
        ++i;
    }
    out << ")";
}

bool RequirementsImpl::test(loki::pddl::RequirementEnum requirement) const { return m_requirements.count(requirement); }

const loki::pddl::RequirementEnumSet& RequirementsImpl::get_requirements() const { return m_requirements; }
}

namespace std
{
bool less<mimir::Requirements>::operator()(const mimir::Requirements& left_requirements, const mimir::Requirements& right_requirements) const
{
    return *left_requirements < *right_requirements;
}

std::size_t hash<mimir::RequirementsImpl>::operator()(const mimir::RequirementsImpl& requirements) const { return requirements.hash(); }
}
