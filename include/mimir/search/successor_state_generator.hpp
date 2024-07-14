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

#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATOR_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATOR_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{

/**
 * Implementation class
 */
class SuccessorStateGenerator
{
private:
    std::shared_ptr<IApplicableActionGenerator> m_aag;

    FlatStateSet m_states;
    StateBuilder m_state_builder;

    FlatBitsetBuilder<Fluent> m_reached_fluent_atoms;
    FlatBitsetBuilder<Derived> m_reached_derived_atoms;

public:
    explicit SuccessorStateGenerator(std::shared_ptr<IApplicableActionGenerator> aag);

    SuccessorStateGenerator(const SuccessorStateGenerator& other) = delete;
    SuccessorStateGenerator& operator=(const SuccessorStateGenerator& other) = delete;
    SuccessorStateGenerator(SuccessorStateGenerator&& other) = delete;
    SuccessorStateGenerator& operator=(SuccessorStateGenerator&& other) = delete;

    State get_or_create_initial_state();

    State get_or_create_state(const GroundAtomList<Fluent>& atoms);

    State get_or_create_successor_state(State state, GroundAction action);

    size_t get_state_count() const;

    const FlatBitsetBuilder<Fluent>& get_reached_fluent_ground_atoms() const;

    const FlatBitsetBuilder<Derived>& get_reached_derived_ground_atoms() const;

    std::shared_ptr<IAAG> get_aag() const;
};

}

#endif