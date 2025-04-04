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

#ifndef MIMIR_LANGUAGES_POLICY_SKETCHES_POLICY_SKETCH_HPP_
#define MIMIR_LANGUAGES_POLICY_SKETCHES_POLICY_SKETCH_HPP_

#include "mimir/datasets/declarations.hpp"
#include "mimir/datasets/generalized_state_space/class_graph.hpp"
#include "mimir/languages/general_policies/declarations.hpp"
#include "mimir/languages/general_policies/general_policy.hpp"

namespace mimir::languages::policy_sketches
{
class PolicySketch
{
private:
    general_policies::GeneralPolicyImpl m_general_policy;

public:
    /// @brief Create a `PolicySketch` with underlying `general_policies::GeneralPolicy`.
    /// @param general_policy is a syntactic `general_policies::GeneralPolicy`.
    explicit PolicySketch(general_policies::GeneralPolicyImpl general_policy);

    /// @brief Return true if and only if there the state pair (transition) is compatible with a `Rule` in the underlying `general_policies::GeneralPolicy`.
    /// @param source_context is the source context.
    /// @param target_context is the target context.
    /// @return true if the state pair (transition) is compatible with a `Rule` in the underlying `general_policies::GeneralPolicy`.
    bool evaluate(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const;

    /// @brief Return true if and only if the underlying `general_policies::GeneralPolicy` is structurally terminating.
    /// @return true if the underlying `general_policies::GeneralPolicy` is structurally terminating, and false otherwise.
    bool is_terminating() const;

    /// @brief Return true if and only if the underlying `general_policies::GeneralPolicy` with the semantics of a `PolicySketch` solves all vertices of the
    /// `graphs::ClassGraph` in the given `datasets::GeneralizedStateSpace`.
    /// @param generalized_state_space is the `GeneralizedStateSpace`.
    /// @param tuple_graphs is the `TupleGraphCollection` of the `GeneralizedStateSpace`.
    /// @param graph is a subgraph of or equal to the `ClassGraph` in the `GeneralizedStateSpace`.
    /// @return true if the the underlying `general_policies::GeneralPolicy` with the semantics of a `PolicySketch` solves all vertices of the
    /// `graphs::ClassGraph` in the given `datasets::GeneralizedStateSpace`, and false otherwise.
    bool
    solves(const datasets::GeneralizedStateSpace& generalized_state_space, const datasets::TupleGraphCollection& tuple_graphs, const graphs::ClassGraph& graph);

    /// @brief Get the underlying `GeneralPolicy`.
    /// @return the underlying `GeneralPolicy`.
    const general_policies::GeneralPolicyImpl& get_general_policy() const;
};
}

#endif