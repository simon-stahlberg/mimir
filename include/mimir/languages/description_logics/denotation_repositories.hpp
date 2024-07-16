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

#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/denotations.hpp"

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
    using FlatDenotationBuilderType = typename Denotation<D>::FlatDenotationBuilderType;
    using FlatDenotationType = typename Denotation<D>::FlatDenotationType;
    using FlatDenotationSetType = typename Denotation<D>::FlatDenotationSetType;

    // Store denotations uniquely.
    FlatDenotationSetType m_storage;

    struct Key
    {
        size_t constructor_identifier;
        size_t state_identifier;
    };

    struct KeyHash
    {
        size_t operator()(const Key& key) const;
    };

    struct KeyEqual
    {
        bool operator()(const Key& left, const Key& right) const;
    };

    std::unordered_map<Key, Denotation<D>, KeyHash, KeyEqual> m_cached_dynamic_denotations;

public:
    Denotation<D> insert(size_t constructor_identifier, size_t state_identifier, const DenotationBuilder<D>& denotation);

    std::optional<Denotation<D>> get_if(size_t constructor_identifier, size_t state_identifier) const;
};

}

#endif
