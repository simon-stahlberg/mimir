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
#include "mimir/languages/description_logics/denotation_repositories.hpp"
#include "mimir/languages/general_policies/declarations.hpp"

namespace mimir::languages::general_policies
{
class GeneralPolicyImpl
{
private:
    Index m_index;
    NamedFeatureLists<dl::Boolean, dl::Numerical> m_features;
    RuleList m_rules;

    /// @brief Create a `GeneralPolicyImpl` for the given `RuleList`.
    /// @param rules
    GeneralPolicyImpl(Index index, NamedFeatureLists<dl::Boolean, dl::Numerical> features, RuleList rules);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    /// @brief Return true if and only if there the state pair (transition) is compatible with a `Rule` in the `GeneralPolicyImpl`.
    /// @param source_context is the source context.
    /// @param target_context is the target context.
    /// @return true if the state pair (transition) is compatible with a `Rule` in the `GeneralPolicyImpl`.
    bool evaluate(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const;

    bool evaluate_with_debug(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const;

    /// @brief Accept a `IVisitor`.
    /// @param visitor the `IVisitor`.
    void accept(IVisitor& visitor);

    /// @brief Return true if and only if the `GeneralPolicyImpl` is structurally terminating.
    /// @return true if the `GeneralPolicyImpl` is structurally terminating, and false otherwise.
    bool is_terminating() const;

    enum class UnsolvabilityReason
    {
        NONE = 0,
        CYCLE = 1,
        UNSOLVABLE = 2,
    };

    /// @brief Return true if and only if the `GeneralPolicyImpl` solves all the given `vertices` in the given `datasets::StateSpace`.
    /// @param state_space is the `datasets::StateSpace`.
    /// @param vertices are the indices, one for each `ClassVertex` that must be solved.
    /// @param denotation_repositories is the repository that stored feature denotations to avoid expensive recomputations.
    /// @return a reason for unsolvability or None.
    UnsolvabilityReason
    solves(const datasets::StateSpace& state_space, const graphs::VertexIndexList& vertices, dl::DenotationRepositories& denotation_repositories) const;

    /// @brief Return true if and only if the `GeneralPolicyImpl` solves all the given `vertices` in the given `datasets::GeneralizedStateSpace`.
    /// @param generalized_state_space is the `datasets::GeneralizedStateSpace`.
    /// @param vertices are the indices, one for each `ClassVertex` that must be solved.
    /// @param denotation_repositories is the repository that stored feature denotations to avoid expensive recomputations.
    /// @return a reason for unsolvability or None.
    UnsolvabilityReason solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                               const graphs::VertexIndexList& vertices,
                               dl::DenotationRepositories& denotation_repositories) const;

    /// @brief Get the index.
    /// @return the index.
    Index get_index() const;

    template<dl::FeatureCategory D>
    const NamedFeatureList<D>& get_features() const;

    /// @brief Get the features in the `GeneralPolicyImpl`.
    /// @return the features in the `GeneralPolicyImpl`.
    const NamedFeatureLists<dl::Boolean, dl::Numerical>& get_hana_features() const;

    /// @brief Get the rules in the `GeneralPolicyImpl`.
    /// @return the rules in the `GeneralPolicyImpl`.
    const RuleList& get_rules() const;

    auto identifying_members() const
    {
        return std::tuple(std::cref(get_features<dl::Boolean>()), std::cref(get_features<dl::Numerical>()), std::cref(get_rules()));
    }
};
}

#endif