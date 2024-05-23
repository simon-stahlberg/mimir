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
#include "mimir/formalism/literal.hpp"

#include <cassert>
#include <loki/loki.hpp>

namespace mimir
{
AxiomImpl::AxiomImpl(int identifier,
                     VariableList parameters,
                     Literal<FluentPredicateImpl> literal,
                     LiteralList<StaticPredicateImpl> static_conditions,
                     LiteralList<FluentPredicateImpl> fluent_conditions) :
    Base(identifier),
    m_parameters(std::move(parameters)),
    m_literal(std::move(literal)),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions))
{
    assert(is_all_unique(m_parameters));
    assert(is_all_unique(m_static_conditions));
    assert(is_all_unique(m_fluent_conditions));
}

bool AxiomImpl::is_structurally_equivalent_to_impl(const AxiomImpl& other) const
{
    if (this != &other)
    {
        return (m_literal == other.m_literal) && (loki::get_sorted_vector(m_static_conditions) == loki::get_sorted_vector(other.m_static_conditions))
               && (loki::get_sorted_vector(m_fluent_conditions) == loki::get_sorted_vector(other.m_fluent_conditions));
    }
    return true;
}

size_t AxiomImpl::hash_impl() const
{
    return hash_combine(m_literal,
                        loki::hash_container(loki::get_sorted_vector(m_static_conditions)),
                        loki::hash_container(loki::get_sorted_vector(m_fluent_conditions)));
}

void AxiomImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    auto nested_options = loki::FormattingOptions { options.indent + options.add_indent, options.add_indent };
    out << std::string(options.indent, ' ') << "(:derived " << *m_literal << "\n";

    out << std::string(nested_options.indent, ' ') << "(and";
    for (const auto& condition : m_static_conditions)
    {
        out << " " << *condition;
    }
    for (const auto& condition : m_fluent_conditions)
    {
        out << " " << *condition;
    }
    out << " )\n";
}

const VariableList& AxiomImpl::get_parameters() const { return m_parameters; }

const Literal<FluentPredicateImpl>& AxiomImpl::get_literal() const { return m_literal; }

const LiteralList<StaticPredicateImpl>& AxiomImpl::get_static_conditions() const { return m_static_conditions; }

const LiteralList<FluentPredicateImpl>& AxiomImpl::get_fluent_conditions() const { return m_fluent_conditions; }

size_t AxiomImpl::get_arity() const { return m_parameters.size(); }

}
