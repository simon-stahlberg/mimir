/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_FORMALISM_CONCEPTS_HPP_
#define MIMIR_FORMALISM_CONCEPTS_HPP_

// Do not include headers with transitive dependencies.

#include "mimir/formalism/declarations.hpp"

#include <concepts>

namespace mimir::formalism
{

template<typename T>
concept IsFormalismEntity = requires { typename std::decay_t<std::remove_pointer_t<T>>::FormalismEntity; };

template<typename T>
concept HasConjunctiveCondition = requires(const T a) {
    { a.get_conjunctive_condition() } -> std::same_as<GroundConjunctiveCondition>;
};

}

#endif
