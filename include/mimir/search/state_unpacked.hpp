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

#ifndef MIMIR_SEARCH_STATE_UNPACKED_HPP_
#define MIMIR_SEARCH_STATE_UNPACKED_HPP_

#include "cista/serialization.h"
#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{

/// @brief `UnpackedStateImpl` is an unpacked representation of a `PackedStateImpl`.
/// We unpack for constant time randomized access.
class UnpackedStateImpl
{
private:
    FlatBitset fluent_atoms;
    FlatBitset derived_atoms;
    FlatDoubleList numeric_variables;

    const formalism::ProblemImpl& problem;

public:
    UnpackedStateImpl(const formalism::ProblemImpl& problem);

    template<formalism::IsFluentOrDerivedTag P>
    FlatBitset& get_atoms();
    template<formalism::IsFluentOrDerivedTag P>
    const FlatBitset& get_atoms() const;
    FlatDoubleList& get_numeric_variables();
    const FlatDoubleList& get_numeric_variables() const;
    const formalism::ProblemImpl& get_problem() const;
};

}

#endif
