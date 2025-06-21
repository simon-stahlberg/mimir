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

#include "mimir/search/state_unpacked.hpp"

using namespace mimir::formalism;

namespace mimir::search
{
UnpackedStateImpl::UnpackedStateImpl(const ProblemImpl& problem) : fluent_atoms(), derived_atoms(), numeric_variables(), problem(problem) {}

template<IsFluentOrDerivedTag P>
FlatBitset& UnpackedStateImpl::get_atoms()
{
    if constexpr (std::is_same_v<P, FluentTag>)
    {
        return fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, DerivedTag>)
    {
        return derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
    }
}

template FlatBitset& UnpackedStateImpl::get_atoms<FluentTag>();
template FlatBitset& UnpackedStateImpl::get_atoms<DerivedTag>();

template<IsFluentOrDerivedTag P>
const FlatBitset& UnpackedStateImpl::get_atoms() const
{
    if constexpr (std::is_same_v<P, FluentTag>)
    {
        return fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, DerivedTag>)
    {
        return derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
    }
}

template const FlatBitset& UnpackedStateImpl::get_atoms<FluentTag>() const;
template const FlatBitset& UnpackedStateImpl::get_atoms<DerivedTag>() const;

FlatDoubleList& UnpackedStateImpl::get_numeric_variables() { return numeric_variables; }

const FlatDoubleList& UnpackedStateImpl::get_numeric_variables() const { return numeric_variables; }

const ProblemImpl& UnpackedStateImpl::get_problem() const { return problem; }
}
