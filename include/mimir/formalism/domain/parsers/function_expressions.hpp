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

#ifndef MIMIR_FORMALISM_DOMAIN_PARSER_FUNCTION_EXPRESSIONS_HPP_
#define MIMIR_FORMALISM_DOMAIN_PARSER_FUNCTION_EXPRESSIONS_HPP_

#include "../function_expressions.hpp"
#include "../declarations.hpp"
#include "../../common/types.hpp"



namespace mimir 
{
    class FunctionExpressionVisitor {
        private:
            PDDLFactories& factories;

        public:
            FunctionExpressionVisitor(PDDLFactories& factories_);

            FunctionExpression operator()(const loki::pddl::FunctionExpressionNumberImpl& node);
            FunctionExpression operator()(const loki::pddl::FunctionExpressionBinaryOperatorImpl& node);
            FunctionExpression operator()(const loki::pddl::FunctionExpressionMultiOperatorImpl& node);
            FunctionExpression operator()(const loki::pddl::FunctionExpressionMinusImpl& node);
            FunctionExpression operator()(const loki::pddl::FunctionExpressionFunctionImpl& node);
    };

    extern FunctionExpression parse(loki::pddl::FunctionExpression function_expression, PDDLFactories& factories);
    extern FunctionExpressionList parse(loki::pddl::FunctionExpressionList function_expression_list, PDDLFactories& factories);
} 


#endif 
