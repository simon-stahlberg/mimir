/*
 * Copyright (C) 2023 Dominik Drexler
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

#include "mimir/graphs/property.hpp"

#include "mimir/common/formatter.hpp"
#include "mimir/formalism/formatter.hpp"
#include "mimir/graphs/formatter.hpp"

namespace mimir::graphs
{

PropertyValue::PropertyValue(std::shared_ptr<IProperty> property) : m_property(std::move(property)) {}

bool PropertyValue::operator==(const PropertyValue& other) const { return loki::EqualTo<IProperty> {}(*m_property, *other.m_property); }

bool PropertyValue::operator<(const PropertyValue& other) const { return *m_property < *other.m_property; }

std::string PropertyValue::str() const
{
    std::stringstream ss;
    ss << *m_property;
    return ss.str();
}

size_t PropertyValue::hash() const { return loki::Hash<IProperty> {}(*m_property); }

}

namespace loki
{

size_t Hash<mimir::graphs::IProperty>::operator()(const mimir::graphs::IProperty& property) const { return property.hash(); }

bool EqualTo<mimir::graphs::IProperty>::operator()(const mimir::graphs::IProperty& lhs, const mimir::graphs::IProperty& rhs) const { return lhs == rhs; }

size_t Hash<mimir::graphs::PropertyValue>::operator()(const mimir::graphs::PropertyValue& property) const { return property.hash(); }

bool EqualTo<mimir::graphs::PropertyValue>::operator()(const mimir::graphs::PropertyValue& lhs, const mimir::graphs::PropertyValue& rhs) const
{
    return lhs == rhs;
}
}
