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
class EffectSimpleImpl
{
private:
    Index m_index;
    Literal<Fluent> m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    EffectSimpleImpl(Index index, Literal<Fluent> effect);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    EffectSimpleImpl(const EffectSimpleImpl& other) = delete;
    EffectSimpleImpl& operator=(const EffectSimpleImpl& other) = delete;
    EffectSimpleImpl(EffectSimpleImpl&& other) = default;
    EffectSimpleImpl& operator=(EffectSimpleImpl&& other) = default;

    std::string str() const;

    Index get_index() const;
    const Literal<Fluent>& get_effect() const;
};

/**
 * Type 2 effects
 */
class EffectComplexImpl
{
private:
    Index m_index;
    VariableList m_quantified_variables;
    LiteralList<Static> m_static_conditions;
    LiteralList<Fluent> m_fluent_conditions;
    LiteralList<Derived> m_derived_conditions;
    Literal<Fluent> m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    EffectComplexImpl(Index index,
                      VariableList quantified_variables,
                      LiteralList<Static> static_conditions,
                      LiteralList<Fluent> fluent_conditions,
                      LiteralList<Derived> derived_conditions,
                      Literal<Fluent> effect);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    EffectComplexImpl(const EffectComplexImpl& other) = delete;
    EffectComplexImpl& operator=(const EffectComplexImpl& other) = delete;
    EffectComplexImpl(EffectComplexImpl&& other) = default;
    EffectComplexImpl& operator=(EffectComplexImpl&& other) = default;

    std::string str() const;

    Index get_index() const;
    const VariableList& get_parameters() const;
    template<PredicateTag P>
    const LiteralList<P>& get_conditions() const;
    const Literal<Fluent>& get_effect() const;

    size_t get_arity() const;
};

extern std::ostream& operator<<(std::ostream& out, const EffectSimpleImpl& element);
extern std::ostream& operator<<(std::ostream& out, const EffectComplexImpl& element);

extern std::ostream& operator<<(std::ostream& out, EffectSimple element);
extern std::ostream& operator<<(std::ostream& out, EffectComplex element);

}

#endif
