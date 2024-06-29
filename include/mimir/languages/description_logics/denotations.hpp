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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_DENOTATIONS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_DENOTATIONS_HPP_

#include "mimir/formalism/problem.hpp"
#include "mimir/languages/description_logics/constructors_interface.hpp"
#include "mimir/search/state.hpp"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimir::dl
{
template<IsConceptOrRole D>
class DenotationBuilder
{
private:
    // TODO: add flatmemory::builder to avoid memory allocations
public:
};

template<IsConceptOrRole D>
class DenotationConstView
{
private:
    // TODO: add flatmemory::const_view to avoid memory allocations
public:
};

}

#endif
