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

#ifndef MIMIR_DATASETS_CONCRETE_TRANSITION_HPP_
#define MIMIR_DATASETS_CONCRETE_TRANSITION_HPP_

#include "mimir/datasets/declarations.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/state.hpp"

#include <span>
#include <vector>

namespace mimir
{

/**
 * ConcreteTransition
 */

class ConcreteTransition
{
private:
    TransitionIndex m_index;
    StateIndex m_source_state;
    StateIndex m_target_state;
    GroundAction m_creating_action;

public:
    ConcreteTransition(TransitionIndex index, StateIndex source_state, StateIndex target_state, GroundAction creating_action);

    bool operator==(const ConcreteTransition& other) const;
    size_t hash() const;

    TransitionIndex get_index() const;
    StateIndex get_source() const;
    StateIndex get_target() const;
    TransitionCost get_cost() const;
    GroundAction get_creating_action() const;
};

using ConcreteTransitionList = std::vector<ConcreteTransition>;

}

#endif
