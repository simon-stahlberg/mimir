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

namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
 */
template<typename T>
struct PriorityQueueTag : public OpenListTag
{
};

/**
 * Implementation class
 */
template<typename T>
class OpenList<OpenListDispatcher<PriorityQueueTag<T>>> : public IOpenList<OpenList<OpenListDispatcher<PriorityQueueTag<T>>>>
{
    // Implement configuration specific functionality.
private:
    std::priority_queue<std::pair<double, T>, std::vector<std::pair<double, T>>, std::greater<std::pair<double, T>>> priority_queue_;

    /* Implement IOpenList interface */
    friend class IOpenList<OpenList<OpenListDispatcher<PriorityQueueTag<T>>>>;

    void insert_impl(double priority, const T& item) { priority_queue_.emplace(priority, item); }

    const T& top_impl() const { return priority_queue_.top().second; }

    void pop_impl() { priority_queue_.pop(); }

    bool empty_impl() const { return priority_queue_.empty(); }

    std::size_t size_impl() const { return priority_queue_.size(); }
};

template<typename T>
using PriorityQueue = OpenList<OpenListDispatcher<PriorityQueueTag<T>>>;

}  // namespace mimir

#endif
