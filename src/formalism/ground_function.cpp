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

#include "mimir/formalism/ground_function.hpp"

#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/object.hpp"

namespace mimir::formalism
{
template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunctionImpl<F>::GroundFunctionImpl(Index index, FunctionSkeleton<F> function_skeleton, ObjectList objects) :
    m_index(index),
    m_function_skeleton(function_skeleton),
    m_objects(std::move(objects))
{
}

template<IsStaticOrFluentOrAuxiliaryTag F>
Index GroundFunctionImpl<F>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
FunctionSkeleton<F> GroundFunctionImpl<F>::get_function_skeleton() const
{
    return m_function_skeleton;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
const ObjectList& GroundFunctionImpl<F>::get_objects() const
{
    return m_objects;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
size_t GroundFunctionImpl<F>::get_arity() const
{
    return m_objects.size();
}

template class GroundFunctionImpl<StaticTag>;
template class GroundFunctionImpl<FluentTag>;
template class GroundFunctionImpl<AuxiliaryTag>;
}
