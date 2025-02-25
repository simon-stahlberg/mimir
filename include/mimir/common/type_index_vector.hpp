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

#ifndef MIMIR_COMMON_TYPE_INDEXED_VECTOR_HPP_
#define MIMIR_COMMON_TYPE_INDEXED_VECTOR_HPP_

#include "mimir/common/concepts.hpp"

#include <boost/hana.hpp>
#include <vector>

namespace mimir
{
template<template<typename> typename T, typename... Is>
class TypedIndexedVector
{
private:
    template<typename X>
    using VectorType = std::vector<T<X>>;

    boost::hana::map<boost::hana::pair<boost::hana::type<Is>, VectorType<X>>...> m_data;

public:
    template<typename I>
        requires InTypes<I, Is...>
    VectorType<I>& get()
    {
        return boost::hana::at_key(m_data, boost::hana::type<I> {});
    }

    template<typename I>
        requires InTypes<I, Is...>
    const VectorType<I>& get() const
    {
        return boost::hana::at_key(m_data, boost::hana::type<I> {});
    }
};

}

#endif
