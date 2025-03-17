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

#include "mimir/formalism/requirements.hpp"

#include "formatter.hpp"

#include <cassert>

namespace mimir::formalism
{
RequirementsImpl::RequirementsImpl(Index index, loki::RequirementEnumSet requirements) : m_index(index), m_requirements(std::move(requirements)) {}

bool RequirementsImpl::test(loki::RequirementEnum requirement) const { return m_requirements.count(requirement); }

Index RequirementsImpl::get_index() const { return m_index; }

const loki::RequirementEnumSet& RequirementsImpl::get_requirements() const { return m_requirements; }

std::ostream& operator<<(std::ostream& out, const RequirementsImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Requirements element)
{
    write(*element, AddressFormatter(), out);
    return out;
}
}
