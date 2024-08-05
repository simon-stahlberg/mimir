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

#ifndef MIMIR_DATASETS_ABSTRACT_TRANSITION_HPP_
#define MIMIR_DATASETS_ABSTRACT_TRANSITION_HPP_

#include "mimir/datasets/declarations.hpp"
#include "mimir/graphs/graph_edges.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/state.hpp"

#include <span>
#include <vector>

namespace mimir
{

class AbstractTransition : public BaseEdge<AbstractTransition>
{
public:
    AbstractTransition(TransitionIndex index, StateIndex source_state, StateIndex target_state, std::span<const GroundAction> actions);

    TransitionCost get_cost() const;
    std::span<const GroundAction> get_actions() const;
    GroundAction get_representative_action() const;

private:
    std::span<const GroundAction> m_actions;

    bool is_equal_impl(const AbstractTransition& other) const;
    size_t hash_impl() const;

    friend class BaseEdge<AbstractTransition>;
};

static_assert(IsEdge<AbstractTransition>);

using AbstractTransitionList = std::vector<AbstractTransition>;

}

#endif
