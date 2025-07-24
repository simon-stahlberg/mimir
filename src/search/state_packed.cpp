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

template<IsFluentOrDerivedTag P>
bool PackedStateImpl::literal_holds(GroundLiteral<P> literal, const ProblemImpl& problem) const
{
    auto atoms = get_atoms<P>(problem);
    return (std::find(atoms.begin(), atoms.end(), literal->get_index()) != atoms.end()) == literal->get_polarity();
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
    size_t num_fluents = problem.get_inner_tree_table().lookup_root(el.get_atoms<FluentTag>()).i2;

    size_t numeric_index = el.get_numeric_variables();
    size_t num_numeric = problem.get_inner_tree_table().lookup_root(el.get_numeric_variables()).i2;

    PackedStateHashData data { .fluent_index = fluent_index, .num_fluents = num_fluents, .numeric_index = numeric_index, .num_numeric = num_numeric };

    return absl::HashOf(data);
}

bool EqualTo<mimir::search::PackedStateImpl>::operator()(const mimir::search::PackedStateImpl& lhs, const mimir::search::PackedStateImpl& rhs) const
{
    return lhs.get_atoms<FluentTag>() == rhs.get_atoms<FluentTag>() && lhs.get_numeric_variables() == rhs.get_numeric_variables();
}

}