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
class MemoryPoolUniquePtr
{
private:
    MemoryPool<T>* m_pool;
    T* m_object;

private:
    void deallocate() noexcept
    {
        if (m_pool && m_object)
        {
            m_pool->free(m_object);
        }
        m_pool = nullptr;
        m_object = nullptr;
    }

public:
    MemoryPoolUniquePtr(MemoryPool<T>* pool, T* object) : m_pool(pool), m_object(object) { assert(pool && object); }
    // Non-copyable
    MemoryPoolUniquePtr(const MemoryPoolUniquePtr&) = delete;
    MemoryPoolUniquePtr& operator=(const MemoryPoolUniquePtr&) = delete;
    // Movable
    MemoryPoolUniquePtr(MemoryPoolUniquePtr&& other) noexcept : m_pool(other.m_pool), m_object(other.m_object)
    {
        other.m_pool = nullptr;
        other.m_object = nullptr;
    }
    MemoryPoolUniquePtr& operator=(MemoryPoolUniquePtr&& other) noexcept
    {
        if (this != &other)
        {
            deallocate();
            m_pool = other.m_pool;
            m_object = other.m_object;
            other.m_pool = nullptr;
            other.m_object = nullptr;
        }
        return *this;
    }

    ~MemoryPoolUniquePtr() noexcept { deallocate(); }

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

    bool operator()() const noexcept { return m_object != nullptr; }

    T* release() noexcept
    {
        T* temp = m_object;
        m_object = nullptr;
        m_pool = nullptr;
        return temp;
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
        m_storage.push_back(std::make_unique<T>());
        m_stack.push(m_storage.back().get());
    }

    void free(T* element) { m_stack.push(element); }

    template<typename>
    friend class MemoryPoolUniquePtr;

public:
    MemoryPool() : m_storage(), m_stack() {}
    // Non-copyable to prevent dangling memory pool pointers.
    MemoryPool(const MemoryPool& other) = delete;
    MemoryPool& operator=(const MemoryPool& other) = delete;
    MemoryPool(MemoryPool&& other) noexcept = delete;
    MemoryPool& operator=(MemoryPool&& other) noexcept = delete;

    [[nodiscard]] MemoryPoolUniquePtr<T> get_or_allocate()
    {
        if (m_stack.empty())
        {
            allocate();
        }
        T* element = m_stack.top();
        m_stack.pop();
        return MemoryPoolUniquePtr<T>(this, element);
    }

    [[nodiscard]] size_t get_size() const { return m_storage.size(); }

    [[nodiscard]] size_t get_num_free() const { return m_stack.size(); }
};

}

#endif
