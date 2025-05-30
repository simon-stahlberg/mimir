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

template<std::totally_ordered Key, typename Item>
class PriorityQueue
{
private:
    struct Entry
    {
        Key priority;
        Item item;

        Entry(Key priority, Item item) : priority(std::move(priority)), item(std::move(item)) {}
    };

    struct EntryComparator
    {
        bool operator()(const Entry& l, const Entry& r) { return l.priority > r.priority; }
    };

public:
    using KeyType = Key;
    using ItemType = Item;

    void insert(Key priority, Item item) { m_priority_queue.emplace(std::move(priority), std::move(item)); }

    const Item& top() const
    {
        assert(!empty());
        return m_priority_queue.top().item;
    }

    void pop()
    {
        assert(!empty());
        m_priority_queue.pop();
    }

    void clear()
    {
        auto tmp = std::priority_queue<Entry, std::vector<Entry>, EntryComparator> {};
        std::swap(m_priority_queue, tmp);
    }

    bool empty() const { return m_priority_queue.empty(); }

    std::size_t size() const { return m_priority_queue.size(); }

private:
    std::priority_queue<Entry, std::vector<Entry>, EntryComparator> m_priority_queue;
};

static_assert((IsOpenList<PriorityQueue<int, int>>) );

}

#endif
