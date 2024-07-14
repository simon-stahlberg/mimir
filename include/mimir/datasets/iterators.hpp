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
#include "mimir/datasets/transitions.hpp"
#include "mimir/search/state.hpp"

#include <concepts>
#include <iterator>
#include <ranges>
#include <span>
#include <type_traits>

namespace mimir
{

template<IsTransition T>
class TargetStateIndexIterator
{
private:
    std::span<const T> m_transitions;

public:
    using value_type = T;

    TargetStateIndexIterator(std::span<const T> transitions);

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
        value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

static_assert(std::ranges::forward_range<TargetStateIndexIterator<Transition>>);
static_assert(std::ranges::forward_range<TargetStateIndexIterator<AbstractTransition>>);

template<IsTransition T>
class SourceStateIndexIterator
{
private:
    StateIndex m_target;
    std::span<const T> m_transitions;

public:
    using value_type = T;

    SourceStateIndexIterator(StateIndex target, std::span<const T> transitions);

    class const_iterator
    {
    private:
        StateIndex m_target;
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
        const_iterator(StateIndex target, std::span<const T> transitions, bool begin);
        value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

static_assert(std::ranges::forward_range<SourceStateIndexIterator<Transition>>);
static_assert(std::ranges::forward_range<SourceStateIndexIterator<AbstractTransition>>);

template<IsTransition T>
class ForwardTransitionIndexIterator
{
private:
    std::span<const T> m_transitions;

public:
    using value_type = T;

    ForwardTransitionIndexIterator(std::span<const T> transitions);

    class const_iterator
    {
    private:
        size_t m_pos;
        std::span<const T> m_transitions;

        void advance();

    public:
        using difference_type = int;
        using value_type = TransitionIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(std::span<const T> transitions, bool begin);
        value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

static_assert(std::ranges::forward_range<ForwardTransitionIndexIterator<Transition>>);
static_assert(std::ranges::forward_range<ForwardTransitionIndexIterator<AbstractTransition>>);

template<IsTransition T>
class BackwardTransitionIndexIterator
{
private:
    StateIndex m_target;
    std::span<const T> m_transitions;

public:
    using value_type = T;

    BackwardTransitionIndexIterator(StateIndex target, std::span<const T> transitions);

    class const_iterator
    {
    private:
        StateIndex m_target;
        size_t m_pos;
        std::span<const T> m_transitions;

        void advance();

    public:
        using difference_type = int;
        using value_type = TransitionIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(StateIndex target, std::span<const T> transitions, bool begin);
        value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

static_assert(std::ranges::forward_range<BackwardTransitionIndexIterator<Transition>>);
static_assert(std::ranges::forward_range<BackwardTransitionIndexIterator<AbstractTransition>>);

template<IsTransition T>
class ForwardTransitionIterator
{
private:
    std::span<const T> m_transitions;

public:
    using value_type = T;

    ForwardTransitionIterator(std::span<const T> transitions);

    class const_iterator
    {
    private:
        size_t m_pos;
        std::span<const T> m_transitions;

        void advance();

    public:
        using difference_type = int;
        using value_type = const T;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(std::span<const T> transitions, bool begin);
        reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

static_assert(std::ranges::forward_range<ForwardTransitionIterator<Transition>>);
static_assert(std::ranges::forward_range<ForwardTransitionIterator<AbstractTransition>>);

template<IsTransition T>
class BackwardTransitionIterator
{
private:
    StateIndex m_target;
    std::span<const T> m_transitions;

public:
    using value_type = T;

    BackwardTransitionIterator(StateIndex target, std::span<const T> transitions);

    class const_iterator
    {
    private:
        StateIndex m_target;
        size_t m_pos;
        std::span<const T> m_transitions;

        void advance();

    public:
        using difference_type = int;
        using value_type = const T;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(StateIndex target, std::span<const T> transitions, bool begin);
        reference operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

static_assert(std::ranges::forward_range<BackwardTransitionIterator<Transition>>);
static_assert(std::ranges::forward_range<BackwardTransitionIterator<AbstractTransition>>);

/**
 * Implementations
 */

/* TargetStateIndexIterator */

template<IsTransition T>
TargetStateIndexIterator<T>::TargetStateIndexIterator(std::span<const T> transitions) : m_transitions(transitions)
{
}

template<IsTransition T>
void TargetStateIndexIterator<T>::const_iterator::advance()
{
    ++m_pos;
}

template<IsTransition T>
TargetStateIndexIterator<T>::const_iterator::const_iterator()
{
}

template<IsTransition T>
TargetStateIndexIterator<T>::const_iterator::const_iterator(std::span<const T> transitions, bool begin) :
    m_pos(begin ? 0 : transitions.size()),
    m_transitions(transitions)
{
}

template<IsTransition T>
TargetStateIndexIterator<T>::const_iterator::value_type TargetStateIndexIterator<T>::const_iterator::operator*() const
{
    assert(m_pos < m_transitions.size());
    return m_transitions[m_pos].get_target_state();
}

template<IsTransition T>
TargetStateIndexIterator<T>::const_iterator& TargetStateIndexIterator<T>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsTransition T>
TargetStateIndexIterator<T>::const_iterator TargetStateIndexIterator<T>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsTransition T>
bool TargetStateIndexIterator<T>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsTransition T>
bool TargetStateIndexIterator<T>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsTransition T>
TargetStateIndexIterator<T>::const_iterator TargetStateIndexIterator<T>::begin() const
{
    return const_iterator(m_transitions, true);
}

template<IsTransition T>
TargetStateIndexIterator<T>::const_iterator TargetStateIndexIterator<T>::end() const
{
    return const_iterator(m_transitions, false);
}

/* SourceStateIndexIterator */

template<IsTransition T>
SourceStateIndexIterator<T>::SourceStateIndexIterator(StateIndex target, std::span<const T> transitions) : m_target(target), m_transitions(transitions)
{
}

template<IsTransition T>
void SourceStateIndexIterator<T>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_transitions.size() && m_transitions[m_pos].get_target_state() != m_target);
}

template<IsTransition T>
SourceStateIndexIterator<T>::const_iterator::const_iterator()
{
}

template<IsTransition T>
SourceStateIndexIterator<T>::const_iterator::const_iterator(StateIndex target, std::span<const T> transitions, bool begin) :
    m_target(target),
    m_pos(begin ? 0 : transitions.size()),
    m_transitions(transitions)
{
    if (begin && m_transitions.size() > 0 && m_transitions[0].get_target_state() != m_target)
    {
        advance();
    }
}

template<IsTransition T>
SourceStateIndexIterator<T>::const_iterator::value_type SourceStateIndexIterator<T>::const_iterator::operator*() const
{
    assert(m_pos < m_transitions.size());
    return m_transitions[m_pos].get_source_state();
}

template<IsTransition T>
SourceStateIndexIterator<T>::const_iterator& SourceStateIndexIterator<T>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsTransition T>
SourceStateIndexIterator<T>::const_iterator SourceStateIndexIterator<T>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsTransition T>
bool SourceStateIndexIterator<T>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsTransition T>
bool SourceStateIndexIterator<T>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsTransition T>
SourceStateIndexIterator<T>::const_iterator SourceStateIndexIterator<T>::begin() const
{
    return const_iterator(m_target, m_transitions, true);
}

template<IsTransition T>
SourceStateIndexIterator<T>::const_iterator SourceStateIndexIterator<T>::end() const
{
    return const_iterator(m_target, m_transitions, false);
}

/* ForwardTransitionIndexIterator */

template<IsTransition T>
ForwardTransitionIndexIterator<T>::ForwardTransitionIndexIterator(std::span<const T> transitions) : m_transitions(transitions)
{
}

template<IsTransition T>
void ForwardTransitionIndexIterator<T>::const_iterator::advance()
{
    ++m_pos;
}

template<IsTransition T>
ForwardTransitionIndexIterator<T>::const_iterator::const_iterator()
{
}

template<IsTransition T>
ForwardTransitionIndexIterator<T>::const_iterator::const_iterator(std::span<const T> transitions, bool begin) :
    m_pos(begin ? 0 : transitions.size()),
    m_transitions(transitions)
{
}

template<IsTransition T>
ForwardTransitionIndexIterator<T>::const_iterator::value_type ForwardTransitionIndexIterator<T>::const_iterator::operator*() const
{
    assert(m_pos < m_transitions.size());
    return m_transitions[m_pos].get_index();
}

template<IsTransition T>
ForwardTransitionIndexIterator<T>::const_iterator& ForwardTransitionIndexIterator<T>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsTransition T>
ForwardTransitionIndexIterator<T>::const_iterator ForwardTransitionIndexIterator<T>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsTransition T>
bool ForwardTransitionIndexIterator<T>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsTransition T>
bool ForwardTransitionIndexIterator<T>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsTransition T>
ForwardTransitionIndexIterator<T>::const_iterator ForwardTransitionIndexIterator<T>::begin() const
{
    return const_iterator(m_transitions, true);
}

template<IsTransition T>
ForwardTransitionIndexIterator<T>::const_iterator ForwardTransitionIndexIterator<T>::end() const
{
    return const_iterator(m_transitions, false);
}

/* BackwardTransitionIndexIterator */

template<IsTransition T>
BackwardTransitionIndexIterator<T>::BackwardTransitionIndexIterator(StateIndex target, std::span<const T> transitions) :
    m_target(target),
    m_transitions(transitions)
{
}

template<IsTransition T>
void BackwardTransitionIndexIterator<T>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_transitions.size() && m_transitions[m_pos].get_target_state() != m_target);
}

template<IsTransition T>
BackwardTransitionIndexIterator<T>::const_iterator::const_iterator()
{
}

template<IsTransition T>
BackwardTransitionIndexIterator<T>::const_iterator::const_iterator(StateIndex target, std::span<const T> transitions, bool begin) :
    m_target(target),
    m_pos(begin ? 0 : transitions.size()),
    m_transitions(transitions)
{
    if (begin && m_transitions.size() > 0 && m_transitions[0].get_target_state() != m_target)
    {
        advance();
    }
}

template<IsTransition T>
BackwardTransitionIndexIterator<T>::const_iterator::value_type BackwardTransitionIndexIterator<T>::const_iterator::operator*() const
{
    assert(m_pos < m_transitions.size());
    return m_transitions[m_pos].get_index();
}

template<IsTransition T>
BackwardTransitionIndexIterator<T>::const_iterator& BackwardTransitionIndexIterator<T>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsTransition T>
BackwardTransitionIndexIterator<T>::const_iterator BackwardTransitionIndexIterator<T>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsTransition T>
bool BackwardTransitionIndexIterator<T>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsTransition T>
bool BackwardTransitionIndexIterator<T>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsTransition T>
BackwardTransitionIndexIterator<T>::const_iterator BackwardTransitionIndexIterator<T>::begin() const
{
    return const_iterator(m_target, m_transitions, true);
}

template<IsTransition T>
BackwardTransitionIndexIterator<T>::const_iterator BackwardTransitionIndexIterator<T>::end() const
{
    return const_iterator(m_target, m_transitions, false);
}

/* ForwardTransitionIterator */

template<IsTransition T>
ForwardTransitionIterator<T>::ForwardTransitionIterator(std::span<const T> transitions) : m_transitions(transitions)
{
}

template<IsTransition T>
void ForwardTransitionIterator<T>::const_iterator::advance()
{
    ++m_pos;
}

template<IsTransition T>
ForwardTransitionIterator<T>::const_iterator::const_iterator()
{
}

template<IsTransition T>
ForwardTransitionIterator<T>::const_iterator::const_iterator(std::span<const T> transitions, bool begin) :
    m_pos(begin ? 0 : transitions.size()),
    m_transitions(transitions)
{
}

template<IsTransition T>
ForwardTransitionIterator<T>::const_iterator::reference ForwardTransitionIterator<T>::const_iterator::operator*() const
{
    assert(m_pos < m_transitions.size());
    return m_transitions[m_pos];
}

template<IsTransition T>
ForwardTransitionIterator<T>::const_iterator& ForwardTransitionIterator<T>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsTransition T>
ForwardTransitionIterator<T>::const_iterator ForwardTransitionIterator<T>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsTransition T>
bool ForwardTransitionIterator<T>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsTransition T>
bool ForwardTransitionIterator<T>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsTransition T>
ForwardTransitionIterator<T>::const_iterator ForwardTransitionIterator<T>::begin() const
{
    return const_iterator(m_transitions, true);
}

template<IsTransition T>
ForwardTransitionIterator<T>::const_iterator ForwardTransitionIterator<T>::end() const
{
    return const_iterator(m_transitions, false);
}

/* BackwardTransitionIterator */

template<IsTransition T>
BackwardTransitionIterator<T>::BackwardTransitionIterator(StateIndex target, std::span<const T> transitions) : m_target(target), m_transitions(transitions)
{
}

template<IsTransition T>
void BackwardTransitionIterator<T>::const_iterator::advance()
{
    do
    {
        // Always advance 1 before the first loop termination check.
        ++m_pos;
    } while (m_pos < m_transitions.size() && m_transitions[m_pos].get_target_state() != m_target);
}

template<IsTransition T>
BackwardTransitionIterator<T>::const_iterator::const_iterator()
{
}

template<IsTransition T>
BackwardTransitionIterator<T>::const_iterator::const_iterator(StateIndex target, std::span<const T> transitions, bool begin) :
    m_target(target),
    m_pos(begin ? 0 : transitions.size()),
    m_transitions(transitions)
{
    if (begin && m_transitions.size() > 0 && m_transitions[0].get_target_state() != m_target)
    {
        advance();
    }
}

template<IsTransition T>
BackwardTransitionIterator<T>::const_iterator::reference BackwardTransitionIterator<T>::const_iterator::operator*() const
{
    assert(m_pos < m_transitions.size());
    return m_transitions[m_pos];
}

template<IsTransition T>
BackwardTransitionIterator<T>::const_iterator& BackwardTransitionIterator<T>::const_iterator::operator++()
{
    advance();
    return *this;
}

template<IsTransition T>
BackwardTransitionIterator<T>::const_iterator BackwardTransitionIterator<T>::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<IsTransition T>
bool BackwardTransitionIterator<T>::const_iterator::operator==(const const_iterator& other) const
{
    return (m_pos == other.m_pos);
}

template<IsTransition T>
bool BackwardTransitionIterator<T>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<IsTransition T>
BackwardTransitionIterator<T>::const_iterator BackwardTransitionIterator<T>::begin() const
{
    return const_iterator(m_target, m_transitions, true);
}

template<IsTransition T>
BackwardTransitionIterator<T>::const_iterator BackwardTransitionIterator<T>::end() const
{
    return const_iterator(m_target, m_transitions, false);
}

}

#endif
