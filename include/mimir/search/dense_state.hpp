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

#ifndef MIMIR_SEARCH_DENSE_STATE_HPP_
#define MIMIR_SEARCH_DENSE_STATE_HPP_

#include "cista/serialization.h"
#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{

/// @brief `DenseState` is an intermediate representation of a `StateImpl`.
/// We use it internally for constant time randomized access.
struct DenseState
{
    FlatBitset fluent_atoms;
    FlatBitset derived_atoms;
    FlatDoubleList numeric_variables;

    const formalism::ProblemImpl& problem;

    template<formalism::IsFluentOrDerivedTag P>
    FlatBitset& get_atoms()
    {
        if constexpr (std::is_same_v<P, formalism::FluentTag>)
        {
            return fluent_atoms;
        }
        else if constexpr (std::is_same_v<P, formalism::DerivedTag>)
        {
            return derived_atoms;
        }
        else
        {
            static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
        }
    }
    template<formalism::IsFluentOrDerivedTag P>
    const FlatBitset& get_atoms() const
    {
        if constexpr (std::is_same_v<P, formalism::FluentTag>)
        {
            return fluent_atoms;
        }
        else if constexpr (std::is_same_v<P, formalism::DerivedTag>)
        {
            return derived_atoms;
        }
        else
        {
            static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
        }
    }
    FlatDoubleList& get_numeric_variables() { return numeric_variables; }
    const FlatDoubleList& get_numeric_variables() const { return numeric_variables; }
    const formalism::ProblemImpl& get_problem() const { return problem; }

    DenseState(const formalism::ProblemImpl& problem) : fluent_atoms(), derived_atoms(), numeric_variables(), problem(problem) {}
};

}

#endif
