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
    LiteralList m_static_conditions;
    LiteralList m_fluent_conditions;
    Literal m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    EffectImpl(int identifier,
               ParameterList quantified_variables,
               LiteralList conditions,
               LiteralList static_conditions,
               LiteralList fluent_conditions,
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

/**
 * Type 1 effects
 */
class EffectSimpleImpl
{
private:
    Literal m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    EffectSimpleImpl(int identifier, Literal effect);

    // Give access to the constructor.
    friend class loki::PDDLFactory<EffectSimpleImpl, loki::Hash<EffectSimpleImpl*>, loki::EqualTo<EffectSimpleImpl*>>;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectSimpleImpl>;

    bool is_structurally_equivalent_to_impl(const EffectSimpleImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

public:
    const Literal& get_effect() const;
};

/**
 * Type 2 effects
 */
class EffectConditionalSimpleImpl
{
private:
    LiteralList m_conditions;
    LiteralList m_static_conditions;
    LiteralList m_fluent_conditions;
    Literal m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    EffectConditionalSimpleImpl(int identifier, LiteralList conditions, LiteralList static_conditions, LiteralList fluent_conditions, Literal effect);

    // Give access to the constructor.
    friend class loki::PDDLFactory<EffectConditionalSimpleImpl, loki::Hash<EffectConditionalSimpleImpl*>, loki::EqualTo<EffectConditionalSimpleImpl*>>;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectConditionalSimpleImpl>;

    bool is_structurally_equivalent_to_impl(const EffectConditionalSimpleImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

public:
    const LiteralList& get_conditions() const;
    const LiteralList& get_static_conditions() const;
    const LiteralList& get_fluent_conditions() const;
    const Literal& get_effect() const;
};

/**
 * Type 3 effects
 */
class EffectUniversalConditionalSimpleImpl
{
private:
    ParameterList m_quantified_variables;
    LiteralList m_conditions;
    LiteralList m_static_conditions;
    LiteralList m_fluent_conditions;
    Literal m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    EffectUniversalConditionalSimpleImpl(int identifier,
                                         ParameterList quantified_variables,
                                         LiteralList conditions,
                                         LiteralList static_conditions,
                                         LiteralList fluent_conditions,
                                         Literal effect);

    // Give access to the constructor.
    friend class loki::PDDLFactory<EffectUniversalConditionalSimpleImpl,
                                   loki::Hash<EffectUniversalConditionalSimpleImpl*>,
                                   loki::EqualTo<EffectUniversalConditionalSimpleImpl*>>;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectUniversalConditionalSimpleImpl>;

    bool is_structurally_equivalent_to_impl(const EffectUniversalConditionalSimpleImpl& other) const;
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
