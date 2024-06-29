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

#include "mimir/languages/description_logics/constructors_interface.hpp"

#include <unordered_set>
#include <vector>

namespace mimir::dl
{

// TODO: maybe use the persistent factory implementation from loki
template<IsConcreteConceptOrRole D>
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
