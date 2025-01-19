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

#include "mimir/formalism/ground_function_value.hpp"

#include "formatter.hpp"
#include "mimir/formalism/ground_function.hpp"

namespace mimir
{
template<FunctionTag F>
GroundFunctionValueImpl<F>::GroundFunctionValueImpl(Index index, GroundFunction<F> function, double number) :
    m_index(index),
    m_function(std::move(function)),
    m_number(number)
{
}

template<FunctionTag F>
Index GroundFunctionValueImpl<F>::get_index() const
{
    return m_index;
}

template<FunctionTag F>
const GroundFunction<F>& GroundFunctionValueImpl<F>::get_function() const
{
    return m_function;
}

template<FunctionTag F>
double GroundFunctionValueImpl<F>::get_number() const
{
    return m_number;
}

template class GroundFunctionValueImpl<Static>;
template class GroundFunctionValueImpl<Fluent>;
template class GroundFunctionValueImpl<Auxiliary>;

template<FunctionTag F>
std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<F>& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<Static>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<Fluent>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<Auxiliary>& element);

template<FunctionTag F>
std::ostream& operator<<(std::ostream& out, GroundFunctionValue<F> element)
{
    out << *element;
    return out;
}

template std::ostream& operator<<(std::ostream& out, GroundFunctionValue<Static> element);
template std::ostream& operator<<(std::ostream& out, GroundFunctionValue<Fluent> element);
template std::ostream& operator<<(std::ostream& out, GroundFunctionValue<Auxiliary> element);

}