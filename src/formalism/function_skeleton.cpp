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

#include "mimir/formalism/variable.hpp"

namespace mimir::formalism
{
template<IsStaticOrFluentOrAuxiliaryTag F>
FunctionSkeletonImpl<F>::FunctionSkeletonImpl(Index index, std::string name, ParameterList parameters) :
    m_index(index),
    m_name(std::move(name)),
    m_parameters(std::move(parameters))
{
}

template<IsStaticOrFluentOrAuxiliaryTag F>
Index FunctionSkeletonImpl<F>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
const std::string& FunctionSkeletonImpl<F>::get_name() const
{
    return m_name;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
const ParameterList& FunctionSkeletonImpl<F>::get_parameters() const
{
    return m_parameters;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
size_t FunctionSkeletonImpl<F>::get_arity() const
{
    return m_parameters.size();
}

template class FunctionSkeletonImpl<StaticTag>;
template class FunctionSkeletonImpl<FluentTag>;
template class FunctionSkeletonImpl<AuxiliaryTag>;
}
