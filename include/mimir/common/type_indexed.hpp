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
 * along with this program. If not, see <httIs://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_COMMON_TYPE_INDEXED_HPP_
#define MIMIR_COMMON_TYPE_INDEXED_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/common/type_indices.hpp"

#include <boost/hana.hpp>
#include <vector>

namespace mimir
{
/// @brief For each type index in L, create a T.
/// @tparam T
/// @tparam L
template<typename T, typename L>
    requires IsTypeIndices<L>
class TypeIndexed;

template<typename T, typename... Is>
class TypeIndexed<T, TypeIndices<Is...>>
{
private:
    boost::hana::map<boost::hana::pair<boost::hana::type<Is>, T>...> m_data;

public:
    template<typename I>
        requires InTypes<I, Is...>
    T& get()
    {
        return boost::hana::at_key(m_data, boost::hana::type<I> {});
    }

    template<typename I>
        requires InTypes<I, Is...>
    const T& get() const
    {
        return boost::hana::at_key(m_data, boost::hana::type<I> {});
    }
};

}

#endif
