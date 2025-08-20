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

template<IsFluentOrDerivedTag P>
bool PackedStateImpl::literal_holds(GroundLiteral<P> literal, const ProblemImpl& problem) const
{
    auto atoms = get_atoms<P>(problem);
    return (std::find(atoms.begin(), atoms.end(), literal->get_atom()->get_index()) != atoms.end()) == literal->get_polarity();
}

template bool PackedStateImpl::literal_holds(GroundLiteral<FluentTag> literal, const ProblemImpl& problem) const;
template bool PackedStateImpl::literal_holds(GroundLiteral<DerivedTag> literal, const ProblemImpl& problem) const;

template<IsFluentOrDerivedTag P>
bool PackedStateImpl::literals_hold(const GroundLiteralList<P>& literals, const ProblemImpl& problem) const
{
    return std::all_of(literals.begin(), literals.end(), [this, &problem](auto&& arg) { return this->literal_holds(arg, problem); });
}

template bool PackedStateImpl::literals_hold(const GroundLiteralList<FluentTag>& literals, const ProblemImpl& problem) const;
template bool PackedStateImpl::literals_hold(const GroundLiteralList<DerivedTag>& literals, const ProblemImpl& problem) const;

}

namespace loki
{

size_t Hash<mimir::search::PackedStateImpl>::operator()(const mimir::search::PackedStateImpl& el) const
{
    return loki::hash_combine(valla::Hasher<valla::Slot<mimir::Index>> {}(el.get_atoms<FluentTag>()),
                              valla::Hasher<valla::Slot<mimir::Index>> {}(el.get_numeric_variables()));
}

bool EqualTo<mimir::search::PackedStateImpl>::operator()(const mimir::search::PackedStateImpl& lhs, const mimir::search::PackedStateImpl& rhs) const
{
    return lhs.get_atoms<FluentTag>() == rhs.get_atoms<FluentTag>() && lhs.get_numeric_variables() == rhs.get_numeric_variables();
}

}
