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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_REPOSITORIES_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_REPOSITORIES_HPP_

#include "mimir/formalism/problem.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/search/state.hpp"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimir::dl
{
template<IsDLConstructor D>
class DenotationBuilder
{
private:
    // TODO: add flatmemory::builder to avoid memory allocations
public:
};

template<IsDLConstructor D>
class DenotationConstView
{
private:
    // TODO: add flatmemory::const_view to avoid memory allocations
public:
};

template<IsDLConstructor D>
class DenotationRepository
{
private:
    // TODO: use flatmemory::unordered_set

    std::unordered_map<std::pair<State, const D*>, DenotationConstView<D>> m_cached_denotations;

public:
    DenotationConstView<D> insert(DenotationBuilder<D> denotation);

    std::optional<DenotationConstView<D>> get_if(Problem problem, State state, const D* constructor) const;
};

template<IsDLConstructor D>
class ConstructorRepository
{
private:
    // Persistent storage of concrete dl constructors to avoid frequent allocations.
    std::vector<std::vector<D>> m_storage;

    std::unordered_set<D> m_uniqueness;

public:
    /// @brief Insert the dl constructor and return a pointer to the uniquely existing dl constructor.
    /// @param constructor the dl constructor to be created uniquely.
    /// @return
    const D* register_uniquely(D constructor);
};

}

#endif
