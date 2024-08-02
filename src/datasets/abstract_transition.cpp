/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "mimir/datasets/abstract_transition.hpp"

namespace mimir
{

AbstractTransition::AbstractTransition(TransitionIndex index,
                                       StateIndex source_state,
                                       StateIndex target_state,
                                       std::span<const GroundAction> creating_actions) :
    m_index(index),
    m_source_state(source_state),
    m_target_state(target_state),
    m_actions(creating_actions)
{
}

bool AbstractTransition::operator==(const AbstractTransition& other) const
{
    if (this != &other)
    {
        return (m_source_state == other.m_source_state)         //
               && (m_target_state == other.m_target_state)      //
               && (m_actions.size() == other.m_actions.size())  //
               && std::equal(m_actions.begin(), m_actions.end(), other.m_actions.begin());
    }
    return true;
}

size_t AbstractTransition::hash() const
{
    size_t creating_actions_hash = m_actions.size();
    for (const auto& action : m_actions)
    {
        loki::hash_combine(creating_actions_hash, action.hash());
    }
    return loki::hash_combine(m_source_state, m_target_state, creating_actions_hash);
}

TransitionIndex AbstractTransition::get_index() const { return m_index; }

StateIndex AbstractTransition::get_source() const { return m_source_state; }

StateIndex AbstractTransition::get_target() const { return m_target_state; }

TransitionCost AbstractTransition::get_cost() const
{
    auto cost = std::numeric_limits<double>::max();

    std::for_each(m_actions.begin(), m_actions.end(), [&cost](const auto& action) { cost = std::min(cost, (double) action.get_cost()); });

    return cost;
}

std::span<const GroundAction> AbstractTransition::get_actions() const { return m_actions; }

GroundAction AbstractTransition::get_representative_action() const
{
    assert(!m_actions.empty());
    return m_actions.front();
}

}
