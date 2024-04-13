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

#ifndef MIMIR_FORMALISM_PARSER_FUNCTION_EXPRESSIONS_HPP_
#define MIMIR_FORMALISM_PARSER_FUNCTION_EXPRESSIONS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/function_expressions.hpp"

namespace mimir
{
class FunctionExpressionVisitor
{
private:
    PDDLFactories& factories;

public:
    FunctionExpressionVisitor(PDDLFactories& factories_);

    FunctionExpression operator()(const loki::FunctionExpressionNumberImpl& node);
    FunctionExpression operator()(const loki::FunctionExpressionBinaryOperatorImpl& node);
    FunctionExpression operator()(const loki::FunctionExpressionMultiOperatorImpl& node);
    FunctionExpression operator()(const loki::FunctionExpressionMinusImpl& node);
    FunctionExpression operator()(const loki::FunctionExpressionFunctionImpl& node);
};

extern FunctionExpression parse(loki::FunctionExpression function_expression, PDDLFactories& factories);
extern FunctionExpressionList parse(loki::FunctionExpressionList function_expression_list, PDDLFactories& factories);
}

#endif
