/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_FORMALISM_AXIOM_HPP_
#define MIMIR_FORMALISM_AXIOM_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/loki.hpp>
#include <string>

namespace mimir
{
class AxiomImpl : public loki::Base<AxiomImpl>
{
private:
    ParameterList m_parameters;
    Literal m_literal;
    LiteralList m_conditions;
    LiteralList m_static_conditions;
    LiteralList m_fluent_conditions;

    AxiomImpl(int identifier, ParameterList parameters, Literal literal, LiteralList conditions, LiteralList static_conditions, LiteralList fluent_conditions);

    // Give access to the constructor.
    friend class loki::PDDLFactory<AxiomImpl, loki::Hash<AxiomImpl*>, loki::EqualTo<AxiomImpl*>>;

    /// @brief Test for structural equivalence
    bool is_structurally_equivalent_to_impl(const AxiomImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<AxiomImpl>;

public:
    const ParameterList& get_parameters() const;
    const Literal& get_literal() const;
    const LiteralList& get_conditions() const;
    const LiteralList& get_static_conditions() const;
    const LiteralList& get_fluent_conditions() const;
};

}

#endif
