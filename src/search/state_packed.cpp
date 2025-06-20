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

using namespace mimir::formalism;

namespace mimir::search
{

PackedStateImpl::PackedStateImpl(v::RootSlotType fluent_atoms, v::RootSlotType derived_atoms, Index numeric_variables) :
    m_fluent_atoms_index(valla::first(fluent_atoms)),
    m_fluent_atoms_size(valla::second(fluent_atoms)),
    m_derived_atoms_index(valla::first(derived_atoms)),
    m_derived_atoms_size(valla::second(derived_atoms)),
    m_numeric_variables(numeric_variables)
{
}

template<IsFluentOrDerivedTag P>
v::RootSlotType PackedStateImpl::get_atoms() const
{
    if constexpr (std::is_same_v<P, FluentTag>)
    {
        return valla::make_slot(m_fluent_atoms_index, m_fluent_atoms_size);
    }
    else if constexpr (std::is_same_v<P, DerivedTag>)
    {
        return valla::make_slot(m_derived_atoms_index, m_derived_atoms_size);
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
    }
}

template v::RootSlotType PackedStateImpl::get_atoms<FluentTag>() const;
template v::RootSlotType PackedStateImpl::get_atoms<DerivedTag>() const;

Index PackedStateImpl::get_numeric_variables() const { return m_numeric_variables; }

}

namespace loki
{

size_t Hash<mimir::search::PackedStateImpl>::operator()(const mimir::search::PackedStateImpl& el) const
{
    static_assert(std::is_standard_layout_v<mimir::search::PackedStateImpl>, "PackedStateImpl must be standard layout");

    size_t seed = 0;
    size_t hash[2] = { 0, 0 };

    loki::MurmurHash3_x64_128(reinterpret_cast<const uint8_t*>(&el), sizeof(mimir::search::PackedStateImpl), seed, hash);

    loki::hash_combine(seed, hash[0]);
    loki::hash_combine(seed, hash[1]);

    return seed;
}

bool EqualTo<mimir::search::PackedStateImpl>::operator()(const mimir::search::PackedStateImpl& lhs, const mimir::search::PackedStateImpl& rhs) const
{
    static_assert(std::is_standard_layout_v<mimir::search::PackedStateImpl>, "PackedStateImpl must be standard layout");

    const auto lhs_begin = reinterpret_cast<const uint8_t*>(&lhs);
    const auto rhs_begin = reinterpret_cast<const uint8_t*>(&rhs);

    return std::equal(lhs_begin, lhs_begin + sizeof(mimir::search::PackedStateImpl), rhs_begin);
}

}