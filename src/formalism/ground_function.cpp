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

#include "formatter.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/object.hpp"

namespace mimir
{
GroundFunctionImpl::GroundFunctionImpl(Index index, FunctionSkeleton function_skeleton, ObjectList objects) :
    m_index(index),
    m_function_skeleton(std::move(function_skeleton)),
    m_objects(std::move(objects))
{
}

Index GroundFunctionImpl::get_index() const { return m_index; }

const FunctionSkeleton& GroundFunctionImpl::get_function_skeleton() const { return m_function_skeleton; }

const ObjectList& GroundFunctionImpl::get_objects() const { return m_objects; }

std::ostream& operator<<(std::ostream& out, const GroundFunctionImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, GroundFunction element)
{
    out << *element;
    return out;
}

}
