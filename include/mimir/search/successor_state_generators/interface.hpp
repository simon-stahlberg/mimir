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

#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_INTERFACE_HPP_

#include "mimir/formalism/formalism.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{
/**
 * Dynamic interface class
 */
class ISuccessorStateGenerator
{
public:
    virtual ~ISuccessorStateGenerator() = default;

    [[nodiscard]] virtual State get_or_create_initial_state() = 0;

    /// @brief Expert interface for creating states.
    /// The user must ensure that the atoms are part of the problem from the applicable action generator.
    [[nodiscard]] virtual State get_or_create_state(const GroundAtomList<Fluent>& atoms) = 0;

    [[nodiscard]] virtual State get_or_create_successor_state(const State state, const GroundAction action) = 0;

    [[nodiscard]] virtual size_t get_state_count() const = 0;

    [[nodiscard]] virtual const FlatBitsetBuilder<Fluent>& get_reached_fluent_ground_atoms() const = 0;

    [[nodiscard]] virtual const FlatBitsetBuilder<Derived>& get_reached_derived_ground_atoms() const = 0;

    [[nodiscard]] virtual std::shared_ptr<IAAG> get_aag() const = 0;
};

/**
 * Type aliases
 */

using ISSG = ISuccessorStateGenerator;

}

#endif
