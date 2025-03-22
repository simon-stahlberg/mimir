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

#include "mimir/formalism/function.hpp"

#include "formatter.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/term.hpp"

namespace mimir::formalism
{
template<IsStaticOrFluentOrAuxiliaryTag F>
FunctionImpl<F>::FunctionImpl(Index index, FunctionSkeleton<F> function_skeleton, TermList terms, IndexList parent_terms_to_terms_mapping) :
    m_index(index),
    m_function_skeleton(std::move(function_skeleton)),
    m_terms(std::move(terms)),
    m_parent_terms_to_terms_mapping(std::move(parent_terms_to_terms_mapping))
{
}

template<IsStaticOrFluentOrAuxiliaryTag F>
Index FunctionImpl<F>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
FunctionSkeleton<F> FunctionImpl<F>::get_function_skeleton() const
{
    return m_function_skeleton;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
const TermList& FunctionImpl<F>::get_terms() const
{
    return m_terms;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
const IndexList& FunctionImpl<F>::get_parent_terms_to_terms_mapping() const
{
    return m_parent_terms_to_terms_mapping;
}

template class FunctionImpl<StaticTag>;
template class FunctionImpl<FluentTag>;
template class FunctionImpl<AuxiliaryTag>;

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const FunctionImpl<F>& element)
{
    write(element, StringFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const FunctionImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionImpl<AuxiliaryTag>& element);

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, Function<F> element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, Function<StaticTag> element);
template std::ostream& operator<<(std::ostream& out, Function<FluentTag> element);
template std::ostream& operator<<(std::ostream& out, Function<AuxiliaryTag> element);
}
