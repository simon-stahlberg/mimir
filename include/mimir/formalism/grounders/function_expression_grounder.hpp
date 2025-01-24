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

#ifndef MIMIR_FORMALISM_GROUNDERS_FUNCTION_EXPRESSION_GROUNDER_HPP_
#define MIMIR_FORMALISM_GROUNDERS_FUNCTION_EXPRESSION_GROUNDER_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/grounders/grounding_table.hpp"

namespace mimir
{
class FunctionExpressionGrounder
{
private:
    std::shared_ptr<FunctionGrounder> m_function_grounder;

    // GroundingTableList<GroundFunctionExpression> m_grounding_tables;

public:
    /// @brief Simplest construction
    explicit FunctionExpressionGrounder(std::shared_ptr<FunctionGrounder> function_grounder);

    // Uncopyable
    FunctionExpressionGrounder(const FunctionExpressionGrounder& other) = delete;
    FunctionExpressionGrounder& operator=(const FunctionExpressionGrounder& other) = delete;
    // Moveable
    FunctionExpressionGrounder(FunctionExpressionGrounder&& other) = default;
    FunctionExpressionGrounder& operator=(FunctionExpressionGrounder&& other) = default;

    GroundFunctionExpression ground(FunctionExpression fexpr, const ObjectList& binding);

    /**
     * Getters
     */

    const std::shared_ptr<FunctionGrounder>& get_function_grounder() const;
};

}  // namespace mimir

#endif
