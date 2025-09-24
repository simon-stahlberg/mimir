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

#if MIMIR_STATESET_LIST

namespace mimir::search
{

PackedStateImpl::PackedStateImpl(Index fluent_atoms, Index derived_atoms, Index numeric_variables) :
    m_fluent_atoms(fluent_atoms),
    m_derived_atoms(derived_atoms),
    m_numeric_variables(numeric_variables)
{
}

template<IsFluentOrDerivedTag P>
Index PackedStateImpl::get_atoms() const
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

template Index PackedStateImpl::get_atoms<FluentTag>() const;
template Index PackedStateImpl::get_atoms<DerivedTag>() const;

Index PackedStateImpl::get_numeric_variables() const { return m_numeric_variables; }
}

namespace loki
{

Hash<mimir::search::PackedStateImpl>::Hash(const mimir::formalism::ProblemImpl& problem) : problem(problem) {}

size_t Hash<mimir::search::PackedStateImpl>::operator()(const mimir::search::PackedStateImpl& el) const
{
    return loki::hash_combine(el.get_atoms<FluentTag>(),
                              problem.get_index_list(el.get_atoms<FluentTag>())->size(),
                              el.get_numeric_variables(),
                              problem.get_double_list(el.get_numeric_variables())->size());
}

bool EqualTo<mimir::search::PackedStateImpl>::operator()(const mimir::search::PackedStateImpl& lhs, const mimir::search::PackedStateImpl& rhs) const
{
    return lhs.get_atoms<FluentTag>() == rhs.get_atoms<FluentTag>() && lhs.get_numeric_variables() == rhs.get_numeric_variables();
}
}
#elif MIMIR_STATESET_DTDB_S
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

#elif MIMIR_STATESET_DTDB_H

namespace mimir::search
{

PackedStateImpl::PackedStateImpl(Index fluent_atoms, Index derived_atoms, Index numeric_variables) :
    m_fluent_atoms(fluent_atoms),
    m_derived_atoms(derived_atoms),
    m_numeric_variables(numeric_variables)
{
}

template<IsFluentOrDerivedTag P>
Index PackedStateImpl::get_atoms() const
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

template Index PackedStateImpl::get_atoms<FluentTag>() const;
template Index PackedStateImpl::get_atoms<DerivedTag>() const;

Index PackedStateImpl::get_numeric_variables() const { return m_numeric_variables; }

}

namespace loki
{

Hash<mimir::search::PackedStateImpl>::Hash(const mimir::formalism::ProblemImpl& problem) : problem(problem) {}

struct PackedStateHashData
{
    size_t fluent_index = 0;
    size_t num_fluents = 0;
    size_t numeric_index = 0;
    size_t num_numeric = 0;

    template<typename H>
    friend H AbslHashValue(H h, const PackedStateHashData& s)
    {
        if (s.num_fluents > 0)
        {
            h = H::combine(std::move(h), s.fluent_index, s.num_fluents);
        }
        if (s.num_numeric > 0)
        {
            h = H::combine(std::move(h), s.numeric_index, s.num_numeric);
        }
        return h;
    }
};

size_t Hash<mimir::search::PackedStateImpl>::operator()(const mimir::search::PackedStateImpl& el) const
{
    size_t fluent_index = el.get_atoms<FluentTag>();
    size_t num_fluents = problem.get_index_tree_table().lookup_root(el.get_atoms<FluentTag>()).i2;

    size_t numeric_index = el.get_numeric_variables();
    size_t num_numeric = problem.get_index_tree_table().lookup_root(el.get_numeric_variables()).i2;

    PackedStateHashData data { .fluent_index = fluent_index, .num_fluents = num_fluents, .numeric_index = numeric_index, .num_numeric = num_numeric };

    return absl::HashOf(data);
}

bool EqualTo<mimir::search::PackedStateImpl>::operator()(const mimir::search::PackedStateImpl& lhs, const mimir::search::PackedStateImpl& rhs) const
{
    return lhs.get_atoms<FluentTag>() == rhs.get_atoms<FluentTag>() && lhs.get_numeric_variables() == rhs.get_numeric_variables();
}
}
#endif
