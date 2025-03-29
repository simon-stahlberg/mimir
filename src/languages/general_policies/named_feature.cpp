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

#include "mimir/languages/general_policies/named_feature.hpp"

#include "mimir/languages/general_policies/visitor_interface.hpp"

namespace mimir::languages::general_policies
{
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
NamedFeatureImpl<D>::NamedFeatureImpl(Index index, std::string name, dl::Constructor<D> feature) : m_index(index), m_name(std::move(name)), m_feature(feature)
{
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
void NamedFeatureImpl<D>::accept(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
Index NamedFeatureImpl<D>::get_index() const
{
    return m_index;
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
const std::string& NamedFeatureImpl<D>::get_name() const
{
    return m_name;
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
dl::Constructor<D> NamedFeatureImpl<D>::get_feature() const
{
    return m_feature;
}

template class NamedFeatureImpl<dl::ConceptTag>;
template class NamedFeatureImpl<dl::RoleTag>;
template class NamedFeatureImpl<dl::BooleanTag>;
template class NamedFeatureImpl<dl::NumericalTag>;

}
