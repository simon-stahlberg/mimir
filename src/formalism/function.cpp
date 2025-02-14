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

namespace mimir
{
template<StaticOrFluentOrAuxiliary F>
FunctionImpl<F>::FunctionImpl(Index index, FunctionSkeleton<F> function_skeleton, TermList terms, IndexList parent_terms_to_terms_mapping) :
    m_index(index),
    m_function_skeleton(std::move(function_skeleton)),
    m_terms(std::move(terms)),
    m_parent_terms_to_terms_mapping(std::move(parent_terms_to_terms_mapping))
{
}

template<StaticOrFluentOrAuxiliary F>
Index FunctionImpl<F>::get_index() const
{
    return m_index;
}

template<StaticOrFluentOrAuxiliary F>
const FunctionSkeleton<F>& FunctionImpl<F>::get_function_skeleton() const
{
    return m_function_skeleton;
}

template<StaticOrFluentOrAuxiliary F>
const TermList& FunctionImpl<F>::get_terms() const
{
    return m_terms;
}

template<StaticOrFluentOrAuxiliary F>
const IndexList& FunctionImpl<F>::get_parent_terms_to_terms_mapping() const
{
    return m_parent_terms_to_terms_mapping;
}

template class FunctionImpl<Static>;
template class FunctionImpl<Fluent>;
template class FunctionImpl<Auxiliary>;

template<StaticOrFluentOrAuxiliary F>
std::ostream& operator<<(std::ostream& out, const FunctionImpl<F>& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const FunctionImpl<Static>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionImpl<Fluent>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionImpl<Auxiliary>& element);

template<StaticOrFluentOrAuxiliary F>
std::ostream& operator<<(std::ostream& out, Function<F> element)
{
    out << *element;
    return out;
}

template std::ostream& operator<<(std::ostream& out, Function<Static> element);
template std::ostream& operator<<(std::ostream& out, Function<Fluent> element);
template std::ostream& operator<<(std::ostream& out, Function<Auxiliary> element);
}
