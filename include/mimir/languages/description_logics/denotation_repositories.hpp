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
#include "mimir/languages/description_logics/constructor_ids.hpp"
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

    struct Key
    {
        const Constructor<D>* constructor;
        State state;
    };

    struct KeyHash
    {
        size_t operator()(const Key& key) const { return loki::hash_combine(key.constructor, key.state.hash()); }
    };

    struct KeyEqual
    {
        bool operator()(const Key& left, const Key& right) const
        {
            if (&left != &right)
            {
                return (left.constructor == right.constructor) && (left.state == right.state);
            }
            return true;
        }
    };

    // Cache for fluent and derived infos
    std::unordered_map<Key, Denotation<D>, KeyHash, KeyEqual> m_cached_dynamic_denotations;
    // Cache for static infos
    std::unordered_map<const Constructor<D>*, Denotation<D>> m_cached_static_denotations;

public:
    Denotation<D> insert(const Constructor<D>* constructor, State state, const DenotationBuilder<D>& denotation)
    {
        const auto [it, inserted] = m_storage.insert(denotation.get_flatmemory_builder());
        if (inserted)
        {
            m_cached_dynamic_denotations.emplace(Key { constructor, state }, Denotation<D>(*it));
        }
        return Denotation<D>(*it);
    }

    Denotation<D> insert(const Constructor<D>* constructor, const DenotationBuilder<D>& denotation)
    {
        const auto [it, inserted] = m_storage.insert(denotation.get_flatmemory_builder());
        if (inserted)
        {
            m_cached_static_denotations.emplace(constructor, Denotation<D>(*it));
        }
        return Denotation<D>(*it);
    }

    std::optional<Denotation<D>> get_if(const Constructor<D>* constructor, State state) const
    {
        auto it = m_cached_dynamic_denotations.find(Key { constructor, state });
        if (it == m_cached_dynamic_denotations.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    std::optional<Denotation<D>> get_if(const Constructor<D>* constructor) const
    {
        auto it = m_cached_static_denotations.find(constructor);
        if (it == m_cached_static_denotations.end())
        {
            return std::nullopt;
        }
        return it->second;
    }
};

}

#endif
