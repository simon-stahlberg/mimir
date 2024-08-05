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

ConcreteTransition::ConcreteTransition(TransitionIndex index, StateIndex source, StateIndex target, GroundAction creating_action) :
    BaseEdge<ConcreteTransition>(index, source, target),
    m_creating_action(creating_action)
{
}

bool ConcreteTransition::is_equal_impl(const BaseEdge<ConcreteTransition>& other) const
{
    if (this != &other)
    {
        const auto& otherDerived = static_cast<const ConcreteTransition&>(other);
        return (m_creating_action == otherDerived.m_creating_action);
    }
    return true;
}

size_t ConcreteTransition::hash_impl() const { return loki::hash_combine(m_creating_action.hash()); }

TransitionCost ConcreteTransition::get_cost() const { return m_creating_action.get_cost(); }

GroundAction ConcreteTransition::get_creating_action() const { return m_creating_action; }

}
