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

#include "policy_graph.hpp"

#include "mimir/languages/general_policies/general_policy.hpp"
#include "mimir/languages/general_policies/named_feature.hpp"
#include "mimir/languages/general_policies/repositories.hpp"
#include "mimir/languages/general_policies/rule.hpp"
#include "mimir/languages/general_policies/visitor_null.hpp"

namespace mimir::languages::general_policies
{

static bool evaluate_conditions(const graphs::PolicyVertex& vertex, Rule rule)
{
    return std::all_of(rule->get_conditions().begin(), rule->get_conditions().end(), [&](auto&& arg) { return get_conditions(vertex).contains(arg); });
}

static bool evaluate_effects(const graphs::PolicyVertex& vertex, Rule rule) {}

graphs::PolicyGraph create(GeneralPolicy policy, Repositories& repositories)
{
    using FeatureVariant = std::variant<NamedFeature<dl::BooleanTag>, NamedFeature<dl::NumericalTag>>;

    auto all_features = std::vector<FeatureVariant> {};

    boost::hana::for_each(policy->get_hana_features(),
                          [&](auto pair)
                          {
                              const auto& second = boost::hana::second(pair);

                              for (const auto& feature : second)
                              {
                                  all_features.emplace_back(feature);
                              }
                          });

    std::size_t num_features = all_features.size();
    std::size_t num_combinations = 1ULL << num_features;

    auto graph = graphs::PolicyGraph {};

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
                all_features[i]);
        }

        graph.add_vertex(conditions);
    }

    for (const auto& [v1_idx, v1] : graph.get_vertices())
    {
        for (const auto& rule : policy->get_rules())
        {
            if (std::all_of(rule->get_conditions().begin(), rule->get_conditions().end(), [&](auto&& arg) { return get_conditions(v1).contains(arg); }))
            {
                for (const auto& [v2_idx, v2] : graph.get_vertices()) {}
            }
        }
    }
}
}
