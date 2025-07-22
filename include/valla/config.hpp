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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VALLA_INCLUDE_CONFIG_HPP_
#define VALLA_INCLUDE_CONFIG_HPP_

// ----------------------------------------------------------------------
// Figure out SSE support
// ----------------------------------------------------------------------
#ifndef VALLA_HAVE_SSE2
#if defined(__SSE2__) || (defined(_MSC_VER) && (defined(_M_X64) || (defined(_M_IX86) && _M_IX86_FP >= 2)))
#define VALLA_HAVE_SSE2 1
#else
#define VALLA_HAVE_SSE2 0
#endif
#endif

#ifndef VALLA_HAVE_SSSE3
#if defined(__SSSE3__) || defined(__AVX2__)
#define VALLA_HAVE_SSSE3 1
#else
#define VALLA_HAVE_SSSE3 0
#endif
#endif

#if VALLA_HAVE_SSSE3 && !VALLA_HAVE_SSE2
#error "Bad configuration!"
#endif

#if VALLA_HAVE_SSE2
#include <emmintrin.h>
#endif

#if VALLA_HAVE_SSSE3
#include <tmmintrin.h>
#endif

#endif
