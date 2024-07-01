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

#include "mimir/common/collections.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/literal.hpp"

#include <cassert>
#include <loki/loki.hpp>

namespace mimir
{
AxiomImpl::AxiomImpl(int identifier,
                     VariableList parameters,
                     Literal<Derived> literal,
                     LiteralList<Static> static_conditions,
                     LiteralList<Fluent> fluent_conditions,
                     LiteralList<Derived> derived_conditions) :
    Base(identifier),
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
}

bool AxiomImpl::is_structurally_equivalent_to_impl(const AxiomImpl& other) const
{
    if (this != &other)
    {
        return (m_literal == other.m_literal) && (loki::get_sorted_vector(m_static_conditions) == loki::get_sorted_vector(other.m_static_conditions))
               && (loki::get_sorted_vector(m_fluent_conditions) == loki::get_sorted_vector(other.m_fluent_conditions))
               && (loki::get_sorted_vector(m_derived_conditions) == loki::get_sorted_vector(other.m_derived_conditions));
    }
    return true;
}

size_t AxiomImpl::hash_impl() const
{
    return loki::hash_combine(m_literal,
                              loki::hash_container(loki::get_sorted_vector(m_static_conditions)),
                              loki::hash_container(loki::get_sorted_vector(m_fluent_conditions)),
                              loki::hash_container(loki::get_sorted_vector(m_derived_conditions)));
}

void AxiomImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    auto nested_options = loki::FormattingOptions { options.indent + options.add_indent, options.add_indent };
    out << std::string(options.indent, ' ') << "(:derived " << *m_literal << std::endl;
    out << std::string(nested_options.indent, ' ') << "(and";
    for (const auto& condition : m_static_conditions)
    {
        out << " " << *condition;
    }
    for (const auto& condition : m_fluent_conditions)
    {
        out << " " << *condition;
    }
    for (const auto& condition : m_derived_conditions)
    {
        out << " " << *condition;
    }
    out << ")" << std::endl;
    out << std::string(options.indent, ' ') << ")" << std::endl;
}

const VariableList& AxiomImpl::get_parameters() const { return m_parameters; }

const Literal<Derived>& AxiomImpl::get_literal() const { return m_literal; }

size_t AxiomImpl::get_arity() const { return m_parameters.size(); }

}
