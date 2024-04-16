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

#include "mimir/formalism/conditions.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/literal.hpp"

#include <loki/loki.hpp>

namespace mimir
{
AxiomImpl::AxiomImpl(int identifier, ParameterList parameters, Literal literal, LiteralList condition) :
    Base(identifier),
    m_parameters(std::move(parameters)),
    m_literal(std::move(literal)),
    m_condition(std::move(condition))
{
}

bool AxiomImpl::is_structurally_equivalent_to_impl(const AxiomImpl& other) const
{
    return (m_literal == other.m_literal) && (loki::get_sorted_vector(m_condition) == loki::get_sorted_vector(other.m_condition));
}

size_t AxiomImpl::hash_impl() const { return hash_combine(m_literal, loki::hash_container(loki::get_sorted_vector(m_condition))); }

void AxiomImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    auto nested_options = loki::FormattingOptions { options.indent + options.add_indent, options.add_indent };
    out << std::string(options.indent, ' ') << "(:derived " << *m_literal << "\n";
    out << std::string(nested_options.indent, ' ') << "(and ";
    for (size_t i = 0; i < m_condition.size(); ++i)
    {
        if (i != 0)
        {
            out << " ";
        }
        out << *m_condition[i];
    }
    out << ")\n";
}

const ParameterList& AxiomImpl::get_parameters() const { return m_parameters; }

const Literal& AxiomImpl::get_literal() const { return m_literal; }

const LiteralList& AxiomImpl::get_condition() const { return m_condition; }

}
