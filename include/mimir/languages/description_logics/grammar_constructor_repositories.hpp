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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONSTRUCTOR_REPOSITORIES_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONSTRUCTOR_REPOSITORIES_HPP_

#include "mimir/languages/description_logics/grammar_constructors_interface.hpp"

#include <loki/loki.hpp>
#include <unordered_set>
#include <vector>

namespace mimir::dl::grammar
{

template<IsConcreteConceptOrRole D>
class ConstructorRepository
{
private:
    // Persistent storage of concrete dl constructors to avoid frequent allocations.
    loki::SegmentedVector<D> m_persistent_vector;

    std::unordered_set<const D*, loki::Hash<D*>, loki::EqualTo<D*>> m_uniqueness;

public:
    template<typename... Args>
    const D& create(Args&&... args)
    {
        const auto& element = m_persistent_vector.emplace_back(std::forward<Args>(args)...);
        const auto* element_ptr = &element;

        auto it = m_uniqueness.find(element_ptr);

        if (it == m_uniquenes.end())
        {
            /* Element is unique! */

            m_uniqueness_set.emplace(element_ptr);
        }
        else
        {
            /* Element is not unique! */

            element_ptr = *it;
            // Remove duplicate from vector
            m_persistent_vector.pop_back();
        }

        // Ensure that indexing matches size of uniqueness set.
        assert(m_uniqueness_set.size() == m_count);

        return element;
    }
};

}

#endif
