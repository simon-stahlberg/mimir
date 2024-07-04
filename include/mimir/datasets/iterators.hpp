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

#ifndef MIMIR_DATASETS_ITERATORS_HPP_
#define MIMIR_DATASETS_ITERATORS_HPP_

#include "mimir/datasets/transition_interface.hpp"
#include "mimir/search/state.hpp"

#include <concepts>
#include <iterator>
#include <span>
#include <type_traits>

namespace mimir
{

template<IsTransition T>
class TargetStateIterator
{
private:
    std::span<const T> m_transitions;

public:
    using value_type = T;

    TargetStateIterator(std::span<const T> transitions);

    class const_iterator
    {
    private:
        size_t m_pos;
        std::span<const T> m_transitions;

        void advance();

    public:
        using difference_type = int;
        using value_type = StateIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(std::span<const T> transitions, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
};

template<IsTransition T>
class SourceStateIterator
{
};

template<IsTransition T>
class ForwardTransitionIterator
{
};

template<IsTransition T>
class BackwardTransitionIterator
{
};

/**
 * Implementations
 */

/* TargetStateIterator */

template<IsTransition T>
TargetStateIterator<T>::TargetStateIterator(std::span<const T> transitions) : m_transitions(transitions)
{
}

template<IsTransition T>
void TargetStateIterator<T>::const_iterator::advance()
{
    ++m_pos;
}

template<IsTransition T>
TargetStateIterator<T>::const_iterator::const_iterator()
{
}

template<IsTransition T>
TargetStateIterator<T>::const_iterator::const_iterator(std::span<const T> transitions, bool begin) :
    m_pos(begin ? 0 : transitions.size()),
    m_transitions(transitions)
{
}

template<IsTransition T>
TargetStateIterator<T>::const_iterator::value_type TargetStateIterator<T>::const_iterator::operator*() const
{
    assert(m_pos < m_transitions.size());
    return m_transitions[m_pos].get_target_state();
}

template<IsTransition T>
TargetStateIterator<T>::const_iterator& TargetStateIterator<T>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsTransition T>
TargetStateIterator<T>::const_iterator TargetStateIterator<T>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsTransition T>
bool TargetStateIterator<T>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsTransition T>
bool TargetStateIterator<T>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsTransition T>
TargetStateIterator<T>::const_iterator TargetStateIterator<T>::begin() const
{
    return const_iterator(m_transitions, true);
}

template<IsTransition T>
TargetStateIterator<T>::const_iterator TargetStateIterator<T>::end() const
{
    return const_iterator(m_transitions, false);
}

}

#endif
