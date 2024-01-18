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

#ifndef MIMIR_FORMALISM_DOMAIN_EFFECTS_HPP_
#define MIMIR_FORMALISM_DOMAIN_EFFECTS_HPP_

#include "declarations.hpp"

#include "../../common/mixins.hpp"

#include <loki/domain/pddl/effects.hpp>

#include <string>


namespace loki {
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}


namespace mimir {

/* Literal */
class EffectLiteralImpl : public loki::Base<EffectLiteralImpl> {
private:
    loki::pddl::Effect external_;

    Literal m_literal;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    EffectLiteralImpl(int identifier, loki::pddl::Effect external, Literal literal);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectLiteralImpl>;

public:
    bool is_structurally_equivalent_to_impl(const EffectLiteralImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    const Literal& get_literal() const;
};


/* And */
class EffectAndImpl : public loki::Base<EffectAndImpl> {
private:
    loki::pddl::Effect external_;

    EffectList m_effects;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    EffectAndImpl(int identifier, EffectList effects);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    // Give access to the private interface implementations.
    friend class loki::Base<EffectAndImpl>;

public:
    bool is_structurally_equivalent_to_impl(const EffectAndImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    const EffectList& get_effects() const;
};


/* EffectNumeric */
class EffectNumericImpl : public loki::Base<EffectNumericImpl> {
private:
    loki::pddl::Effect external_;

    loki::pddl::AssignOperatorEnum m_assign_operator;
    Function m_function;
    FunctionExpression m_function_expression;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    EffectNumericImpl(int identifier, loki::pddl::Effect external, loki::pddl::AssignOperatorEnum assign_operator, Function function, FunctionExpression function_expression);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

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
class EffectConditionalForallImpl : public loki::Base<EffectConditionalForallImpl> {
private:
    loki::pddl::Effect external_;

    ParameterList m_parameters;
    Effect m_effect;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    EffectConditionalForallImpl(int identifier, loki::pddl::Effect external, ParameterList parameters, Effect effect);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

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
class EffectConditionalWhenImpl : public loki::Base<EffectConditionalWhenImpl> {
private:
    loki::pddl::Effect external_;

    Condition m_condition;
    Effect m_effect;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    EffectConditionalWhenImpl(int identifier, loki::pddl::Effect external, Condition condition, Effect effect);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

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


namespace std {
    // Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct less<loki::pddl::Effect>
    {
        bool operator()(const loki::pddl::Effect& left_effect, const loki::pddl::Effect& right_effect) const;
    };

    template<>
    struct hash<loki::pddl::EffectLiteralImpl>
    {
        std::size_t operator()(const loki::pddl::EffectLiteralImpl& effect) const;
    };

    template<>
    struct hash<loki::pddl::EffectAndImpl>
    {
        std::size_t operator()(const loki::pddl::EffectAndImpl& effect) const;
    };

    template<>
    struct hash<loki::pddl::EffectNumericImpl>
    {
        std::size_t operator()(const loki::pddl::EffectNumericImpl& effect) const;
    };

    template<>
    struct hash<loki::pddl::EffectConditionalForallImpl>
    {
        std::size_t operator()(const loki::pddl::EffectConditionalForallImpl& effect) const;
    };

    template<>
    struct hash<loki::pddl::EffectConditionalWhenImpl>
    {
        std::size_t operator()(const loki::pddl::EffectConditionalWhenImpl& effect) const;
    };
}


#endif
