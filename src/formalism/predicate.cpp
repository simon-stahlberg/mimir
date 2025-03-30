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

#include "mimir/formalism/predicate.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/formalism/variable.hpp"

namespace mimir::formalism
{

template<IsStaticOrFluentOrDerivedTag P>
PredicateImpl<P>::PredicateImpl(Index index, std::string name, VariableList parameters) :
    m_index(index),
    m_name(std::move(name)),
    m_parameters(std::move(parameters))
{
    assert(is_all_unique(m_parameters));
}

template<IsStaticOrFluentOrDerivedTag P>
Index PredicateImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
const std::string& PredicateImpl<P>::get_name() const
{
    return m_name;
}

template<IsStaticOrFluentOrDerivedTag P>
const VariableList& PredicateImpl<P>::get_parameters() const
{
    return m_parameters;
}

template<IsStaticOrFluentOrDerivedTag P>
size_t PredicateImpl<P>::get_arity() const
{
    return m_parameters.size();
}

template class PredicateImpl<StaticTag>;
template class PredicateImpl<FluentTag>;
template class PredicateImpl<DerivedTag>;

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const PredicateImpl<P>& element)
{
    write(element, StringFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const PredicateImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const PredicateImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const PredicateImpl<DerivedTag>& element);

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, Predicate<P> element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, Predicate<StaticTag> element);
template std::ostream& operator<<(std::ostream& out, Predicate<FluentTag> element);
template std::ostream& operator<<(std::ostream& out, Predicate<DerivedTag> element);

}
