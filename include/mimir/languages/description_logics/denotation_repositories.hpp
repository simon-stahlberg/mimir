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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_DENOTATION_REPOSITORIES_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_DENOTATION_REPOSITORIES_HPP_

#include "mimir/formalism/problem.hpp"
#include "mimir/languages/description_logics/constructors_interface.hpp"
#include "mimir/languages/description_logics/denotations.hpp"
#include "mimir/search/state.hpp"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimir::dl
{

/// @brief DenotationRepository encapsulate logic for obtaining unique denotation views and caching.
template<IsConceptOrRole D>
class DenotationRepository
{
private:
    using FlatDenotationType = typename Denotation<D>::FlatDenotationType;
    using FlatDenotationSetType = typename Denotation<D>::FlatDenotationSetType;

    // Store denotations uniquely.
    FlatDenotationSetType m_storage;

    // Cache for fluent and derived infos
    std::unordered_map<std::tuple<State, const D*>, Denotation<D>> m_cached_dynamic_denotations;
    // Cache for static infos
    std::unordered_map<const D*, Denotation<D>> m_cached_static_denotations;

public:
    auto insert(State state, const D* constructor, const DenotationBuilder<D>& denotation)
    {
        const auto [it, inserted] = m_storage.insert(denotation);
        if (inserted)
        {
            m_cached_dynamic_denotations.emplace(std::make_tuple(state, constructor), *it);
        }
        return *it;
    }

    auto insert(const D* constructor, const DenotationBuilder<D>& denotation)
    {
        const auto [it, inserted] = m_storage.insert(denotation);
        if (inserted)
        {
            m_cached_static_denotations.emplace(constructor, *it);
        }
        return *it;
    }

    std::optional<Denotation<D>> get_if(State state, const D* constructor) const
    {
        auto it = m_cached_dynamic_denotations.find(std::make_tuple(state, constructor));
        if (it == m_cached_dynamic_denotations.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    std::optional<Denotation<D>> get_if(const D* constructor) const
    {
        auto it = m_cached_dynamic_denotations.find(constructor);
        if (it == m_cached_dynamic_denotations.end())
        {
            return std::nullopt;
        }
        return it->second;
    }
};

}

#endif
