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

#ifndef MIMIR_FORMALISM_DOMAIN_FUNCTION_EXPRESSIONS_HPP_
#define MIMIR_FORMALISM_DOMAIN_FUNCTION_EXPRESSIONS_HPP_

#include "declarations.hpp"

#include "../../common/mixins.hpp"

#include <loki/domain/pddl/function_expressions.hpp>

#include <string>


namespace loki {
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}


namespace mimir {

/* FunctionExpressionNumber */
class FunctionExpressionNumberImpl : public loki::Base<FunctionExpressionNumberImpl> {
private:
    loki::pddl::FunctionExpression external_;

    double m_number;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    FunctionExpressionNumberImpl(int identifier, loki::pddl::FunctionExpression external, double number);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionNumberImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionNumberImpl>;

public:
    double get_number() const;
};


/* FunctionExpressionBinaryOperator */
class FunctionExpressionBinaryOperatorImpl : public loki::Base<FunctionExpressionBinaryOperatorImpl> {
private:
    loki::pddl::FunctionExpression external_;

    loki::pddl::BinaryOperatorEnum m_binary_operator;
    FunctionExpression m_left_function_expression;
    FunctionExpression m_right_function_expression;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    FunctionExpressionBinaryOperatorImpl(int identifier,
        loki::pddl::FunctionExpression external,
        loki::pddl::BinaryOperatorEnum binary_operator,
        FunctionExpression left_function_expression,
        FunctionExpression right_function_expression);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionBinaryOperatorImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionBinaryOperatorImpl>;

public:
    loki::pddl::BinaryOperatorEnum get_binary_operator() const;
    const FunctionExpression& get_left_function_expression() const;
    const FunctionExpression& get_right_function_expression() const;
};


/* FunctionExpressionMultiOperator */
class FunctionExpressionMultiOperatorImpl : public loki::Base<FunctionExpressionMultiOperatorImpl> {
private:
    loki::pddl::FunctionExpression external_;

    loki::pddl::MultiOperatorEnum m_multi_operator;
    FunctionExpressionList m_function_expressions;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    FunctionExpressionMultiOperatorImpl(int identifier,
        loki::pddl::FunctionExpression external,
        loki::pddl::MultiOperatorEnum multi_operator,
        FunctionExpressionList function_expressions);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionMultiOperatorImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionMultiOperatorImpl>;

public:
    loki::pddl::MultiOperatorEnum get_multi_operator() const;
    const FunctionExpressionList& get_function_expressions() const;
};


/* FunctionExpressionMinus */
class FunctionExpressionMinusImpl : public loki::Base<FunctionExpressionMinusImpl> {
private:
    loki::pddl::FunctionExpression external_;

    FunctionExpression m_function_expression;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    FunctionExpressionMinusImpl(int identifier, loki::pddl::FunctionExpression external, FunctionExpression function_expression);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionMinusImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionMinusImpl>;

public:
    const FunctionExpression& get_function_expression() const;
};


/* FunctionExpressionFunction */
class FunctionExpressionFunctionImpl : public loki::Base<FunctionExpressionFunctionImpl> {
private:
    loki::pddl::FunctionExpression external_;

    Function m_function;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    FunctionExpressionFunctionImpl(int identifier, loki::pddl::FunctionExpression external, Function function);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const FunctionExpressionFunctionImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<FunctionExpressionFunctionImpl>;

public:
    const Function& get_function() const;
};

}


namespace std {
    // Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct less<loki::pddl::FunctionExpression>
    {
        bool operator()(const loki::pddl::FunctionExpression& left_function_expression, const loki::pddl::FunctionExpression& right_function_expression) const;
    };

    template<>
    struct hash<loki::pddl::FunctionExpressionNumberImpl>
    {
        std::size_t operator()(const loki::pddl::FunctionExpressionNumberImpl& function_expressions) const;
    };

    template<>
    struct hash<loki::pddl::FunctionExpressionBinaryOperatorImpl>
    {
        std::size_t operator()(const loki::pddl::FunctionExpressionBinaryOperatorImpl& function_expressions) const;
    };

    template<>
    struct hash<loki::pddl::FunctionExpressionMultiOperatorImpl>
    {
        std::size_t operator()(const loki::pddl::FunctionExpressionMultiOperatorImpl& function_expressions) const;
    };

    template<>
    struct hash<loki::pddl::FunctionExpressionMinusImpl>
    {
        std::size_t operator()(const loki::pddl::FunctionExpressionMinusImpl& function_expressions) const;
    };

    template<>
    struct hash<loki::pddl::FunctionExpressionFunctionImpl>
    {
        std::size_t operator()(const loki::pddl::FunctionExpressionFunctionImpl& function_expressions) const;
    };
}

#endif
