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

#ifndef MIMIR_ALGORITHMS_MEMORY_POOL_HPP_
#define MIMIR_ALGORITHMS_MEMORY_POOL_HPP_

#include <cassert>
#include <memory>
#include <stack>
#include <vector>

namespace mimir
{
template<typename T>
class MemoryPool;

template<typename T>
class MemoryPoolPointer
{
private:
    MemoryPool<T>* m_pool;
    T* m_object;

public:
    MemoryPoolPointer(MemoryPool<T>* pool, T* object) : m_pool(pool), m_object(object) { assert(pool && object); }

    ~MemoryPoolPointer()
    {
        if (m_pool && m_object)
        {
            m_pool->free(m_object);
        }
        m_object = nullptr;
    }

    T& operator*() const
    {
        assert(m_object);
        return *m_object;
    }

    T* operator->() const
    {
        assert(m_object);
        return m_object;
    }
};

template<typename T>
class MemoryPool
{
private:
    // TODO: maybe we want a segmented vector here in the future
    // to store several objects persistently in 1 segment.
    std::vector<std::unique_ptr<T>> m_storage;
    std::stack<T*> m_stack;

    void allocate()
    {
        m_storage.push_back(std::make_unique<T>(T {}));
        m_stack.push(m_storage.back().get());
    }

    void free(T* element) { m_stack.push(element); }

    template<typename>
    friend class MemoryPoolPointer;

public:
    [[nodiscard]] MemoryPoolPointer<T> get_or_allocate()
    {
        if (m_stack.empty())
        {
            allocate();
        }
        T* element = m_stack.top();
        m_stack.pop();
        return MemoryPoolPointer<T>(this, element);
    }

    [[nodiscard]] size_t get_size() const { return m_storage.size(); }

    [[nodiscard]] size_t get_num_free() const { return m_stack.size(); }
};

}

#endif  // MIMIR_ALLOCATORS_MEMORY_POOL_HPP_
