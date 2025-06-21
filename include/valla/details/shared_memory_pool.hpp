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

#ifndef VALLA_INCLUDE_DETAILS_SHARED_MEMORY_POOL_HPP_
#define VALLA_INCLUDE_DETAILS_SHARED_MEMORY_POOL_HPP_

#include <cassert>
#include <concepts>
#include <memory>
#include <stack>
#include <utility>
#include <vector>

namespace valla
{
template<typename T>
    requires std::default_initializable<T>
class SharedMemoryPool;

template<typename T>
    requires std::default_initializable<T>
class SharedMemoryPoolPtr
{
private:
    SharedMemoryPool<T>* m_pool;
    std::pair<size_t, T>* m_object;

private:
    void deallocate()
    {
        assert(m_pool && m_object);

        m_pool->free(m_object);
        m_pool = nullptr;
        m_object = nullptr;
    }

    std::pair<size_t, T>* release() noexcept
    {
        std::pair<size_t, T>* temp = m_object;
        m_object = nullptr;
        m_pool = nullptr;
        return temp;
    }

    void inc_ref_count()
    {
        assert(m_object);

        ++m_object->first;
    }

    void dec_ref_count()
    {
        assert(m_object);

        --m_object->first;

        if (m_object->first == 0)
        {
            deallocate();
        }
    }

public:
    SharedMemoryPoolPtr() : SharedMemoryPoolPtr(nullptr, nullptr) {}

    SharedMemoryPoolPtr(SharedMemoryPool<T>* pool, std::pair<size_t, T>* object) : m_pool(pool), m_object(object)
    {
        if (m_pool && m_object)
            inc_ref_count();
    }

    /// @brief Copy other into this through an additional allocation from the pool.
    /// Requires a user define copy operation of the data.
    /// @param other
    SharedMemoryPoolPtr(const SharedMemoryPoolPtr& other) : m_pool(nullptr), m_object(nullptr)
    {
        if (other.m_pool && other.m_object)
        {
            m_pool = other.m_pool;
            m_object = other.m_object;

            inc_ref_count();
        }
    }

    /// @brief Assign other into this through an additional allocation from the pool.
    /// Requires a user define copy operation of the data.
    /// @param other
    /// @return
    SharedMemoryPoolPtr& operator=(const SharedMemoryPoolPtr& other)
    {
        if (this != &other)
        {
            if (other.m_pool && other.m_object)
            {
                if (m_pool && m_object)
                    dec_ref_count();

                m_pool = other.m_pool;
                m_object = other.m_object;
                inc_ref_count();
            }
        }
        return *this;
    }

    // Movable
    SharedMemoryPoolPtr(SharedMemoryPoolPtr&& other) noexcept : m_pool(other.m_pool), m_object(other.m_object)
    {
        other.m_pool = nullptr;
        other.m_object = nullptr;
    }

    SharedMemoryPoolPtr& operator=(SharedMemoryPoolPtr&& other) noexcept
    {
        if (this != &other)
        {
            if (m_pool && m_object)
                dec_ref_count();

            m_pool = other.m_pool;
            m_object = other.m_object;

            other.m_pool = nullptr;
            other.m_object = nullptr;
        }
        return *this;
    }

    ~SharedMemoryPoolPtr()
    {
        if (m_pool && m_object)
        {
            dec_ref_count();
        }
    }

    SharedMemoryPoolPtr clone() const
    {
        if (m_pool && m_object)
        {
            SharedMemoryPoolPtr pointer = m_pool->get_or_allocate();
            copy(this->operator*(), *pointer);
            return pointer;
        }
        else
        {
            return SharedMemoryPoolPtr();
        }
    }

    T& operator*() const
    {
        assert(m_object);
        return m_object->second;
    }

    T* operator->() const
    {
        assert(m_object);
        return &m_object->second;
    }

    size_t ref_count() const
    {
        assert(m_object);
        return m_object->first;
    }

    bool operator()() const { return m_object != nullptr; }
};

template<typename T>
    requires std::default_initializable<T>
class SharedMemoryPool
{
private:
    using Entry = std::pair<size_t, T>;

    std::vector<std::unique_ptr<Entry>> m_storage;
    std::stack<Entry*> m_stack;

    void allocate()
    {
        m_storage.push_back(std::make_unique<Entry>(size_t(0), T()));
        m_stack.push(m_storage.back().get());
    }

    void free(Entry* element) { m_stack.push(element); }

    friend class SharedMemoryPoolPtr<T>;

public:
    // Non-copyable to prevent dangling memory pool pointers.
    SharedMemoryPool() = default;
    SharedMemoryPool(const SharedMemoryPool& other) = delete;
    SharedMemoryPool& operator=(const SharedMemoryPool& other) = delete;
    SharedMemoryPool(SharedMemoryPool&& other) = delete;
    SharedMemoryPool& operator=(SharedMemoryPool&& other) = delete;

    [[nodiscard]] SharedMemoryPoolPtr<T> get_or_allocate()
    {
        if (m_stack.empty())
        {
            allocate();
        }
        Entry* element = m_stack.top();
        m_stack.pop();
        return SharedMemoryPoolPtr<T>(this, element);
    }

    [[nodiscard]] size_t get_size() const { return m_storage.size(); }

    [[nodiscard]] size_t get_num_free() const { return m_stack.size(); }
};

}

#endif