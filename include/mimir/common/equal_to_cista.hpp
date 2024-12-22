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

#ifndef MIMIR_COMMON_EQUAL_TO_CISTA_HPP_
#define MIMIR_COMMON_EQUAL_TO_CISTA_HPP_

#include "cista/containers/dynamic_bitset.h"
#include "cista/containers/flexible_index_vector.h"
#include "cista/containers/tuple.h"
#include "cista/containers/vector.h"

#include <loki/details/utils/observer_ptr.hpp>

/* DynamicBitset */
template<typename Block, template<typename> typename Ptr>
struct std::equal_to<loki::ObserverPtr<const cista::basic_dynamic_bitset<Block, Ptr>>>
{
    bool operator()(loki::ObserverPtr<const cista::basic_dynamic_bitset<Block, Ptr>> lhs,
                    loki::ObserverPtr<const cista::basic_dynamic_bitset<Block, Ptr>> rhs) const
    {
        return *lhs == *rhs;
    }
};

/* Tuple */
template<typename... Ts>
struct std::equal_to<loki::ObserverPtr<const cista::tuple<Ts...>>>
{
    bool operator()(loki::ObserverPtr<const cista::tuple<Ts...>> lhs, loki::ObserverPtr<const cista::tuple<Ts...>> rhs) const { return *lhs == *rhs; }
};

/* Vector */
template<typename T, template<typename> typename Ptr, bool IndexPointers, typename TemplateSizeType, typename Allocator>
struct std::equal_to<loki::ObserverPtr<const cista::basic_vector<T, Ptr, IndexPointers, TemplateSizeType, Allocator>>>
{
    bool operator()(loki::ObserverPtr<const cista::basic_vector<T, Ptr, IndexPointers, TemplateSizeType, Allocator>> lhs,
                    loki::ObserverPtr<const cista::basic_vector<T, Ptr, IndexPointers, TemplateSizeType, Allocator>> rhs) const
    {
        return *lhs == *rhs;
    }
};

/* FlexibleIndexVector */
template<std::unsigned_integral IndexType, template<typename> typename Ptr>
struct std::equal_to<loki::ObserverPtr<const cista::basic_flexible_index_vector<IndexType, Ptr>>>
{
    bool operator()(loki::ObserverPtr<const cista::basic_flexible_index_vector<IndexType, Ptr>> lhs,
                    loki::ObserverPtr<const cista::basic_flexible_index_vector<IndexType, Ptr>> rhs) const
    {
        return *lhs == *rhs;
    }
};
#endif
