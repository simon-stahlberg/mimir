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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_COMMON_ITERTOOLS_HPP_
#define MIMIR_COMMON_ITERTOOLS_HPP_

#include <unordered_set>
#include <vector>

namespace mimir
{

template<typename T>
class CartesianProduct
{
private:
    const std::vector<std::vector<T>>& m_vectors;

public:
    explicit CartesianProduct(const std::vector<std::vector<T>>& vectors) : m_vectors(vectors) {}

    /**
     * Iterators
     */
    class const_iterator
    {
    private:
        const std::vector<std::vector<T>>& m_vectors;
        typename std::vector<typename std::vector<T>::const_iterator> m_current;
        bool m_is_end;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::vector<typename std::vector<T>::const_iterator>;
        using pointer = const value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator(const std::vector<std::vector<T>>& vectors, bool begin) : m_vectors(vectors), m_is_end(false)
        {
            if (vectors.empty() || !begin)
            {
                m_is_end = true;
                return;
            }

            for (auto& vec : vectors)
            {
                if (vec.empty())
                {
                    m_is_end = true;
                    return;
                }
                m_current.push_back(vec.begin());
            }
        }

        [[nodiscard]] reference operator*() const { return m_current; }

        const_iterator& operator++()
        {
            for (int i = m_current.size() - 1; i >= 0; --i)
            {
                ++m_current[i];
                if (m_current[i] != m_vectors[i].end())
                {
                    return *this;
                }
                if (i == 0)
                {  // if we increment the first vector and it's at end, we are at the end
                    m_is_end = true;
                    break;
                }
                m_current[i] = m_vectors[i].begin();  // Reset and increment the next iterator left
            }
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        [[nodiscard]] bool operator==(const const_iterator& other) const
        {
            if (m_is_end && other.m_is_end)
                return true;
            if (m_is_end != other.m_is_end)
                return false;
            return m_current == other.m_current;
        }

        [[nodiscard]] bool operator!=(const const_iterator& other) const { return !(*this == other); }
    };

    [[nodiscard]] const_iterator begin() const { return const_iterator(m_vectors, true); }

    [[nodiscard]] const_iterator end() const { return const_iterator(m_vectors, false); }

    [[nodiscard]] size_t num_combinations() const
    {
        auto result = size_t { 1 };
        for (const auto& vec : m_vectors)
        {
            if (vec.empty())
            {
                return 0;
            }
            result *= vec.size();
        }
        return result;
    }
};

}

#endif
