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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_GENERAL_POLICY_DECL_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_GENERAL_POLICY_DECL_HPP_

#include "mimir/datasets/declarations.hpp"
#include "mimir/languages/description_logics/denotation_repositories.hpp"
#include "mimir/languages/general_policies/declarations.hpp"
#include "mimir/languages/general_policies/policy_graph.hpp"
#include "mimir/search/algorithms/utils.hpp"

namespace mimir::languages::general_policies
{
enum class SolvabilityStatus
{
    SOLVED = 0,
    CYCLIC = 1,
    UNSOLVABLE = 2,
};

class GeneralPolicyImpl
{
public:
    /// @brief Return true if and only if there the state pair (transition) is compatible with a `Rule` in the `GeneralPolicyImpl`.
    /// @param source_context is the source context.
    /// @param target_context is the target context.
    /// @return true if the state pair (transition) is compatible with a `Rule` in the `GeneralPolicyImpl`.
    bool evaluate(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const;

    /// @brief Accept a `IVisitor`.
    /// @param visitor the `IVisitor`.
    void accept(IVisitor& visitor) const;

    /// @brief Return true if and only if the `GeneralPolicyImpl` is structurally terminating.
    /// Source: https://aair-lab.github.io/Publications/szig_aaai11.pdf
    /// @return true if the `GeneralPolicyImpl` is structurally terminating, and false otherwise.
    bool is_terminating(Repositories& repositories) const;

    /**
     * Solvability of StateSpace.
     */

    /// @brief Return true if and only if the `GeneralPolicyImpl` solves all `graphs::ProblemVertex` in the `graphs::ProblemGraph` of the given
    /// `datasets::StateSpace`.
    /// @param state_space is the `datasets::StateSpace`.
    /// @param denotation_repositories is the repository that stores feature denotations to avoid expensive recomputations.
    /// @return is the `SolvabilityStatus`.
    SolvabilityStatus solves(const datasets::StateSpace& state_space, dl::DenotationRepositories& denotation_repositories) const;

    /// @brief Return true if and only if the `GeneralPolicyImpl` solves all `graphs::ProblemVertex` identified by the given `graphs::VertexIndexList` in the
    /// `graphs::ProblemGraph` of the given `datasets::StateSpace`.
    /// @param state_space is the `datasets::StateSpace`.
    /// @param vertices are the vertex indices, each identifying a `graphs::ClassVertex.
    /// @param denotation_repositories is the repository that stores feature denotations to avoid expensive recomputations.
    /// @return is the `SolvabilityStatus`.
    SolvabilityStatus
    solves(const datasets::StateSpace& state_space, const graphs::VertexIndexList& vertices, dl::DenotationRepositories& denotation_repositories) const;

    /// @brief Return true if and only if the `GeneralPolicyImpl` solves all `graphs::ProblemVertex` identified by the given `VertexIndices` in the
    /// `graphs::ProblemGraph` of the given `datasets::StateSpace`.
    /// @tparam VertexIndices is the type of the `std::ranges::forward_range` over `graphs::VertexIndex`.
    /// @param state_space is the `datasets::StateSpace`.
    /// @param vertices are the vertex indices, each identifying a `graphs::ProblemVertex.
    /// @param denotation_repositories is the repository that stores feature denotations to avoid expensive recomputations.
    /// @return is the `SolvabilityStatus`.
    template<std::ranges::forward_range VertexIndices>
        requires std::same_as<std::ranges::range_value_t<VertexIndices>, graphs::VertexIndex>
    SolvabilityStatus solves(const datasets::StateSpace& state_space, const VertexIndices& vertices, dl::DenotationRepositories& denotation_repositories) const;

    /**
     * Solvability of GeneralizedStateSpace.
     */

    /// @brief Return true if and only if the `GeneralPolicyImpl` solves all `graphs::ClassVertex` in the `graphs::ClassGraph` of the given
    /// `datasets::GeneralizedStateSpace`.
    /// @param generalized_state_space is the `datasets::GeneralizedStateSpace`.
    /// @param denotation_repositories is the repository that stores feature denotations to avoid expensive recomputations.
    /// @return is the `SolvabilityStatus`.
    SolvabilityStatus solves(const datasets::GeneralizedStateSpace& generalized_state_space, dl::DenotationRepositories& denotation_repositories) const;

    /// @brief Return true if and only if the `GeneralPolicyImpl` solves all `graphs::ClassVertex` identified by the given `graphs::VertexIndexList` in the
    /// `graphs::ClassGraph` of the given `datasets::GeneralizedStateSpace`.
    /// @param generalized_state_space is the `datasets::GeneralizedStateSpace`.
    /// @param vertices are the vertex indices, each identifying a `graphs::ClassVertex.
    /// @param denotation_repositories is the repository that stores feature denotations to avoid expensive recomputations.
    /// @return is the `SolvabilityStatus`.
    SolvabilityStatus solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                             const graphs::VertexIndexList& vertices,
                             dl::DenotationRepositories& denotation_repositories) const;

    /// @brief Return true if and only if the `GeneralPolicyImpl` solves all `graphs::ClassVertex` identified by the given `VertexIndices` in the
    /// `graphs::ClassGraph` of the given `datasets::GeneralizedStateSpace`.
    /// @tparam VertexIndices is the type of the `std::ranges::forward_range` over `graphs::VertexIndex`.
    /// @param generalized_state_space is the `datasets::GeneralizedStateSpace`.
    /// @param vertices are the vertex indices, each identifying a `graphs::ClassVertex.
    /// @param denotation_repositories is the repository that stores feature denotations to avoid expensive recomputations.
    /// @return is the `SolvabilityStatus`.
    template<std::ranges::forward_range VertexIndices>
        requires std::same_as<std::ranges::range_value_t<VertexIndices>, graphs::VertexIndex>
    SolvabilityStatus solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                             const VertexIndices& vertices,
                             dl::DenotationRepositories& denotation_repositories) const;

    /**
     * Solvability of Problem.
     */

    search::SearchResult find_solution(const search::SearchContext& search_context) const;

    /// @brief Get the index.
    /// @return the index.
    Index get_index() const;

    template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
    const NamedFeatureList<D>& get_features() const;

    /// @brief Get the features in the `GeneralPolicyImpl`.
    /// @return the features in the `GeneralPolicyImpl`.
    const NamedFeatureLists<dl::BooleanTag, dl::NumericalTag>& get_hana_features() const;

    const NamedFeatureVariantList& get_all_features() const;

    /// @brief Get the rules in the `GeneralPolicyImpl`.
    /// @return the rules in the `GeneralPolicyImpl`.
    const RuleList& get_rules() const;

    auto identifying_members() const
    {
        return std::tuple(std::cref(get_features<dl::BooleanTag>()), std::cref(get_features<dl::NumericalTag>()), std::cref(get_rules()));
    }

private:
    Index m_index;
    NamedFeatureLists<dl::BooleanTag, dl::NumericalTag> m_features;
    RuleList m_rules;

    // Below: add additional members if needed and initialize them in the constructor

    NamedFeatureVariantList m_all_features;

    /// @brief Create a `GeneralPolicyImpl` for the given `RuleList`.
    /// @param index is an index that identifies the `GeneralPolicyImpl` in its `Repositories`.
    /// @param features is the set of Boolean and numerical features.
    /// @param rules is the set of policy rules.
    GeneralPolicyImpl(Index index, NamedFeatureLists<dl::BooleanTag, dl::NumericalTag> features, RuleList rules);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    bool is_terminating(graphs::PolicyGraph& policy_graph, Repositories& repositories) const;

    /// @brief Return true if and only if the `GeneralPolicyImpl` solves the `graphs::ProblemVertex` identified by the given `graphs::VertexIndex` in the
    /// `graphs::ProblemGraph` of the given `datasets::StateSpace`.
    /// @param state_space is the `datasets::StateSpace`.
    /// @param vertex is the vertex index that identifies the `graphs::ProblemVertex`.
    /// @param denotation_repositories is the repository that stores feature denotations to avoid expensive recomputations.
    /// @param ref_visited_vertices are the indices of the vertices that were visited.
    /// @return is the `SolvabilityStatus`.
    SolvabilityStatus solves(const datasets::StateSpace& state_space,
                             graphs::VertexIndex vertex,
                             dl::DenotationRepositories& denotation_repositories,
                             graphs::VertexIndexSet& ref_visited_vertices) const;

    /// @brief Return true if and only if the `GeneralPolicyImpl` solves the `graphs::ClassVertex` identified by the given `graphs::VertexIndex` in the
    /// `graphs::ClassGraph` of the given `datasets::GeneralizedStateSpace`.
    /// @param generalized_state_space is the `datasets::GeneralizedStateSpace`.
    /// @param vertex is the vertex index that identifies the `graphs::ClassVertex`.
    /// @param denotation_repositories is the repository that stores feature denotations to avoid expensive recomputations.
    /// @param ref_visited_vertices are the indices of the vertices that were visited.
    /// @return is the `SolvabilityStatus`.
    SolvabilityStatus solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                             graphs::VertexIndex vertex,
                             dl::DenotationRepositories& denotation_repositories,
                             graphs::VertexIndexSet& ref_visited_vertices) const;
};

}

#endif