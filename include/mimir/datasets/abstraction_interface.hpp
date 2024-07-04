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

#ifndef MIMIR_DATASETS_ABSTRACTION_INTERFACE_HPP_
#define MIMIR_DATASETS_ABSTRACTION_INTERFACE_HPP_

#include "mimir/datasets/transition_system_interface.hpp"
#include "mimir/search/state.hpp"

#include <concepts>
#include <memory>

namespace mimir
{

using AbstractionIndex = uint32_t;

/**
 * Internal concept
 */

template<typename T>
concept IsAbstraction = IsTransitionSystem<T> && requires(T a, State concrete_state) {
    {
        a.get_abstract_state_index(concrete_state)
    } -> std::same_as<StateIndex>;
};

}

#endif