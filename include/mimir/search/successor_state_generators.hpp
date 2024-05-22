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

#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_HPP_

/**
 * Include all specializations here
 */

#if defined(STATE_REPR_DENSE)

#include "mimir/search/successor_state_generators/dense.hpp"

namespace mimir
{
using SuccessorStateGenerator = DenseSSG;
}

#else
#error "State representation undefined."
#endif

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_HPP_
