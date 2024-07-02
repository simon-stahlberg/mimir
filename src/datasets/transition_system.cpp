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

#include "mimir/datasets/transition_system.hpp"

namespace mimir
{

/**
 * Transition
 */

Transition::Transition(StateIndex forward_successor, StateIndex backward_successor, GroundAction creating_action) :
    m_forward_successor(forward_successor),
    m_backward_successor(backward_successor),
    m_creating_action(creating_action)
{
}

bool Transition::operator==(const Transition& other) const
{
    if (this != &other)
    {
        return (m_forward_successor == other.m_forward_successor) && (m_backward_successor == other.m_backward_successor)
               && (m_creating_action == other.m_creating_action);
    }
    return true;
}

size_t Transition::hash() const { return loki::hash_combine(m_forward_successor, m_backward_successor, m_creating_action.hash()); }

GroundAction Transition::get_creating_action() const { return m_creating_action; }

}
