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

#ifndef MIMIR_DATASETS_DECLARATIONS_HPP_
#define MIMIR_DATASETS_DECLARATIONS_HPP_

// Do not include headers with transitive dependencies.
#include "mimir/common/types.hpp"
#include "mimir/graphs/declarations.hpp"

#include <cstdint>
#include <vector>

namespace mimir
{

struct StateSpaceOptions;
struct StateSpacesOptions;
class StateSpace;

struct FaithfulAbstractionOptions;
struct FaithfulAbstractionsOptions;
class FaithfulAbstraction;
class GlobalFaithfulAbstraction;

}

#endif
