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

#ifndef MIMIR_INCLUDE_ALGORITHMS_UNIQUE_OBJECT_POOL_HPP_
#define MIMIR_INCLUDE_ALGORITHMS_UNIQUE_OBJECT_POOL_HPP_

#include <cassert>
#include <concepts>
#include <memory>
#include <stack>
#include <vector>

namespace mimir
{
template<typename T>
class UniqueObjectPool;

template<typename T>
class UniqueObjectPoolPtr
{
private:
    UniqueObjectPool<T>* m_pool;
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
    UniqueObjectPoolPtr() : UniqueObjectPoolPtr(nullptr, nullptr) {}

    UniqueObjectPoolPtr(UniqueObjectPool<T>* pool, T* object) : m_pool(pool), m_object(object) {}

    UniqueObjectPoolPtr(const UniqueObjectPoolPtr& other) = delete;

    UniqueObjectPoolPtr& operator=(const UniqueObjectPoolPtr& other) = delete;

    // Movable
    UniqueObjectPoolPtr(UniqueObjectPoolPtr&& other) noexcept : m_pool(other.m_pool), m_object(other.m_object)
    {
        other.m_pool = nullptr;
        other.m_object = nullptr;
    }

    UniqueObjectPoolPtr& operator=(UniqueObjectPoolPtr&& other) noexcept
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

    UniqueObjectPoolPtr clone() const
    {
        if (m_pool && m_object)
        {
            UniqueObjectPoolPtr pointer = m_pool->get_or_allocate();
            copy(this->operator*(), *pointer);
            return pointer;
        }
        else
        {
            return UniqueObjectPoolPtr();
        }
    }

    ~UniqueObjectPoolPtr()
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

    explicit operator bool() const noexcept { return m_object != nullptr; }
};

template<typename T>
class UniqueObjectPool
{
private:
    std::vector<std::unique_ptr<T>> m_storage;
    std::stack<T*> m_stack;

    template<typename... Args>
    void allocate(Args&&... args)
    {
        m_storage.push_back(std::make_unique<T>(T(std::forward<Args>(args)...)));
        m_stack.push(m_storage.back().get());
    }

    void free(T* element) { m_stack.push(element); }

    friend class UniqueObjectPoolPtr<T>;

public:
    // Non-copyable to prevent dangling memory pool pointers.
    UniqueObjectPool() = default;
    UniqueObjectPool(const UniqueObjectPool& other) = delete;
    UniqueObjectPool& operator=(const UniqueObjectPool& other) = delete;
    UniqueObjectPool(UniqueObjectPool&& other) = delete;
    UniqueObjectPool& operator=(UniqueObjectPool&& other) = delete;

    [[nodiscard]] UniqueObjectPoolPtr<T> get_or_allocate() { return get_or_allocate<>(); }

    template<typename... Args>
    [[nodiscard]] UniqueObjectPoolPtr<T> get_or_allocate(Args&&... args)
    {
        if (m_stack.empty())
        {
            allocate(std::forward<Args>(args)...);
        }
        T* element = m_stack.top();
        m_stack.pop();
        return UniqueObjectPoolPtr<T>(this, element);
    }

    [[nodiscard]] size_t get_size() const { return m_storage.size(); }

    [[nodiscard]] size_t get_num_free() const { return m_stack.size(); }
};

}

#endif