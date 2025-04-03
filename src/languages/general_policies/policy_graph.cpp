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

#include "mimir/languages/general_policies/policy_graph.hpp"

#include "mimir/languages/general_policies/general_policy.hpp"
#include "mimir/languages/general_policies/named_feature.hpp"
#include "mimir/languages/general_policies/repositories.hpp"
#include "mimir/languages/general_policies/rule.hpp"
#include "mimir/languages/general_policies/visitor_formatter.hpp"
#include "mimir/languages/general_policies/visitor_null.hpp"

namespace mimir::graphs
{
std::ostream& operator<<(std::ostream& out, const PolicyVertex& vertex)
{
    out << "v_idx: " << vertex.get_index() << "\n"
        << "conditions: ";
    mimir::operator<<(out, get_conditions(vertex));

    return out;
}

std::ostream& operator<<(std::ostream& out, const PolicyEdge& edge)
{
    out << "e_idx: " << edge.get_index() << "\n"
        << "effects: ";
    mimir::operator<<(out, get_effects(edge));

    return out;
}
}

namespace mimir::languages::general_policies
{

class EffectVisitor : public NullVisitor
{
protected:
    const ConditionSet& m_src_conditions;
    const ConditionSet& m_dst_conditions;
    Repositories& m_repositories;

    bool m_result;

public:
    EffectVisitor(const ConditionSet& src_conditions, const ConditionSet& dst_conditions, Repositories& repositories) :
        m_src_conditions(src_conditions),
        m_dst_conditions(dst_conditions),
        m_repositories(repositories)
    {
    }

    void visit(PositiveBooleanEffect effect) override
    {
        m_result = m_dst_conditions.contains(m_repositories.get_or_create_positive_boolean_condition(effect->get_feature()));
    }
    void visit(NegativeBooleanEffect effect) override
    {
        m_result = m_dst_conditions.contains(m_repositories.get_or_create_negative_boolean_condition(effect->get_feature()));
    }
    void visit(UnchangedBooleanEffect effect) override
    {
        m_result = (m_src_conditions.contains(m_repositories.get_or_create_negative_boolean_condition(effect->get_feature()))
                    && m_dst_conditions.contains(m_repositories.get_or_create_negative_boolean_condition(effect->get_feature())))
                   || (m_src_conditions.contains(m_repositories.get_or_create_positive_boolean_condition(effect->get_feature()))
                       && m_dst_conditions.contains(m_repositories.get_or_create_positive_boolean_condition(effect->get_feature())));
    }
    void visit(IncreaseNumericalEffect effect) override
    {
        m_result = (m_dst_conditions.contains(m_repositories.get_or_create_greater_numerical_condition(effect->get_feature())));
    }
    void visit(DecreaseNumericalEffect effect) override
    {
        m_result = (m_src_conditions.contains(m_repositories.get_or_create_greater_numerical_condition(effect->get_feature()))
                    && (m_dst_conditions.contains(m_repositories.get_or_create_equal_numerical_condition(effect->get_feature()))
                        || m_dst_conditions.contains(m_repositories.get_or_create_greater_numerical_condition(effect->get_feature()))));
    }
    void visit(UnchangedNumericalEffect effect) override
    {
        m_result = (m_src_conditions.contains(m_repositories.get_or_create_greater_numerical_condition(effect->get_feature()))
                    && m_dst_conditions.contains(m_repositories.get_or_create_greater_numerical_condition(effect->get_feature())))
                   || (m_src_conditions.contains(m_repositories.get_or_create_equal_numerical_condition(effect->get_feature()))
                       && m_dst_conditions.contains(m_repositories.get_or_create_equal_numerical_condition(effect->get_feature())));
    }

    bool get_result() const { return m_result; }
};

static bool evaluate_conditions(const graphs::PolicyVertex& vertex, Rule rule)
{
    return std::all_of(rule->get_conditions().begin(), rule->get_conditions().end(), [&](auto&& arg) { return get_conditions(vertex).contains(arg); });
}

static bool evaluate_effects(const graphs::PolicyVertex& src_vertex, const graphs::PolicyVertex& dst_vertex, Rule rule, Repositories& repositories)
{
    return std::all_of(rule->get_effects().begin(),
                       rule->get_effects().end(),
                       [&](auto&& arg)
                       {
                           auto visitor = EffectVisitor(get_conditions(src_vertex), get_conditions(dst_vertex), repositories);
                           arg->accept(visitor);
                           return visitor.get_result();
                       });
}

static void add_vertices(GeneralPolicy policy, Repositories& repositories, graphs::PolicyGraph& graph)
{
    std::size_t num_features = policy->get_all_features().size();
    std::size_t num_combinations = 1ULL << num_features;

    for (std::size_t mask = 0; mask < num_combinations; ++mask)
    {
        auto conditions = ConditionSet {};

        for (std::size_t i = 0; i < num_features; ++i)
        {
            bool bit = (mask >> i) & 1;

            std::visit(
                [&](const auto& feature)
                {
                    using T = std::decay_t<decltype(feature)>;

                    if constexpr (std::is_same_v<T, NamedFeature<dl::BooleanTag>>)
                    {
                        conditions.insert(bit ? repositories.get_or_create_positive_boolean_condition(feature) :
                                                repositories.get_or_create_negative_boolean_condition(feature));
                    }
                    else if constexpr (std::is_same_v<T, NamedFeature<dl::NumericalTag>>)
                    {
                        conditions.insert(bit ? repositories.get_or_create_greater_numerical_condition(feature) :
                                                repositories.get_or_create_equal_numerical_condition(feature));
                    }
                    else
                    {
                        static_assert(dependent_false<T>::value, "Unhandled feature type");
                    }
                },
                policy->get_all_features()[i]);
        }

        graph.add_vertex(conditions);
    }
}

static void add_edges(GeneralPolicy policy, Repositories& repositories, graphs::PolicyGraph& graph)
{
    for (const auto& [v1_idx, v1] : graph.get_vertices())
    {
        for (const auto& rule : policy->get_rules())
        {
            if (evaluate_conditions(v1, rule))
            {
                for (const auto& [v2_idx, v2] : graph.get_vertices())
                {
                    if (evaluate_effects(v1, v2, rule, repositories))
                    {
                        graph.add_directed_edge(v1_idx, v2_idx, EffectSet(rule->get_effects().begin(), rule->get_effects().end()));
                    }
                }
            }
        }
    }
}

graphs::PolicyGraph create_policy_graph(GeneralPolicy policy, Repositories& repositories)
{
    auto graph = graphs::PolicyGraph {};

    add_vertices(policy, repositories, graph);
    add_edges(policy, repositories, graph);

    return graph;
}
}
