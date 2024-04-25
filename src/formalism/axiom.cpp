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
#include "mimir/formalism/effect.hpp"
#include "mimir/formalism/literal.hpp"

#include <cassert>
#include <loki/loki.hpp>

namespace mimir
{
AxiomImpl::AxiomImpl(int identifier,
                     ParameterList parameters,
                     Literal literal,
                     LiteralList conditions,
                     LiteralList static_conditions,
                     LiteralList fluent_conditions) :
    Base(identifier),
    m_parameters(std::move(parameters)),
    m_literal(std::move(literal)),
    m_conditions(std::move(conditions)),
    m_static_conditions(std::move(static_conditions)),
    m_fluent_conditions(std::move(fluent_conditions))
{
    assert(is_subseteq(m_static_conditions, m_conditions));
    assert(is_subseteq(m_fluent_conditions, m_conditions));
}

bool AxiomImpl::is_structurally_equivalent_to_impl(const AxiomImpl& other) const
{
    return (m_literal == other.m_literal) && (loki::get_sorted_vector(m_conditions) == loki::get_sorted_vector(other.m_conditions));
}

size_t AxiomImpl::hash_impl() const { return hash_combine(m_literal, loki::hash_container(loki::get_sorted_vector(m_conditions))); }

void AxiomImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    auto nested_options = loki::FormattingOptions { options.indent + options.add_indent, options.add_indent };
    out << std::string(options.indent, ' ') << "(:derived " << *m_literal << "\n";
    out << std::string(nested_options.indent, ' ') << "(and ";
    for (size_t i = 0; i < m_conditions.size(); ++i)
    {
        if (i != 0)
        {
            out << " ";
        }
        out << *m_conditions[i];
    }
    out << ")\n";
}

const ParameterList& AxiomImpl::get_parameters() const { return m_parameters; }

const Literal& AxiomImpl::get_literal() const { return m_literal; }

const LiteralList& AxiomImpl::get_conditions() const { return m_conditions; }

const LiteralList& AxiomImpl::get_static_conditions() const { return m_static_conditions; }

const LiteralList& AxiomImpl::get_fluent_conditions() const { return m_fluent_conditions; }

}
