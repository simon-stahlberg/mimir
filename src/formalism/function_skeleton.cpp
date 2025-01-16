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

#include "mimir/formalism/function_skeleton.hpp"

#include "formatter.hpp"
#include "mimir/formalism/variable.hpp"

namespace mimir
{
template<FunctionTag F>
FunctionSkeletonImpl<F>::FunctionSkeletonImpl(Index index, std::string name, VariableList parameters) :
    m_index(index),
    m_name(std::move(name)),
    m_parameters(std::move(parameters))
{
}

template<FunctionTag F>
Index FunctionSkeletonImpl<F>::get_index() const
{
    return m_index;
}

template<FunctionTag F>
const std::string& FunctionSkeletonImpl<F>::get_name() const
{
    return m_name;
}

template<FunctionTag F>
const VariableList& FunctionSkeletonImpl<F>::get_parameters() const
{
    return m_parameters;
}

template class FunctionSkeletonImpl<Static>;
template class FunctionSkeletonImpl<Fluent>;
template class FunctionSkeletonImpl<Auxiliary>;

template<FunctionTag F>
std::ostream& operator<<(std::ostream& out, const FunctionSkeletonImpl<F>& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const FunctionSkeletonImpl<Static>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionSkeletonImpl<Fluent>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionSkeletonImpl<Auxiliary>& element);

template<FunctionTag F>
std::ostream& operator<<(std::ostream& out, FunctionSkeleton<F> element)
{
    out << *element;
    return out;
}

template std::ostream& operator<<(std::ostream& out, FunctionSkeleton<Static> element);
template std::ostream& operator<<(std::ostream& out, FunctionSkeleton<Fluent> element);
template std::ostream& operator<<(std::ostream& out, FunctionSkeleton<Auxiliary> element);

}
