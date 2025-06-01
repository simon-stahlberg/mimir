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

#include "mimir/buffering/unordered_set.h"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/denotations.hpp"
#include "mimir/search/declarations.hpp"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimir::languages::dl
{

/// @brief DenotationRepository encapsulate logic for storing and retrieving unique views onto denotations.
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
class DenotationRepository
{
private:
    // Store denotations uniquely.
    DenotationImplSet<D> m_storage;

    using Key = std::pair<Constructor<D>, search::State>;

    std::unordered_map<Key, Denotation<D>, loki::Hash<Key>, loki::EqualTo<Key>> m_cached_dynamic_denotations;

public:
    /// @brief Uniquely insert the denotation for the constructor and state in the repository.
    /// @param constructor is the constructor.
    /// @param state is the state.
    /// @param denotation is the denotation.
    /// @return the unique denotation for the constructor and state.
    Denotation<D> insert(Constructor<D> constructor, search::State state, const DenotationImpl<D>& denotation);

    /// @brief Get the denotation for the constructor and state if it exists, and otherwise, return nullptr.
    /// @param constructor is the constructor.
    /// @param state is the state.
    /// @return the denotation if it exists, and otherwise, return nullptr.
    Denotation<D> get_if(Constructor<D> constructor, search::State state) const;

    /// @brief Clear the repository. Does not quarantee to free memory.
    void clear();
};

using DenotationRepositories = HanaMappedContainer<DenotationRepository, ConceptTag, RoleTag, BooleanTag, NumericalTag>;

/// @brief Clear all DenotationRepository.
/// @param repositories is the container of all denotation repositories.
extern void clear(DenotationRepositories& repositories);

}

#endif
