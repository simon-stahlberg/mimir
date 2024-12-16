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

#include "mimir/search/dense_state.hpp"

#include "mimir/common/concepts.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/repositories.hpp"

#include <ostream>
#include <tuple>

size_t cista::storage::DerefStdHasher<mimir::DenseStateImpl>::operator()(const mimir::DenseStateImpl* ptr) const
{
    return mimir::hash_combine(ptr->get_atoms<mimir::Fluent>());
}

bool cista::storage::DerefStdEqualTo<mimir::DenseStateImpl>::operator()(const mimir::DenseStateImpl* lhs, const mimir::DenseStateImpl* rhs) const
{
    return lhs->get_atoms<mimir::Fluent>() == rhs->get_atoms<mimir::Fluent>();
}

namespace mimir
{

/* State */

template<DynamicPredicateTag P>
bool DenseStateImpl::contains(GroundAtom<P> atom) const
{
    return get_atoms<P>().get(atom->get_index());
}

template bool DenseStateImpl::contains(GroundAtom<Fluent> atom) const;
template bool DenseStateImpl::contains(GroundAtom<Derived> atom) const;

template<DynamicPredicateTag P>
bool DenseStateImpl::superset_of(const GroundAtomList<P>& atoms) const
{
    for (const auto& atom : atoms)
    {
        if (!contains(atom))
        {
            return false;
        }
    }

    return true;
}

template bool DenseStateImpl::superset_of(const GroundAtomList<Fluent>& atoms) const;
template bool DenseStateImpl::superset_of(const GroundAtomList<Derived>& atoms) const;

template<DynamicPredicateTag P>
bool DenseStateImpl::literal_holds(GroundLiteral<P> literal) const
{
    return literal->is_negated() != contains(literal->get_atom());
}

template bool DenseStateImpl::literal_holds(GroundLiteral<Fluent> literal) const;
template bool DenseStateImpl::literal_holds(GroundLiteral<Derived> literal) const;

template<DynamicPredicateTag P>
bool DenseStateImpl::literals_hold(const GroundLiteralList<P>& literals) const
{
    for (const auto& literal : literals)
    {
        if (!literal_holds(literal))
        {
            return false;
        }
    }

    return true;
}

template bool DenseStateImpl::literals_hold(const GroundLiteralList<Fluent>& literals) const;
template bool DenseStateImpl::literals_hold(const GroundLiteralList<Derived>& literals) const;

Index& DenseStateImpl::get_index() { return m_index; }

Index DenseStateImpl::get_index() const { return m_index; }

template<DynamicPredicateTag P>
FlatBitset& DenseStateImpl::get_atoms()
{
    if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template FlatBitset& DenseStateImpl::get_atoms<Fluent>();
template FlatBitset& DenseStateImpl::get_atoms<Derived>();

template<DynamicPredicateTag P>
const FlatBitset& DenseStateImpl::get_atoms() const
{
    if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatBitset& DenseStateImpl::get_atoms<Fluent>() const;
template const FlatBitset& DenseStateImpl::get_atoms<Derived>() const;

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<Problem, DenseState, const PDDLRepositories&>& data)
{
    const auto [problem, state, pddl_repositories] = data;

    auto out_fluent_ground_atoms = GroundAtomList<Fluent> {};
    auto out_static_ground_atoms = GroundAtomList<Static> {};
    auto out_derived_ground_atoms = GroundAtomList<Derived> {};

    pddl_repositories.get_ground_atoms_from_indices(state->get_atoms<Fluent>(), out_fluent_ground_atoms);
    pddl_repositories.get_ground_atoms_from_indices(problem->get_static_initial_positive_atoms_bitset(), out_static_ground_atoms);
    pddl_repositories.get_ground_atoms_from_indices(state->get_atoms<Derived>(), out_derived_ground_atoms);

    // Sort by name for easier comparison
    std::sort(out_fluent_ground_atoms.begin(),
              out_fluent_ground_atoms.end(),
              [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });
    std::sort(out_static_ground_atoms.begin(),
              out_static_ground_atoms.end(),
              [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });
    std::sort(out_derived_ground_atoms.begin(),
              out_derived_ground_atoms.end(),
              [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });

    os << "State(" << "index=" << state->get_index() << ", " << "fluent atoms=" << out_fluent_ground_atoms << ", " << "static atoms=" << out_static_ground_atoms
       << ", " << "derived atoms=" << out_derived_ground_atoms << ")";

    return os;
}
}
