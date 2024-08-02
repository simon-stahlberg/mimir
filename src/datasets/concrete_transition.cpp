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

#include "mimir/datasets/concrete_transition.hpp"

namespace mimir
{

ConcreteTransition::ConcreteTransition(TransitionIndex index, StateIndex source_state, StateIndex target_state, GroundAction creating_action) :
    m_index(index),
    m_source_state(source_state),
    m_target_state(target_state),
    m_creating_action(creating_action)
{
}

bool ConcreteTransition::operator==(const ConcreteTransition& other) const
{
    if (this != &other)
    {
        return (m_source_state == other.m_source_state) && (m_target_state == other.m_target_state) && (m_creating_action == other.m_creating_action);
    }
    return true;
}

size_t ConcreteTransition::hash() const { return loki::hash_combine(m_source_state, m_target_state, m_creating_action.hash()); }

TransitionIndex ConcreteTransition::get_index() const { return m_index; }

StateIndex ConcreteTransition::get_source() const { return m_source_state; }

StateIndex ConcreteTransition::get_target() const { return m_target_state; }

TransitionCost ConcreteTransition::get_cost() const { return m_creating_action.get_cost(); }

GroundAction ConcreteTransition::get_creating_action() const { return m_creating_action; }

}
