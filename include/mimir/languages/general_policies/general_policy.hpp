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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_GENERAL_POLICY_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_GENERAL_POLICY_HPP_

#include "mimir/datasets/declarations.hpp"
#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/languages/general_policies/declarations.hpp"

namespace mimir::languages::general_policies
{
class GeneralPolicy
{
private:
    RuleList m_rules;

public:
    /// @brief Create a `GeneralPolicy` for the given `RuleList`.
    /// @param rules
    explicit GeneralPolicy(RuleList rules);

    /// @brief Return true if and only if there the state pair (transition) is compatible with a `Rule` in the `GeneralPolicy`.
    /// @param source_context is the source context.
    /// @param target_context is the target context.
    /// @return true if the state pair (transition) is compatible with a `Rule` in the `GeneralPolicy`.
    bool evaluate(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const;

    /// @brief Accept a `IVisitor`.
    /// @param visitor the `IVisitor`.
    void accept(IVisitor& visitor);

    /// @brief Return true if and only if the `GeneralPolicy` is structurally terminating.
    /// @return true if the `GeneralPolicy` is structurally terminating, and false otherwise.
    bool is_terminating() const;

    /// @brief Return true if and only if the `GeneralPolicy` solves all vertices of the `graphs::ClassGraph` in the given
    /// `datasets::GeneralizedStateSpace`.
    /// @param generalized_state_space is the `GeneralizedStateSpace`.
    /// @param graph is a subgraph of or equal to the `ClassGraph` in the `GeneralizedStateSpace`.
    /// @return true if the `GeneralPolicy` solves all vertices of the `graphs::ClassGraph` in the given `datasets::GeneralizedStateSpace`, and false otherwise.
    bool solves(const datasets::GeneralizedStateSpace& generalized_state_space, const graphs::ClassGraph& graph);

    /// @brief Get the rules in the `GeneralPolicy`.
    /// @return the rules in the `GeneralPolicy`.
    const RuleList& get_rules() const;
};
}

#endif