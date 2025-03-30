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

namespace mimir::formalism
{
template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunctionValueImpl<F>::GroundFunctionValueImpl(Index index, GroundFunction<F> function, double number) :
    m_index(index),
    m_function(std::move(function)),
    m_number(number)
{
}

template<IsStaticOrFluentOrAuxiliaryTag F>
Index GroundFunctionValueImpl<F>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunction<F> GroundFunctionValueImpl<F>::get_function() const
{
    return m_function;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
double GroundFunctionValueImpl<F>::get_number() const
{
    return m_number;
}

template class GroundFunctionValueImpl<StaticTag>;
template class GroundFunctionValueImpl<FluentTag>;
template class GroundFunctionValueImpl<AuxiliaryTag>;

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<F>& element)
{
    write(element, StringFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<AuxiliaryTag>& element);

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, GroundFunctionValue<F> element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, GroundFunctionValue<StaticTag> element);
template std::ostream& operator<<(std::ostream& out, GroundFunctionValue<FluentTag> element);
template std::ostream& operator<<(std::ostream& out, GroundFunctionValue<AuxiliaryTag> element);

}