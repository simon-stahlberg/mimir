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

#ifndef MIMIR_DATASETS_TRANSITION_HPP_
#define MIMIR_DATASETS_TRANSITION_HPP_

#include "mimir/datasets/transition_interface.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/state.hpp"

#include <span>
#include <vector>

namespace mimir
{

/**
 * Transition
 */

class Transition
{
private:
    StateIndex m_source_state;
    StateIndex m_target_state;
    GroundAction m_creating_action;

public:
    Transition(StateIndex source_state, StateIndex target_state, GroundAction creating_action);

    [[nodiscard]] bool operator==(const Transition& other) const;
    [[nodiscard]] size_t hash() const;

    [[nodiscard]] StateIndex get_source_state() const;
    [[nodiscard]] StateIndex get_target_state() const;
    [[nodiscard]] TransitionCost get_cost() const;
    [[nodiscard]] GroundAction get_creating_action() const;
};

using TransitionList = std::vector<Transition>;

static_assert(IsTransition<Transition>);

/**
 * AbstractTransition
 */

class AbstractTransition
{
private:
    StateIndex m_source_state;
    StateIndex m_target_state;
    std::span<GroundAction> m_creating_actions;

public:
    AbstractTransition(StateIndex source_state, StateIndex target_state, std::span<GroundAction> creating_actions);

    [[nodiscard]] bool operator==(const AbstractTransition& other) const;
    [[nodiscard]] size_t hash() const;

    StateIndex get_source_state() const;
    StateIndex get_target_state() const;
    [[nodiscard]] TransitionCost get_cost() const;
    std::span<GroundAction> get_creating_actions() const;
};

using AbstractTransitionList = std::vector<AbstractTransition>;

static_assert(IsTransition<AbstractTransition>);

}

#endif
