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

#ifndef MIMIR_COMMON_TUPLE_HPP_
#define MIMIR_COMMON_TUPLE_HPP_

#include "mimir/common/concepts.hpp"

namespace mimir
{

/**
 * std::get<I> with runtime I.
 */

template<size_t I>
struct visit_impl
{
    template<typename T, typename F>
    static void visit(T& tup, size_t idx, F function)
    {
        if (idx == I - 1)
        {
            function(std::get<I - 1>(tup));
        }
        else
            visit_impl<I - 1>::visit(tup, idx, function);
    }
};

template<>
struct visit_impl<0>
{
    template<typename T, typename F>
    static void visit(T& tup, size_t idx, F function)
    {
        if (idx == 0)
        {
            function(std::get<0>(tup));
        }
    }
};

template<typename F, typename... Ts>
void visit_at(std::tuple<Ts...> const& tup, size_t idx, F function)
{
    return visit_impl<sizeof...(Ts)>::visit(tup, idx, function);
}

template<typename F, typename... Ts>
void visit_at(std::tuple<Ts...>& tup, size_t idx, F function)
{
    return visit_impl<sizeof...(Ts)>::visit(tup, idx, function);
}

}

#endif
