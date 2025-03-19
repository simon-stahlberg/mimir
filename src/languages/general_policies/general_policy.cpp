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

#include "mimir/languages/general_policies/general_policy.hpp"

#include "mimir/languages/description_logics/evaluation_context.hpp"
#include "mimir/languages/general_policies/rule.hpp"
#include "mimir/languages/general_policies/visitor_interface.hpp"
#include "mimir/search/state.hpp"

#include <queue>

namespace mimir::languages::general_policies
{

GeneralPolicyImpl::GeneralPolicyImpl(Index index, NamedFeatureLists<dl::Boolean, dl::Numerical> features, RuleList rules) :
    m_index(index),
    m_features(features),
    m_rules(rules)
{
}

bool GeneralPolicyImpl::evaluate(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return std::any_of(get_rules().begin(), get_rules().end(), [&](auto&& arg) { return arg->evaluate(source_context, target_context); });
}

bool GeneralPolicyImpl::evaluate_with_debug(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return std::any_of(get_rules().begin(), get_rules().end(), [&](auto&& arg) { return arg->evaluate_with_debug(source_context, target_context); });
}

void GeneralPolicyImpl::accept(IVisitor& visitor) { visitor.visit(this); }

bool GeneralPolicyImpl::is_terminating() const { throw std::runtime_error("Not implemented"); }

GeneralPolicyImpl::UnsolvabilityReason GeneralPolicyImpl::solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                                                                 const graphs::VertexIndexList& vertices,
                                                                 dl::DenotationRepositories& denotation_repositories) const
{
    auto queue = std::deque<graphs::VertexIndex> {};

    auto visited_v_idxs = graphs::VertexIndexSet {};

    const auto& class_graph = generalized_state_space.get_graph();

    // TODO: need to switch to DFS to detect cycles!

    for (const auto& v_idx : vertices)
    {
        if (visited_v_idxs.contains(v_idx))
            continue;

        queue.clear();
        queue.emplace_back(v_idx);

        while (!queue.empty())
        {
            const auto src_v_idx = queue.front();
            queue.pop_front();

            const auto& src_v = class_graph.get_vertex(src_v_idx);

            if (graphs::is_goal(src_v))
            {
                continue;
            }

            const auto& src_problem = generalized_state_space.get_problem(src_v);
            const auto& src_problem_v = generalized_state_space.get_problem_vertex(src_v);
            const auto src_state = graphs::get_state(src_problem_v);
            auto src_eval_context = dl::EvaluationContext(src_state, src_problem, denotation_repositories);

            std::cout << std::endl;
            std::cout << "Src " << std::make_tuple(src_state, std::cref(*src_problem)) << std::endl;

            bool src_has_compatible_edge = false;

            for (const auto& dst_v_idx : class_graph.get_adjacent_vertex_indices<graphs::Forward>(src_v_idx))
            {
                const auto& dst_v = class_graph.get_vertex(dst_v_idx);
                const auto& dst_problem = generalized_state_space.get_problem(dst_v);
                const auto& dst_problem_v = generalized_state_space.get_problem_vertex(dst_v);
                const auto dst_state = graphs::get_state(dst_problem_v);
                auto dst_eval_context = dl::EvaluationContext(dst_state, dst_problem, denotation_repositories);

                const bool is_compatible = evaluate(src_eval_context, dst_eval_context);

                if (is_compatible)
                {
                    if (visited_v_idxs.contains(dst_v_idx))
                    {
                        std::cout << "Src has cyclic compatible edge " << std::make_tuple(dst_state, std::cref(*dst_problem)) << std::endl;
                        return UnsolvabilityReason::CYCLE;
                    }

                    src_has_compatible_edge = true;

                    queue.emplace_back(dst_v_idx);
                    visited_v_idxs.insert(dst_v_idx);

                    std::cout << "Compatible edge " << std::make_tuple(dst_state, std::cref(*dst_problem)) << std::endl;
                }
                else
                {
                    // std::cout << "Incompatible edge " << std::make_tuple(dst_state, std::cref(*dst_problem)) << std::endl;
                }
            }

            if (!src_has_compatible_edge)
            {
                std::cout << "Src has no compatible edge" << std::endl;
                return UnsolvabilityReason::UNSOLVABLE;
            }
        }
    }

    return UnsolvabilityReason::NONE;
}

Index GeneralPolicyImpl::get_index() const { return m_index; }

template<dl::FeatureCategory D>
const NamedFeatureList<D>& GeneralPolicyImpl::get_features() const
{
    return boost::hana::at_key(m_features, boost::hana::type<D> {});
}

template const NamedFeatureList<dl::Boolean>& GeneralPolicyImpl::get_features() const;
template const NamedFeatureList<dl::Numerical>& GeneralPolicyImpl::get_features() const;

const NamedFeatureLists<dl::Boolean, dl::Numerical>& GeneralPolicyImpl::get_hana_features() const { return m_features; }

const RuleList& GeneralPolicyImpl::get_rules() const { return m_rules; }

}
