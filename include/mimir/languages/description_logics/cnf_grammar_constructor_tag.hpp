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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_CONSTRUCTORS_TAG_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_CONSTRUCTORS_TAG_HPP_

#include <concepts>
#include <cstddef>

namespace mimir::dl::cnf_grammar
{

struct Undefined
{
};

/// @brief `Primitive` only have to be generated in the first step
struct Primitive
{
};

/// @brief `Composite` are refined in all subsequent steps
struct Composite
{
};

template<typename T>
concept PrimitiveOrComposite = std::is_same<T, Primitive>::value || std::is_same<T, Composite>::value;

struct Undefined
{
};

/// @brief `Unary` have exactly one non-terminal, i.e.,
/// they have exactly one non-terminal as a child.
/// Hence, when generating sentences with syntactic complexity level i,
/// they must compose with elements of the non-terminal with syntactic complexity i-1.
struct Unary
{
    static const size_t value = 1;
};

struct Binary
{
    static const size_t value = 2;
};

struct Ternary
{
    static const size_t value = 3;
};

template<typename T>
concept Arity = std::is_same<T, Undefined>::value || std::is_same<T, Unary>::value || std::is_same<T, Binary>::value || std::is_same<T, Ternary>::value;

struct Yes
{
};

struct No
{
};

template<typename T>
concept Commutativity = std::is_same<T, Undefined>::value || std::is_same<T, Yes>::value || std::is_same<T, No>::value;

}

#endif
