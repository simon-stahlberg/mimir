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
#include "mimir/formalism/predicate.hpp"

#include <loki/loki.hpp>
#include <string>

namespace mimir
{
class AxiomImpl : public loki::Base<AxiomImpl>
{
private:
    VariableList m_parameters;
    Literal<FluentPredicateImpl> m_literal;
    LiteralList<StaticPredicateImpl> m_static_conditions;
    LiteralList<FluentPredicateImpl> m_fluent_conditions;

    AxiomImpl(int identifier,
              VariableList parameters,
              Literal<FluentPredicateImpl> literal,
              LiteralList<StaticPredicateImpl> static_conditions,
              LiteralList<FluentPredicateImpl> fluent_conditions);

    // Give access to the constructor.
    friend class loki::PDDLFactory<AxiomImpl, loki::Hash<AxiomImpl*>, loki::EqualTo<AxiomImpl*>>;

    /// @brief Test for structural equivalence
    bool is_structurally_equivalent_to_impl(const AxiomImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<AxiomImpl>;

public:
    const VariableList& get_parameters() const;
    const Literal<FluentPredicateImpl>& get_literal() const;
    const LiteralList<StaticPredicateImpl>& get_static_conditions() const;
    const LiteralList<FluentPredicateImpl>& get_fluent_conditions() const;

    size_t get_arity() const;
};

}

#endif
