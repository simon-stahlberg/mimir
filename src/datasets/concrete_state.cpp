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

#include "mimir/datasets/concrete_state.hpp"

namespace mimir
{

ConcreteState::ConcreteState(StateIndex index, State state) : BaseVertex<ConcreteState>(index), m_state(state) {}

bool ConcreteState::is_equal_impl(const BaseVertex<ConcreteState>& other) const
{
    if (this != &other)
    {
        const auto& otherDerived = static_cast<const ConcreteState&>(other);
        return (m_state == otherDerived.m_state);
    }
    return true;
}

size_t ConcreteState::hash_impl() const { return loki::hash_combine(m_state.hash()); }

State ConcreteState::get_state() const { return m_state; }

}
