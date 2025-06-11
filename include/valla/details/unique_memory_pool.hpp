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

#ifndef VALLA_INCLUDE_DETAILS_UNIQUE_MEMORY_POOL_HPP_
#define VALLA_INCLUDE_DETAILS_UNIQUE_MEMORY_POOL_HPP_

#include <cassert>
#include <concepts>
#include <memory>
#include <stack>
#include <vector>

namespace valla
{
template<typename T>
    requires std::default_initializable<T>
class UniqueMemoryPool;

template<typename T>
    requires std::default_initializable<T>
class UniqueMemoryPoolPtr
{
private:
    UniqueMemoryPool<T>* m_pool;
    T* m_object;

private:
    void deallocate()
    {
        assert(m_pool && m_object);

        m_pool->free(m_object);
        m_pool = nullptr;
        m_object = nullptr;
    }

    T* release() noexcept
    {
        T* temp = m_object;
        m_object = nullptr;
        m_pool = nullptr;
        return temp;
    }

public:
    UniqueMemoryPoolPtr() : UniqueMemoryPoolPtr(nullptr, nullptr) {}

    UniqueMemoryPoolPtr(UniqueMemoryPool<T>* pool, T* object) : m_pool(pool), m_object(object) {}

    /// @brief Copy other into this through an additional allocation from the pool.
    /// Requires a user define copy operation of the data.
    /// @param other
    UniqueMemoryPoolPtr(const UniqueMemoryPoolPtr& other) : m_pool(nullptr), m_object(nullptr)
    {
        if (other.m_pool && other.m_object)
        {
            *this = other.m_pool->get_or_allocate();
            copy(*other.m_object, *m_object);
        }
    }

    /// @brief Assign other into this through an additional allocation from the pool.
    /// Requires a user define copy operation of the data.
    /// @param other
    /// @return
    UniqueMemoryPoolPtr& operator=(const UniqueMemoryPoolPtr& other)
    {
        if (this != &other)
        {
            if (other.m_pool && other.m_object)
            {
                m_pool = other.m_pool;
                if (!m_object)
                {
                    auto pointer = m_pool->get_or_allocate();
                    m_object = pointer.release();
                }
                copy(*other.m_object, *m_object);
            }
        }
        return *this;
    }

    // Movable
    UniqueMemoryPoolPtr(UniqueMemoryPoolPtr&& other) noexcept : m_pool(other.m_pool), m_object(other.m_object)
    {
        other.m_pool = nullptr;
        other.m_object = nullptr;
    }

    UniqueMemoryPoolPtr& operator=(UniqueMemoryPoolPtr&& other) noexcept
    {
        if (this != &other)
        {
            if (m_pool && m_object)
                deallocate();

            m_pool = other.m_pool;
            m_object = other.m_object;
            other.m_pool = nullptr;
            other.m_object = nullptr;
        }
        return *this;
    }

    ~UniqueMemoryPoolPtr()
    {
        if (m_pool && m_object)
            deallocate();
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

    bool operator()() const { return m_object != nullptr; }
};

template<typename T>
    requires std::default_initializable<T>
class UniqueMemoryPool
{
private:
    std::vector<std::unique_ptr<T>> m_storage;
    std::stack<T*> m_stack;

    void allocate()
    {
        m_storage.push_back(std::make_unique<T>(T()));
        m_stack.push(m_storage.back().get());
    }

    void free(T* element) { m_stack.push(element); }

    friend class UniqueMemoryPoolPtr<T>;

public:
    // Non-copyable to prevent dangling memory pool pointers.
    UniqueMemoryPool() = default;
    UniqueMemoryPool(const UniqueMemoryPool& other) = delete;
    UniqueMemoryPool& operator=(const UniqueMemoryPool& other) = delete;
    UniqueMemoryPool(UniqueMemoryPool&& other) = delete;
    UniqueMemoryPool& operator=(UniqueMemoryPool&& other) = delete;

    [[nodiscard]] UniqueMemoryPoolPtr<T> get_or_allocate()
    {
        if (m_stack.empty())
        {
            allocate();
        }
        T* element = m_stack.top();
        m_stack.pop();
        return UniqueMemoryPoolPtr<T>(this, element);
    }

    [[nodiscard]] size_t get_size() const { return m_storage.size(); }

    [[nodiscard]] size_t get_num_free() const { return m_stack.size(); }
};

}

#endif