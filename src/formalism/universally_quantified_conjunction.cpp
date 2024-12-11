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

#include "mimir/formalism/universally_quantified_conjunction.hpp"

#include "formatter.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/common/hash.hpp"

namespace mimir
{
UniversallyQuantifiedConjunctionImpl::UniversallyQuantifiedConjunctionImpl(Index index,
                                                                           VariableList parameters,
                                                                           LiteralList<Static> static_conditions,
                                                                           LiteralList<Fluent> fluent_conditions,
                                                                           LiteralList<Derived> derived_conditions,
                                                                           GroundLiteralList<Static> nullary_static_conditions,
                                                                           GroundLiteralList<Fluent> nullary_fluent_conditions,
                                                                           GroundLiteralList<Derived> nullary_derived_conditions) :
    m_index(index),
    m_parameters(std::move(parameters)),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions)),
    m_derived_conditions(std::move(derived_conditions)),
    m_nullary_static_conditions(std::move(nullary_static_conditions)),
    m_nullary_fluent_conditions(std::move(nullary_fluent_conditions)),
    m_nullary_derived_conditions(std::move(nullary_derived_conditions))
{
}

Index UniversallyQuantifiedConjunctionImpl::get_index() const { return m_index; }

const VariableList& UniversallyQuantifiedConjunctionImpl::get_parameters() const { return m_parameters; }

template<PredicateTag P>
const LiteralList<P>& UniversallyQuantifiedConjunctionImpl::get_literals() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_static_conditions;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_conditions;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_conditions;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const LiteralList<Static>& UniversallyQuantifiedConjunctionImpl::get_literals<Static>() const;
template const LiteralList<Fluent>& UniversallyQuantifiedConjunctionImpl::get_literals<Fluent>() const;
template const LiteralList<Derived>& UniversallyQuantifiedConjunctionImpl::get_literals<Derived>() const;

template<PredicateTag P>
const GroundLiteralList<P>& UniversallyQuantifiedConjunctionImpl::get_nullary_ground_literals() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_nullary_static_conditions;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_nullary_fluent_conditions;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_nullary_derived_conditions;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const GroundLiteralList<Static>& UniversallyQuantifiedConjunctionImpl::get_nullary_ground_literals<Static>() const;
template const GroundLiteralList<Fluent>& UniversallyQuantifiedConjunctionImpl::get_nullary_ground_literals<Fluent>() const;
template const GroundLiteralList<Derived>& UniversallyQuantifiedConjunctionImpl::get_nullary_ground_literals<Derived>() const;

size_t UniversallyQuantifiedConjunctionImpl::get_arity() const { return m_parameters.size(); }

std::ostream& operator<<(std::ostream& out, const UniversallyQuantifiedConjunctionImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, UniversallyQuantifiedConjunction element)
{
    out << *element;
    return out;
}
}
