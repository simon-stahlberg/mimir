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
    TransitionIndex m_index;
    StateIndex m_source_state;
    StateIndex m_target_state;
    GroundAction m_creating_action;

public:
    Transition(TransitionIndex index, StateIndex source_state, StateIndex target_state, GroundAction creating_action);

    bool operator==(const Transition& other) const;
    size_t hash() const;

    TransitionIndex get_index() const;
    StateIndex get_source_state() const;
    StateIndex get_target_state() const;
    TransitionCost get_cost() const;
    GroundAction get_creating_action() const;
};

using TransitionList = std::vector<Transition>;

static_assert(IsTransition<Transition>);

/**
 * AbstractTransition
 */

class AbstractTransition
{
private:
    TransitionIndex m_index;
    StateIndex m_source_state;
    StateIndex m_target_state;
    std::span<const GroundAction> m_actions;

public:
    AbstractTransition(TransitionIndex index, StateIndex source_state, StateIndex target_state, std::span<const GroundAction> actions);

    bool operator==(const AbstractTransition& other) const;
    size_t hash() const;

    TransitionIndex get_index() const;
    StateIndex get_source_state() const;
    StateIndex get_target_state() const;
    TransitionCost get_cost() const;
    std::span<const GroundAction> get_actions() const;
    GroundAction get_representative_action() const;
};

using AbstractTransitionList = std::vector<AbstractTransition>;

static_assert(IsTransition<AbstractTransition>);

}

#endif
