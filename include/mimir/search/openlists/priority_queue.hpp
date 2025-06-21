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

#ifndef MIMIR_SEARCH_OPENLISTS_PRIORITY_QUEUE_HPP_
#define MIMIR_SEARCH_OPENLISTS_PRIORITY_QUEUE_HPP_

#include "mimir/search/openlists/interface.hpp"

#include <cassert>
#include <queue>

namespace mimir::search
{

template<typename T>
concept IsPriorityQueueEntry = requires(const T a) {
    typename T::KeyType;
    typename T::ItemType;
    { a.get_key() } -> std::totally_ordered;
    { a.get_item() };
};

template<IsPriorityQueueEntry E>
class PriorityQueue
{
private:
    struct EntryComparator
    {
        bool operator()(const E& l, const E& r) { return l.get_key() > r.get_key(); }
    };

public:
    using EntryType = E;
    using KeyType = typename E::KeyType;
    using ItemType = typename E::ItemType;

    void insert(E entry) { m_priority_queue.push(std::move(entry)); }

    decltype(auto) top() const
    {
        assert(!empty());
        return m_priority_queue.top().get_item();
    }

    const auto& top_entry() const
    {
        assert(!empty());
        return m_priority_queue.top();
    }

    void pop()
    {
        assert(!empty());
        m_priority_queue.pop();
    }

    void clear()
    {
        auto tmp = std::priority_queue<E, std::vector<E>, EntryComparator> {};
        std::swap(m_priority_queue, tmp);
    }

    bool empty() const { return m_priority_queue.empty(); }

    std::size_t size() const { return m_priority_queue.size(); }

private:
    std::priority_queue<E, std::vector<E>, EntryComparator> m_priority_queue;
};

}

#endif
