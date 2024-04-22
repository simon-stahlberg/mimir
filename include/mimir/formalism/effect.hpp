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

#ifndef MIMIR_FORMALISM_EFFECTS_HPP_
#define MIMIR_FORMALISM_EFFECTS_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/loki.hpp>
#include <string>

namespace mimir
{
class EffectImpl : public loki::Base<EffectImpl>
{
private:
    ParameterList m_quantified_variables;
    LiteralList m_conditions;
    LiteralList m_static_condition;
    LiteralList m_fluent_condition;
    Literal m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    EffectImpl(int identifier,
               ParameterList quantified_variables,
               LiteralList conditions,
               LiteralList static_condition,
               LiteralList fluent_condition,
               Literal effect);

    // Give access to the constructor.
    friend class loki::PDDLFactory<EffectImpl, loki::Hash<EffectImpl*>, loki::EqualTo<EffectImpl*>>;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectImpl>;

    bool is_structurally_equivalent_to_impl(const EffectImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

public:
    const ParameterList& get_parameters() const;
    const LiteralList& get_conditions() const;
    const LiteralList& get_static_conditions() const;
    const LiteralList& get_fluent_conditions() const;
    const Literal& get_effect() const;
};

}

#endif
