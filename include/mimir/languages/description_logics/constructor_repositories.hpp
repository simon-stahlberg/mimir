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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_REPOSITORIES_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_REPOSITORIES_HPP_

#include "mimir/languages/description_logics/constructor_interface.hpp"

#include <loki/loki.hpp>
#include <unordered_set>
#include <vector>

namespace mimir::dl
{

template<typename T>
class ConstructorRepository
{
private:
    // Persistent storage of constructors to avoid frequent allocations.
    loki::SegmentedVector<T> m_persistent_vector;

    std::unordered_set<const T*, loki::Hash<T*>, loki::EqualTo<T*>> m_uniqueness;

    size_t m_count = 0;

public:
    ConstructorRepository(size_t elements_per_segment) : m_persistent_vector(loki::SegmentedVector<T>(elements_per_segment)) {}

    template<typename... Args>
    const T& create(Args&&... args)
    {
        const auto index = m_count;
        assert(index == m_persistent_vector.size());

        m_persistent_vector.push_back(T(index, std::forward<Args>(args)...));
        const auto& element = m_persistent_vector.back();
        const auto* element_ptr = &element;

        auto it = m_uniqueness.find(element_ptr);

        if (it == m_uniqueness.end())
        {
            /* Element is unique! */

            m_uniqueness.emplace(element_ptr);
            // Validate the element by increasing the identifier to the next free position
            ++m_count;
        }
        else
        {
            /* Element is not unique! */

            element_ptr = *it;
            // Remove duplicate from vector
            m_persistent_vector.pop_back();
        }

        // Ensure that indexing matches size of uniqueness set.
        assert(m_uniqueness.size() == m_count);

        return element;
    }
};

template<typename... Ts>
class VariadicConstructorRepository
{
private:
    std::tuple<ConstructorRepository<Ts>...> m_repositories;

public:
    VariadicConstructorRepository() : m_repositories(std::make_tuple(ConstructorRepository<Ts>(1000)...)) {}

    template<typename T, typename... Args>
    const T& create(Args&&... args)
    {
        return std::get<ConstructorRepository<T>>(m_repositories).create(std::forward<Args>(args)...);
    }
};

}

#endif
