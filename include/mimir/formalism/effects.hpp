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

#ifndef MIMIR_FORMALISM_EFFECTS_HPP_
#define MIMIR_FORMALISM_EFFECTS_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/pddl/effects.hpp>
#include <loki/pddl/factory.hpp>
#include <string>

namespace mimir
{
/* Literal */
class EffectLiteralImpl : public loki::Base<EffectLiteralImpl>
{
private:
    Literal m_literal;

    // Below: add additional members if needed and initialize them in the constructor

    EffectLiteralImpl(int identifier, Literal literal);

    // Give access to the constructor.
    template<typename>
    friend class loki::PDDLFactory;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectLiteralImpl>;

public:
    bool is_structurally_equivalent_to_impl(const EffectLiteralImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    const Literal& get_literal() const;
};

/* And */
class EffectAndImpl : public loki::Base<EffectAndImpl>
{
private:
    EffectList m_effects;

    // Below: add additional members if needed and initialize them in the constructor

    EffectAndImpl(int identifier, EffectList effects);

    // Give access to the constructor.
    template<typename>
    friend class loki::PDDLFactory;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectAndImpl>;

public:
    bool is_structurally_equivalent_to_impl(const EffectAndImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    const EffectList& get_effects() const;
};

/* EffectNumeric */
class EffectNumericImpl : public loki::Base<EffectNumericImpl>
{
private:
    loki::pddl::AssignOperatorEnum m_assign_operator;
    Function m_function;
    FunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    EffectNumericImpl(int identifier, loki::pddl::AssignOperatorEnum assign_operator, Function function, FunctionExpression function_expression);

    // Give access to the constructor.
    template<typename>
    friend class loki::PDDLFactory;

    // Give access to the private interface implementations.
    friend class Base<EffectNumericImpl>;

public:
    bool is_structurally_equivalent_to_impl(const EffectNumericImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    loki::pddl::AssignOperatorEnum get_assign_operator() const;
    const Function& get_function() const;
    const FunctionExpression& get_function_expression() const;
};

/* ConditionalForall */
class EffectConditionalForallImpl : public loki::Base<EffectConditionalForallImpl>
{
private:
    ParameterList m_parameters;
    Effect m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    EffectConditionalForallImpl(int identifier, ParameterList parameters, Effect effect);

    // Give access to the constructor.
    template<typename>
    friend class loki::PDDLFactory;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectConditionalForallImpl>;

public:
    bool is_structurally_equivalent_to_impl(const EffectConditionalForallImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    const ParameterList& get_parameters() const;
    const Effect& get_effect() const;
};

/* ConditionalWhen */
class EffectConditionalWhenImpl : public loki::Base<EffectConditionalWhenImpl>
{
private:
    Condition m_condition;
    Effect m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    EffectConditionalWhenImpl(int identifier, Condition condition, Effect effect);

    // Give access to the constructor.
    template<typename>
    friend class loki::PDDLFactory;

    bool is_structurally_equivalent_to_impl(const EffectConditionalWhenImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectConditionalWhenImpl>;

public:
    const Condition& get_condition() const;
    const Effect& get_effect() const;
};
}

#endif
