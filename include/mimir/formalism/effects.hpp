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

namespace mimir
{

/**
 * Type 1 effects
 */
class EffectSimpleImpl : public loki::Base<EffectSimpleImpl>
{
private:
    Literal<Fluent> m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    EffectSimpleImpl(size_t index, Literal<Fluent> effect);

    // Give access to the constructor.
    friend class loki::UniqueValueTypeFactory<EffectSimpleImpl, loki::Hash<const EffectSimpleImpl*, true>, loki::EqualTo<const EffectSimpleImpl*, true>>;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectSimpleImpl>;

    bool is_structurally_equivalent_to_impl(const EffectSimpleImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

public:
    const Literal<Fluent>& get_effect() const;
};

/**
 * Type 2 effects
 */
class EffectConditionalImpl : public loki::Base<EffectConditionalImpl>
{
private:
    LiteralList<Static> m_static_conditions;
    LiteralList<Fluent> m_fluent_conditions;
    LiteralList<Derived> m_derived_conditions;
    Literal<Fluent> m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    EffectConditionalImpl(size_t index,
                          LiteralList<Static> static_conditions,
                          LiteralList<Fluent> fluent_conditions,
                          LiteralList<Derived> derived_conditions,
                          Literal<Fluent> effect);

    // Give access to the constructor.
    friend class loki::
        UniqueValueTypeFactory<EffectConditionalImpl, loki::Hash<const EffectConditionalImpl*, true>, loki::EqualTo<const EffectConditionalImpl*, true>>;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectConditionalImpl>;

    bool is_structurally_equivalent_to_impl(const EffectConditionalImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

public:
    template<PredicateCategory P>
    const LiteralList<P>& get_conditions() const;
    const Literal<Fluent>& get_effect() const;
};

/**
 * Type 3 effects
 */
class EffectUniversalImpl : public loki::Base<EffectUniversalImpl>
{
private:
    VariableList m_quantified_variables;
    LiteralList<Static> m_static_conditions;
    LiteralList<Fluent> m_fluent_conditions;
    LiteralList<Derived> m_derived_conditions;
    Literal<Fluent> m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    EffectUniversalImpl(size_t index,
                        VariableList quantified_variables,
                        LiteralList<Static> static_conditions,
                        LiteralList<Fluent> fluent_conditions,
                        LiteralList<Derived> derived_conditions,
                        Literal<Fluent> effect);

    // Give access to the constructor.
    friend class loki::
        UniqueValueTypeFactory<EffectUniversalImpl, loki::Hash<const EffectUniversalImpl*, true>, loki::EqualTo<const EffectUniversalImpl*, true>>;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectUniversalImpl>;

    bool is_structurally_equivalent_to_impl(const EffectUniversalImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

public:
    const VariableList& get_parameters() const;
    template<PredicateCategory P>
    const LiteralList<P>& get_conditions() const;
    const Literal<Fluent>& get_effect() const;

    size_t get_arity() const;
};

}

#endif
