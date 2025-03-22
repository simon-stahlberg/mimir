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

#include "mimir/common/debug.hpp"
#include "mimir/datasets/declarations.hpp"
#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/languages/description_logics/denotation_repositories.hpp"
#include "mimir/languages/description_logics/evaluation_context.hpp"
#include "mimir/languages/general_policies/declarations.hpp"
#include "mimir/languages/general_policies/rule.hpp"

#include <stack>

namespace mimir::languages::general_policies
{
class GeneralPolicyImpl
{
private:
    Index m_index;
    NamedFeatureLists<dl::BooleanTag, dl::NumericalTag> m_features;
    RuleList m_rules;

    /// @brief Create a `GeneralPolicyImpl` for the given `RuleList`.
    /// @param rules
    GeneralPolicyImpl(Index index, NamedFeatureLists<dl::BooleanTag, dl::NumericalTag> features, RuleList rules);

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

    /**
     * Solvability of StateSpace.
     */

    UnsolvabilityReason solves(const datasets::StateSpace& state_space, dl::DenotationRepositories& denotation_repositories) const;

    /// @brief Return true if and only if the `GeneralPolicyImpl` solves all the given `vertices` in the given `datasets::StateSpace`.
    /// @param state_space is the `datasets::StateSpace`.
    /// @param vertices are the indices, one for each `ClassVertex` that must be solved.
    /// @param denotation_repositories is the repository that stored feature denotations to avoid expensive recomputations.
    /// @return a reason for unsolvability or None.
    UnsolvabilityReason
    solves(const datasets::StateSpace& state_space, const graphs::VertexIndexList& vertices, dl::DenotationRepositories& denotation_repositories) const;

    template<std::ranges::forward_range VertexIndices>
        requires std::same_as<std::ranges::range_value_t<VertexIndices>, graphs::VertexIndex>
    UnsolvabilityReason
    solves(const datasets::StateSpace& state_space, const VertexIndices& vertices, dl::DenotationRepositories& denotation_repositories) const;

    /**
     * Solvability of GeneralizedStateSpace.
     */

    UnsolvabilityReason solves(const datasets::GeneralizedStateSpace& generalized_state_space, dl::DenotationRepositories& denotation_repositories) const;

    /// @brief Return true if and only if the `GeneralPolicyImpl` solves all the given `vertices` in the given `datasets::GeneralizedStateSpace`.
    /// @param generalized_state_space is the `datasets::GeneralizedStateSpace`.
    /// @param vertices are the indices, one for each `ClassVertex` that must be solved.
    /// @param denotation_repositories is the repository that stored feature denotations to avoid expensive recomputations.
    /// @return a reason for unsolvability or None.
    UnsolvabilityReason solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                               const graphs::VertexIndexList& vertices,
                               dl::DenotationRepositories& denotation_repositories) const;

    template<std::ranges::forward_range VertexIndices>
        requires std::same_as<std::ranges::range_value_t<VertexIndices>, graphs::VertexIndex>
    UnsolvabilityReason solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                               const VertexIndices& vertices,
                               dl::DenotationRepositories& denotation_repositories) const;

    /// @brief Get the index.
    /// @return the index.
    Index get_index() const;

    template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
    const NamedFeatureList<D>& get_features() const;

    /// @brief Get the features in the `GeneralPolicyImpl`.
    /// @return the features in the `GeneralPolicyImpl`.
    const NamedFeatureLists<dl::BooleanTag, dl::NumericalTag>& get_hana_features() const;

    /// @brief Get the rules in the `GeneralPolicyImpl`.
    /// @return the rules in the `GeneralPolicyImpl`.
    const RuleList& get_rules() const;

    auto identifying_members() const
    {
        return std::tuple(std::cref(get_features<dl::BooleanTag>()), std::cref(get_features<dl::NumericalTag>()), std::cref(get_rules()));
    }
};

/**
 * Implementations
 */

template<std::ranges::forward_range VertexIndices>
    requires std::same_as<std::ranges::range_value_t<VertexIndices>, graphs::VertexIndex>
GeneralPolicyImpl::UnsolvabilityReason
GeneralPolicyImpl::solves(const datasets::StateSpace& state_space, const VertexIndices& vertices, dl::DenotationRepositories& denotation_repositories) const
{
    const auto& graph = state_space->get_graph();

    using IteratorType = graphs::ProblemGraph::AdjacentVertexIndexConstIteratorType<graphs::Forward>;

    struct Entry
    {
        graphs::VertexIndex v_idx;
        IteratorType it;
        IteratorType end;
        bool has_compatible_edge;
    };

    auto visited_v_idxs = graphs::VertexIndexSet {};
    auto stack_v_idxs = graphs::VertexIndexSet {};

    DEBUG_LOG(graph);

    for (const auto& v_idx : vertices)
    {
        if (visited_v_idxs.contains(v_idx))
            continue;

        auto stack = std::stack<Entry> {};
        stack.push(Entry { v_idx,
                           graph.get_adjacent_vertex_indices<graphs::Forward>(v_idx).begin(),
                           graph.get_adjacent_vertex_indices<graphs::Forward>(v_idx).end(),
                           false });
        visited_v_idxs.insert(v_idx);
        stack_v_idxs.insert(v_idx);

        while (!stack.empty())
        {
            auto& src_entry = stack.top();
            const auto src_v_idx = src_entry.v_idx;
            const auto& src_v = graph.get_vertex(src_v_idx);
            const auto& src_problem = graphs::get_problem(src_v);
            const auto src_state = graphs::get_state(src_v);

            if (src_entry.it == src_entry.end)
            {
                /* No more dst_v_idx available. */

                if (!src_entry.has_compatible_edge && !graphs::is_goal(src_v))
                {
                    DEBUG_LOG("\nUnsolvable vertex " << src_v_idx << ": " << std::make_tuple(src_state, std::cref(*src_problem)));

                    return UnsolvabilityReason::UNSOLVABLE;
                }

                stack.pop();
                stack_v_idxs.erase(src_entry.v_idx);
            }
            else
            {
                auto src_eval_context = dl::EvaluationContext(src_state, src_problem, denotation_repositories);

                const auto dst_v_idx = *src_entry.it++;  ///< Fetch and additionally increment iterator for next iteration
                const auto& dst_v = graph.get_vertex(dst_v_idx);
                const auto& dst_problem = graphs::get_problem(dst_v);
                const auto dst_state = graphs::get_state(dst_v);
                auto dst_eval_context = dl::EvaluationContext(dst_state, dst_problem, denotation_repositories);

                const bool is_compatible = evaluate(src_eval_context, dst_eval_context);

                if (is_compatible)
                {
                    if (stack_v_idxs.contains(dst_v_idx))
                    {
                        DEBUG_LOG("\nCompatible cyclic edge " << src_v_idx << "->" << dst_v_idx << ":\n"
                                                              << std::make_tuple(src_state, std::cref(*src_problem)) << "\n"
                                                              << "  -> " << std::make_tuple(dst_state, std::cref(*dst_problem)));

                        return UnsolvabilityReason::CYCLE;
                    }

                    src_entry.has_compatible_edge = true;

                    if (!visited_v_idxs.contains(dst_v_idx))
                    {
                        DEBUG_LOG("\nCompatible edge " << src_v_idx << "->" << dst_v_idx << ":\n"
                                                       << std::make_tuple(src_state, std::cref(*src_problem)) << "\n"
                                                       << "  -> " << std::make_tuple(dst_state, std::cref(*dst_problem)));

                        stack.push(Entry { dst_v_idx,
                                           graph.get_adjacent_vertex_indices<graphs::Forward>(dst_v_idx).begin(),
                                           graph.get_adjacent_vertex_indices<graphs::Forward>(dst_v_idx).end(),
                                           false });
                        visited_v_idxs.insert(dst_v_idx);
                        stack_v_idxs.insert(dst_v_idx);
                    }
                }
                else
                {
                    DEBUG_LOG("\nIncompatible edge " << src_v_idx << "->" << dst_v_idx << ":\n"
                                                     << std::make_tuple(src_state, std::cref(*src_problem)) << "\n"
                                                     << "  -> " << std::make_tuple(dst_state, std::cref(*dst_problem)));
                }
            }
        }
    }

    return UnsolvabilityReason::NONE;
}

template<std::ranges::forward_range VertexIndices>
    requires std::same_as<std::ranges::range_value_t<VertexIndices>, graphs::VertexIndex>
GeneralPolicyImpl::UnsolvabilityReason GeneralPolicyImpl::solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                                                                 const VertexIndices& vertices,
                                                                 dl::DenotationRepositories& denotation_repositories) const
{
    const auto& class_graph = generalized_state_space->get_graph();

    using IteratorType = graphs::ClassGraph::AdjacentVertexIndexConstIteratorType<graphs::Forward>;

    struct Entry
    {
        graphs::VertexIndex v_idx;
        IteratorType it;
        IteratorType end;
        bool has_compatible_edge;
    };

    auto visited_v_idxs = graphs::VertexIndexSet {};
    auto stack_v_idxs = graphs::VertexIndexSet {};

    DEBUG_LOG(class_graph);

    for (const auto& v_idx : vertices)
    {
        if (visited_v_idxs.contains(v_idx))
            continue;

        auto stack = std::stack<Entry> {};
        stack.push(Entry { v_idx,
                           class_graph.get_adjacent_vertex_indices<graphs::Forward>(v_idx).begin(),
                           class_graph.get_adjacent_vertex_indices<graphs::Forward>(v_idx).end(),
                           false });
        visited_v_idxs.insert(v_idx);
        stack_v_idxs.insert(v_idx);

        while (!stack.empty())
        {
            auto& src_entry = stack.top();
            const auto src_v_idx = src_entry.v_idx;
            const auto& src_v = class_graph.get_vertex(src_v_idx);
            const auto& src_problem_v = generalized_state_space->get_problem_vertex(src_v);
            const auto& src_problem = graphs::get_problem(src_problem_v);
            const auto src_state = graphs::get_state(src_problem_v);

            if (src_entry.it == src_entry.end)
            {
                /* No more dst_v_idx available. */

                if (!src_entry.has_compatible_edge && !graphs::is_goal(src_problem_v))
                {
                    DEBUG_LOG("\nUnsolvable vertex " << src_v_idx << ": " << std::make_tuple(src_state, std::cref(*src_problem)));

                    return UnsolvabilityReason::UNSOLVABLE;
                }

                stack.pop();
                stack_v_idxs.erase(src_entry.v_idx);
            }
            else
            {
                auto src_eval_context = dl::EvaluationContext(src_state, src_problem, denotation_repositories);

                const auto dst_v_idx = *src_entry.it++;  ///< Fetch and additionally increment iterator for next iteration
                const auto& dst_v = class_graph.get_vertex(dst_v_idx);
                const auto& dst_problem_v = generalized_state_space->get_problem_vertex(dst_v);
                const auto& dst_problem = graphs::get_problem(dst_problem_v);
                const auto dst_state = graphs::get_state(dst_problem_v);
                auto dst_eval_context = dl::EvaluationContext(dst_state, dst_problem, denotation_repositories);

                const bool is_compatible = evaluate(src_eval_context, dst_eval_context);

                if (is_compatible)
                {
                    if (stack_v_idxs.contains(dst_v_idx))
                    {
                        DEBUG_LOG("\nCompatible cyclic edge " << src_v_idx << "->" << dst_v_idx << ":\n"
                                                              << std::make_tuple(src_state, std::cref(*src_problem)) << "\n"
                                                              << "  -> " << std::make_tuple(dst_state, std::cref(*dst_problem)));

                        return UnsolvabilityReason::CYCLE;
                    }

                    src_entry.has_compatible_edge = true;

                    if (!visited_v_idxs.contains(dst_v_idx))
                    {
                        DEBUG_LOG("\nCompatible edge " << src_v_idx << "->" << dst_v_idx << ":\n"
                                                       << std::make_tuple(src_state, std::cref(*src_problem)) << "\n"
                                                       << "  -> " << std::make_tuple(dst_state, std::cref(*dst_problem)));

                        stack.push(Entry { dst_v_idx,
                                           class_graph.get_adjacent_vertex_indices<graphs::Forward>(dst_v_idx).begin(),
                                           class_graph.get_adjacent_vertex_indices<graphs::Forward>(dst_v_idx).end(),
                                           false });
                        visited_v_idxs.insert(dst_v_idx);
                        stack_v_idxs.insert(dst_v_idx);
                    }
                }
                else
                {
                    DEBUG_LOG("\nIncompatible edge " << src_v_idx << "->" << dst_v_idx << ":\n"
                                                     << std::make_tuple(src_state, std::cref(*src_problem)) << "\n"
                                                     << "  -> " << std::make_tuple(dst_state, std::cref(*dst_problem)));
                }
            }
        }
    }

    return UnsolvabilityReason::NONE;
}

}

#endif