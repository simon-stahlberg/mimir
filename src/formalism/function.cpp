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
FunctionImpl::FunctionImpl(Index index, FunctionSkeleton function_skeleton, TermList terms) :
    m_index(index),
    m_function_skeleton(std::move(function_skeleton)),
    m_terms(std::move(terms))
{
}

Index FunctionImpl::get_index() const { return m_index; }

const FunctionSkeleton& FunctionImpl::get_function_skeleton() const { return m_function_skeleton; }

const TermList& FunctionImpl::get_terms() const { return m_terms; }

std::ostream& operator<<(std::ostream& out, const FunctionImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Function element)
{
    out << *element;
    return out;
}
}
