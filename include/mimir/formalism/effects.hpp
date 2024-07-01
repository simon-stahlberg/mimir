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

#include "mimir/common/concepts.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/variable.hpp"

#include <loki/loki.hpp>
#include <string>
#include <vector>

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

    EffectSimpleImpl(int identifier, Literal<Fluent> effect);

    // Give access to the constructor.
    friend class loki::PDDLFactory<EffectSimpleImpl, loki::Hash<EffectSimpleImpl*>, loki::EqualTo<EffectSimpleImpl*>>;

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

    EffectConditionalImpl(int identifier,
                          LiteralList<Static> static_conditions,
                          LiteralList<Fluent> fluent_conditions,
                          LiteralList<Derived> derived_conditions,
                          Literal<Fluent> effect);

    // Give access to the constructor.
    friend class loki::PDDLFactory<EffectConditionalImpl, loki::Hash<EffectConditionalImpl*>, loki::EqualTo<EffectConditionalImpl*>>;

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

    EffectUniversalImpl(int identifier,
                        VariableList quantified_variables,
                        LiteralList<Static> static_conditions,
                        LiteralList<Fluent> fluent_conditions,
                        LiteralList<Derived> derived_conditions,
                        Literal<Fluent> effect);

    // Give access to the constructor.
    friend class loki::PDDLFactory<EffectUniversalImpl, loki::Hash<EffectUniversalImpl*>, loki::EqualTo<EffectUniversalImpl*>>;

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

/**
 * Type aliases
 */

using EffectSimple = const EffectSimpleImpl*;
using EffectSimpleList = std::vector<EffectSimple>;

using EffectConditional = const EffectConditionalImpl*;
using EffectConditionalList = std::vector<EffectConditional>;

using EffectUniversal = const EffectUniversalImpl*;
using EffectUniversalList = std::vector<EffectUniversal>;

/**
 * Implementations
 */

template<PredicateCategory P>
const LiteralList<P>& EffectConditionalImpl::get_conditions() const
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
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template<PredicateCategory P>
const LiteralList<P>& EffectUniversalImpl::get_conditions() const
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
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

}

#endif
