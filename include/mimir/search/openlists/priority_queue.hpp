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

#include <queue>

namespace mimir::search
{

/**
 * Implementation class
 */
template<typename T>
class PriorityQueue : public IOpenList<PriorityQueue<T>>
{
private:
    struct Entry
    {
        double priority;
        T element;

        Entry(double priority, T element) : priority(priority), element(std::move(element)) {}
    };

    struct EntryComparator
    {
        bool operator()(const Entry& l, const Entry& r) { return l.priority > r.priority; }
    };

    /* Implement IOpenList interface */
    friend class IOpenList<PriorityQueue>;

    void insert_impl(double priority, const T& item) { m_priority_queue.emplace(priority, item); }

    const T& top_impl() const { return m_priority_queue.top().element; }

    void pop_impl() { m_priority_queue.pop(); }

    void clear_impl()
    {
        auto tmp = std::priority_queue<Entry, std::vector<Entry>, EntryComparator> {};
        std::swap(m_priority_queue, tmp);
    }

    bool empty_impl() const { return m_priority_queue.empty(); }

    std::size_t size_impl() const { return m_priority_queue.size(); }

    std::priority_queue<Entry, std::vector<Entry>, EntryComparator> m_priority_queue;
};

}  // namespace mimir

#endif
