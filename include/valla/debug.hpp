/*
 * Copyright (C) 2025 Dominik Drexler
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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VALLA_INCLUDE_DEBUG_HPP_
#define VALLA_INCLUDE_DEBUG_HPP_

// Some further references:
// - https://arxiv.org/pdf/2109.04548
// - https://arxiv.org/pdf/cs/0512081
// - https://citeseerx.ist.psu.edu/document?repid=rep1&type=pdf&doi=b7620871f53d759a15cf9e820584b89f984f74c1
// - https://en.wikipedia.org/wiki/Quotient_filter

#ifdef VERBOSE
#define DEBUG_LOG(x) std::cout << x << std::endl;
#else
#define DEBUG_LOG(x)
#endif

#endif