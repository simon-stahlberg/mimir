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

#ifndef MIMIR_SEARCH_OPENLISTS_ALTERNATING_HPP_
#define MIMIR_SEARCH_OPENLISTS_ALTERNATING_HPP_

#include "mimir/common/tuple.hpp"
#include "mimir/search/openlists/interface.hpp"
#include "mimir/search/openlists/priority_queue.hpp"

#include <array>
#include <cassert>
#include <iostream>
#include <queue>

namespace mimir::search
{

template<IsOpenList... Os>
class AlternatingOpenList
{
public:
    static constexpr std::size_t N = sizeof...(Os);

    static_assert(N > 0);

    using ItemType = typename std::tuple_element<0, std::tuple<Os...>>::type::ItemType;

private:
    bool cur_empty()
    {
        assert(m_pos < N);

        auto result = bool();
        visit_at(m_queues, m_pos, [&result](auto&& queue) { result = queue.get().empty(); });

        return result;
    }

    size_t cur_size()
    {
        assert(m_pos < N);

        auto result = size();
        visit_at(m_queues, m_pos, [&result](auto&& queue) { result = queue.get().size(); });

        return result;
    }

    size_t cur_weight() const
    {
        assert(m_pos < N);

        return m_weights[m_pos];
    }

    void find_next_nonempty_queue()
    {
        do
        {
            if (++m_pos == N)
            {
                m_pos = 0;
            }
        } while (cur_empty());

        m_count = 0;
    }

public:
    AlternatingOpenList(Os&... queues, std::array<size_t, N> weights) :
        m_queues(std::tuple<std::reference_wrapper<Os>...>(std::ref(queues)...)),
        m_weights(weights),
        m_pos(0),
        m_count(0)
    {
    }

    const ItemType& top()
    {
        assert(!empty());

        if (cur_empty() || m_count >= cur_weight())
        {
            find_next_nonempty_queue();
        }

        const ItemType* result = nullptr;
        visit_at(m_queues, m_pos, [&result](auto&& queue) { result = &queue.get().top(); });
        assert(result);
        return *result;
    }

    void pop()
    {
        assert(!cur_empty());

        visit_at(m_queues, m_pos, [](auto&& queue) { queue.get().pop(); });

        ++m_count;
    }

    void clear()
    {
        std::apply([](auto&&... queues) { return (queues.get().clear() && ...); }, m_queues);
    }

    bool empty() const
    {
        return std::apply([](auto&&... queues) { return (queues.get().empty() && ...); }, m_queues);
    }

    std::size_t size() const
    {
        return std::apply([](auto&&... queues) { return (queues.get().size() + ...); }, m_queues);
    }

private:
    std::tuple<std::reference_wrapper<Os>...> m_queues;

    std::array<size_t, N> m_weights;

    size_t m_pos;
    size_t m_count;
};

static_assert((IsOpenListComposition<AlternatingOpenList<PriorityQueue<int, int>, PriorityQueue<double, int>>>) );
}

#endif
