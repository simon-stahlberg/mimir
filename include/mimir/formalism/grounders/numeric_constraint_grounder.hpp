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

#ifndef MIMIR_FORMALISM_GROUNDERS_NUMERIC_CONSTRAINT_GROUNDER_HPP_
#define MIMIR_FORMALISM_GROUNDERS_NUMERIC_CONSTRAINT_GROUNDER_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/grounders/grounding_table.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir
{
class NumericConstraintGrounder
{
private:
    std::shared_ptr<FunctionExpressionGrounder> m_fexpr_grounder;

    // GroundingTableList<GroundConditionNumeric> m_grounding_tables;

public:
    /// @brief Simplest construction
    explicit NumericConstraintGrounder(std::shared_ptr<FunctionExpressionGrounder> fexpr_grounder);

    // Uncopyable
    NumericConstraintGrounder(const NumericConstraintGrounder& other) = delete;
    NumericConstraintGrounder& operator=(const NumericConstraintGrounder& other) = delete;
    // Moveable
    NumericConstraintGrounder(NumericConstraintGrounder&& other) = default;
    NumericConstraintGrounder& operator=(NumericConstraintGrounder&& other) = default;

    GroundNumericConstraint ground(NumericConstraint numeric_constraint, const ObjectList& binding);

    /**
     * Getters
     */

    const std::shared_ptr<FunctionExpressionGrounder>& get_fexpr_grounder() const;
};

}  // namespace mimir

#endif
