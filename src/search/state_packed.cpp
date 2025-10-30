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

#include "mimir/search/state_packed.hpp"

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

using namespace mimir::formalism;

namespace mimir::search
{

PackedStateImpl::PackedStateImpl(valla::Slot<Index> fluent_atoms, valla::Slot<Index> derived_atoms, valla::Slot<Index> numeric_variables) :
    m_fluent_atoms(fluent_atoms),
    m_derived_atoms(derived_atoms),
    m_numeric_variables(numeric_variables)
{
}

template<IsFluentOrDerivedTag P>
valla::Slot<Index> PackedStateImpl::get_atoms() const
{
    if constexpr (std::is_same_v<P, FluentTag>)
    {
        return m_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, DerivedTag>)
    {
        return m_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
    }
}

template valla::Slot<Index> PackedStateImpl::get_atoms<FluentTag>() const;
template valla::Slot<Index> PackedStateImpl::get_atoms<DerivedTag>() const;

valla::Slot<Index> PackedStateImpl::get_numeric_variables() const { return m_numeric_variables; }
}

namespace loki
{

size_t Hash<mimir::search::PackedStateImpl>::operator()(const mimir::search::PackedStateImpl& el) const
{
    return loki::hash_combine(valla::Hash<valla::Slot<mimir::Index>> {}(el.get_atoms<FluentTag>()),
                              valla::Hash<valla::Slot<mimir::Index>> {}(el.get_numeric_variables()));
}

bool EqualTo<mimir::search::PackedStateImpl>::operator()(const mimir::search::PackedStateImpl& lhs, const mimir::search::PackedStateImpl& rhs) const
{
    return lhs.get_atoms<FluentTag>() == rhs.get_atoms<FluentTag>() && lhs.get_numeric_variables() == rhs.get_numeric_variables();
}

}
