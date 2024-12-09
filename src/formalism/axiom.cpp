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

#include "mimir/formalism/axiom.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/variable.hpp"

#include <cassert>

namespace mimir
{
AxiomImpl::AxiomImpl(Index index,
                     VariableList parameters,
                     Literal<Derived> literal,
                     LiteralList<Static> static_conditions,
                     LiteralList<Fluent> fluent_conditions,
                     LiteralList<Derived> derived_conditions) :
    m_index(index),
    m_parameters(std::move(parameters)),
    m_literal(std::move(literal)),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions)),
    m_derived_conditions(std::move(derived_conditions))
{
    assert(!literal->is_negated());
    assert(is_all_unique(m_parameters));
    assert(is_all_unique(m_static_conditions));
    assert(is_all_unique(m_fluent_conditions));
    assert(is_all_unique(m_derived_conditions));
    assert(
        std::is_sorted(m_static_conditions.begin(), m_static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_fluent_conditions.begin(), m_fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(
        std::is_sorted(m_derived_conditions.begin(), m_derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

Index AxiomImpl::get_index() const { return m_index; }

const VariableList& AxiomImpl::get_parameters() const { return m_parameters; }

const Literal<Derived>& AxiomImpl::get_literal() const { return m_literal; }

template<PredicateTag P>
const LiteralList<P>& AxiomImpl::get_conditions() const
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

template const LiteralList<Static>& AxiomImpl::get_conditions<Static>() const;
template const LiteralList<Fluent>& AxiomImpl::get_conditions<Fluent>() const;
template const LiteralList<Derived>& AxiomImpl::get_conditions<Derived>() const;

size_t AxiomImpl::get_arity() const { return m_parameters.size(); }

std::ostream& operator<<(std::ostream& out, const AxiomImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Axiom element)
{
    out << *element;
    return out;
}

}
