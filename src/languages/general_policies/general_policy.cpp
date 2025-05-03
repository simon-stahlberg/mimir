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

#include "mimir/common/debug.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/graphs/bgl/graph_algorithms.hpp"
#include "mimir/graphs/graph_properties.hpp"
#include "mimir/languages/general_policies/general_policy_decl.hpp"
#include "mimir/languages/general_policies/general_policy_impl.hpp"
#include "mimir/languages/general_policies/repositories.hpp"
#include "mimir/languages/general_policies/rule.hpp"
#include "mimir/languages/general_policies/visitor_formatter.hpp"
#include "mimir/languages/general_policies/visitor_interface.hpp"
#include "mimir/languages/general_policies/visitor_null.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::languages::general_policies
{

using mimir::operator<<;

GeneralPolicyImpl::GeneralPolicyImpl(Index index, NamedFeatureLists<dl::BooleanTag, dl::NumericalTag> features, RuleList rules) :
    m_index(index),
    m_features(features),
    m_rules(rules),
    m_all_features()
{
    boost::hana::for_each(m_features,
                          [&](const auto& pair)
                          {
                              const auto& second = boost::hana::second(pair);
                              for (const auto& feature : second)
                              {
                                  m_all_features.push_back(feature);
                              }
                          });
}

bool GeneralPolicyImpl::evaluate(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return std::any_of(get_rules().begin(), get_rules().end(), [&](auto&& arg) { return arg->evaluate(source_context, target_context); });
}

void GeneralPolicyImpl::accept(IVisitor& visitor) const { visitor.visit(this); }

bool GeneralPolicyImpl::is_terminating(Repositories& repositories) const
{
    auto policy_graph = create_policy_graph(this, repositories);
    return is_terminating(policy_graph, repositories);
}

bool GeneralPolicyImpl::is_terminating(graphs::PolicyGraph& policy_graph, Repositories& repositories) const
{
    DEBUG_LOG("policy_graph=" << std::endl << policy_graph << std::endl)

    auto tagged_graph = graphs::DirectionTaggedType(policy_graph, graphs::ForwardTag {});
    auto [num_components, component_map] = graphs::bgl::strong_components(tagged_graph);

    DEBUG_LOG("num_components=" << num_components << std::endl)
    DEBUG_LOG("component_map=" << component_map << std::endl)

    // Line 1
    if (num_components > 1)
    {
        // Line 11
        auto sccs = std::unordered_map<size_t, graphs::VertexIndexList> {};
        for (const auto& [v_idx, component] : component_map)
        {
            sccs[component].push_back(v_idx);
        }

        for (const auto& [component, v_idxs] : sccs)
        {
            auto policy_subgraph = policy_graph.compute_induced_subgraph(v_idxs);

            if (!is_terminating(policy_subgraph, repositories))
            {
                return false;
            }
        }

        return true;
    }

    // Lines 4-5

    auto f_to_finite_witnesses = std::unordered_map<NamedFeatureVariant, graphs::EdgeIndexSet> {};
    auto f_to_infinite_witnesses = std::unordered_map<NamedFeatureVariant, graphs::EdgeIndexSet> {};

    auto find_effect_witnesses_in_scc_func = [&component_map, &policy_graph](auto&& effect,
                                                                             const std::optional<graphs::EdgeIndexSet>& excluded_e_idxs) -> graphs::EdgeIndexSet
    {
        auto witnesses = graphs::EdgeIndexSet {};

        for (const auto& [e_idx, e] : policy_graph.get_edges())
        {
            if (excluded_e_idxs && excluded_e_idxs.value().contains(e_idx))
                continue;  ///< skip excluded edge.

            const auto src_component = component_map.at(e.get_source());
            const auto dst_component = component_map.at(e.get_target());

            if (src_component != dst_component)
                continue;  ///< not in scc

            if (get_effects(e).contains(effect))
                witnesses.insert(e_idx);
        }

        return witnesses;
    };

    for (const auto& feature : get_all_features())
    {
        std::visit(
            [&](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, NamedFeature<dl::BooleanTag>>)
                {
                    const auto pos = repositories.get_or_create_positive_boolean_effect(arg);
                    const auto neg = repositories.get_or_create_negative_boolean_effect(arg);

                    // Assume pos is finite witness
                    auto finite_witnesses = find_effect_witnesses_in_scc_func(pos, std::nullopt);
                    auto infinite_witnesses = find_effect_witnesses_in_scc_func(neg, finite_witnesses);

                    if (finite_witnesses.empty())
                    {
                        // Assume neg is finite witness
                        finite_witnesses = find_effect_witnesses_in_scc_func(neg, std::nullopt);
                        infinite_witnesses = find_effect_witnesses_in_scc_func(pos, finite_witnesses);
                    }

                    f_to_finite_witnesses[arg] = finite_witnesses;
                    f_to_infinite_witnesses[arg] = infinite_witnesses;
                }
                else if constexpr (std::is_same_v<T, NamedFeature<dl::NumericalTag>>)
                {
                    const auto inc = repositories.get_or_create_increase_numerical_effect(arg);
                    const auto dec = repositories.get_or_create_decrease_numerical_effect(arg);

                    // dec is finite witness
                    auto finite_witnesses = find_effect_witnesses_in_scc_func(dec, std::nullopt);
                    auto infinite_witnesses = find_effect_witnesses_in_scc_func(inc, finite_witnesses);

                    f_to_finite_witnesses[arg] = finite_witnesses;
                    f_to_infinite_witnesses[arg] = infinite_witnesses;
                }
                else
                {
                    static_assert(dependent_false<T>::value, "Unhandled feature type");
                }
            },
            feature);
    }

    DEBUG_LOG("f_to_finite_witnesses=" << f_to_finite_witnesses)
    DEBUG_LOG("f_to_infinite_witnesses=" << f_to_infinite_witnesses)

    const auto initial_num_edges = policy_graph.get_num_edges();
    auto cur_num_edges = initial_num_edges;

    do
    {
        cur_num_edges = policy_graph.get_num_edges();

        for (const auto& feature : get_all_features())
        {
            std::visit(
                [&](auto&& arg)
                {
                    if (f_to_finite_witnesses.contains(arg) && f_to_finite_witnesses.at(arg).size() > 0
                        && (!f_to_infinite_witnesses.contains(arg) || f_to_infinite_witnesses.at(arg).size() == 0))
                    {
                        for (const auto& e_idx : graphs::EdgeIndexSet(f_to_finite_witnesses.at(arg)))
                        {
                            for (auto& [f, finite_witnesses] : f_to_finite_witnesses)
                                finite_witnesses.erase(e_idx);
                            for (auto& [f, infinite_witnesses] : f_to_infinite_witnesses)
                                infinite_witnesses.erase(e_idx);

                            policy_graph.remove_edge(e_idx);

                            DEBUG_LOG("Removed edge=" << e_idx)
                        }
                    }
                },
                feature);
        }
    } while (cur_num_edges < policy_graph.get_num_edges());

    // Lines 6-7
    if (graphs::is_acyclic(policy_graph))
    {
        DEBUG_LOG("Is terminating due to acyclicity.")

        return true;
    }

    bool edges_removed = (policy_graph.get_num_edges() < initial_num_edges);

    // Line 10
    if (edges_removed)
    {
        return is_terminating(policy_graph, repositories);
    }

    // Lines 8-9
    DEBUG_LOG("Is nonterminating.")

    return false;
}

SolvabilityStatus GeneralPolicyImpl::solves(const datasets::StateSpace& state_space, dl::DenotationRepositories& denotation_repositories) const
{
    auto alive_vertex_indices = graphs::VertexIndexList {};
    for (const auto& vertex : state_space->get_graph().get_vertices())
    {
        if (is_alive(vertex))
            alive_vertex_indices.push_back(vertex.get_index());
    }

    return solves(state_space, alive_vertex_indices, denotation_repositories);
}

SolvabilityStatus GeneralPolicyImpl::solves(const datasets::StateSpace& state_space,
                                            graphs::VertexIndex v_idx,
                                            dl::DenotationRepositories& denotation_repositories,
                                            graphs::VertexIndexSet& ref_visited_vertices) const
{
    if (ref_visited_vertices.contains(v_idx))
        return SolvabilityStatus::SOLVED;

    const auto& graph = state_space->get_graph();

    using IteratorType = graphs::ProblemGraph::AdjacentVertexIndexConstIterator<graphs::ForwardTag>;

    struct Entry
    {
        graphs::VertexIndex v_idx;
        IteratorType it;
        IteratorType end;
        bool has_compatible_edge;
    };

    auto stack_v_idxs = graphs::VertexIndexSet {};

    DEBUG_LOG(graph);

    auto stack = std::stack<Entry> {};
    stack.push(Entry { v_idx,
                       graph.get_adjacent_vertex_indices<graphs::ForwardTag>(v_idx).begin(),
                       graph.get_adjacent_vertex_indices<graphs::ForwardTag>(v_idx).end(),
                       false });
    ref_visited_vertices.insert(v_idx);
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

                return SolvabilityStatus::UNSOLVABLE;
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

                    return SolvabilityStatus::CYCLIC;
                }

                src_entry.has_compatible_edge = true;

                if (!ref_visited_vertices.contains(dst_v_idx))
                {
                    DEBUG_LOG("\nCompatible edge " << src_v_idx << "->" << dst_v_idx << ":\n"
                                                   << std::make_tuple(src_state, std::cref(*src_problem)) << "\n"
                                                   << "  -> " << std::make_tuple(dst_state, std::cref(*dst_problem)));

                    stack.push(Entry { dst_v_idx,
                                       graph.get_adjacent_vertex_indices<graphs::ForwardTag>(dst_v_idx).begin(),
                                       graph.get_adjacent_vertex_indices<graphs::ForwardTag>(dst_v_idx).end(),
                                       false });
                    ref_visited_vertices.insert(dst_v_idx);
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

    return SolvabilityStatus::SOLVED;
}

SolvabilityStatus GeneralPolicyImpl::solves(const datasets::StateSpace& state_space,
                                            const graphs::VertexIndexList& vertices,
                                            dl::DenotationRepositories& denotation_repositories) const
{
    return solves<graphs::VertexIndexList>(state_space, vertices, denotation_repositories);
}

SolvabilityStatus GeneralPolicyImpl::solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                                            graphs::VertexIndex v_idx,
                                            dl::DenotationRepositories& denotation_repositories,
                                            graphs::VertexIndexSet& ref_visited_vertices) const
{
    if (ref_visited_vertices.contains(v_idx))
        return SolvabilityStatus::SOLVED;

    const auto& class_graph = generalized_state_space->get_graph();

    using IteratorType = graphs::ClassGraph::AdjacentVertexIndexConstIterator<graphs::ForwardTag>;

    struct Entry
    {
        graphs::VertexIndex v_idx;
        IteratorType it;
        IteratorType end;
        bool has_compatible_edge;
    };

    auto stack_v_idxs = graphs::VertexIndexSet {};

    DEBUG_LOG(class_graph);

    auto stack = std::stack<Entry> {};
    stack.push(Entry { v_idx,
                       class_graph.get_adjacent_vertex_indices<graphs::ForwardTag>(v_idx).begin(),
                       class_graph.get_adjacent_vertex_indices<graphs::ForwardTag>(v_idx).end(),
                       false });
    ref_visited_vertices.insert(v_idx);
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

                return SolvabilityStatus::UNSOLVABLE;
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

                    return SolvabilityStatus::CYCLIC;
                }

                src_entry.has_compatible_edge = true;

                if (!ref_visited_vertices.contains(dst_v_idx))
                {
                    DEBUG_LOG("\nCompatible edge " << src_v_idx << "->" << dst_v_idx << ":\n"
                                                   << std::make_tuple(src_state, std::cref(*src_problem)) << "\n"
                                                   << "  -> " << std::make_tuple(dst_state, std::cref(*dst_problem)));

                    stack.push(Entry { dst_v_idx,
                                       class_graph.get_adjacent_vertex_indices<graphs::ForwardTag>(dst_v_idx).begin(),
                                       class_graph.get_adjacent_vertex_indices<graphs::ForwardTag>(dst_v_idx).end(),
                                       false });
                    ref_visited_vertices.insert(dst_v_idx);
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

    return SolvabilityStatus::SOLVED;
}

SolvabilityStatus GeneralPolicyImpl::solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                                            dl::DenotationRepositories& denotation_repositories) const
{
    auto alive_vertex_indices = graphs::VertexIndexList {};
    for (const auto& vertex : generalized_state_space->get_graph().get_vertices())
    {
        if (is_alive(generalized_state_space->get_problem_vertex(vertex)))
            alive_vertex_indices.push_back(vertex.get_index());
    }

    return solves(generalized_state_space, alive_vertex_indices, denotation_repositories);
}

SolvabilityStatus GeneralPolicyImpl::solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                                            const graphs::VertexIndexList& vertices,
                                            dl::DenotationRepositories& denotation_repositories) const
{
    return solves<graphs::VertexIndexList>(generalized_state_space, vertices, denotation_repositories);
}

class GeneralPolicyPruningStrategy : public IPruningStrategy
{
public:
    bool test_prune_initial_state(State state) override { return false; }
    bool test_prune_successor_state(State state, State succ_state, bool is_new_succ) override
    {
        m_src_context.set_state(state);
        m_dst_context.set_state(state);
        return !m_policy->evaluate(m_src_context, m_dst_context);
    }

    GeneralPolicyPruningStrategy(GeneralPolicy policy, Problem problem) :
        m_policy(policy),
        m_denotation_repositories(),
        m_src_context(nullptr, problem, m_denotation_repositories),
        m_dst_context(nullptr, problem, m_denotation_repositories)
    {
    }

private:
    GeneralPolicy m_policy;
    dl::DenotationRepositories m_denotation_repositories;
    dl::EvaluationContext m_src_context;
    dl::EvaluationContext m_dst_context;
};

SearchResult GeneralPolicyImpl::find_solution(const SearchContext& search_context) const
{
    auto pruning_strategy = std::make_shared<GeneralPolicyPruningStrategy>(this, search_context->get_problem());
    return brfs::find_solution(search_context, nullptr, nullptr, nullptr, pruning_strategy);
}

Index GeneralPolicyImpl::get_index() const { return m_index; }

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
const NamedFeatureList<D>& GeneralPolicyImpl::get_features() const
{
    return boost::hana::at_key(m_features, boost::hana::type<D> {});
}

template const NamedFeatureList<dl::BooleanTag>& GeneralPolicyImpl::get_features() const;
template const NamedFeatureList<dl::NumericalTag>& GeneralPolicyImpl::get_features() const;

const NamedFeatureLists<dl::BooleanTag, dl::NumericalTag>& GeneralPolicyImpl::get_hana_features() const { return m_features; }

const NamedFeatureVariantList& GeneralPolicyImpl::get_all_features() const { return m_all_features; }

const RuleList& GeneralPolicyImpl::get_rules() const { return m_rules; }
}
