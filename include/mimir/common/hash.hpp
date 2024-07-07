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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_COMMON_HASH_HPP_
#define MIMIR_COMMON_HASH_HPP_

#include "mimir/common/concepts.hpp"

#include <loki/loki.hpp>

namespace mimir
{
template<IsHashable T>
struct SharedPtrHash
{
    std::size_t operator()(const std::shared_ptr<T>& ptr) const { return ptr->hash(); }

    std::size_t operator()(const std::shared_ptr<const T>& ptr) const { return ptr->hash(); }
};

template<IsComparable T>
struct SharedPtrEqual
{
    bool operator()(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) const { return *lhs == *rhs; }

    bool operator()(const std::shared_ptr<const T>& lhs, const std::shared_ptr<const T>& rhs) const { return *lhs == *rhs; }
};
}

#endif
