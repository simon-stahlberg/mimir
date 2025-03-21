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

#include "mimir/formalism/problem.hpp"
#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/general_policies/repositories.hpp"

#include <gtest/gtest.h>

using namespace mimir::formalism;
using namespace mimir::languages;
using namespace mimir::search;
using namespace mimir::datasets;

namespace mimir::tests
{

TEST(MimirTests, LanguagesGeneralPoliciesGeneralPolicyTest)
{
    {
        // Gripper policy from Frances-et-al-aaai2021: https://arxiv.org/pdf/2101.00692
        auto description = std::string(R"(
        [boolean_features]
            <boolean_robby_at_b> ::= @boolean_nonempty @concept_existential_quantification @role_atomic_goal "at" false @concept_atomic_state "at-robby"

        [numerical_features]
            <numerical_carry> ::= @numerical_count @concept_existential_quantification @role_atomic_state "carry" @concept_top
            <numerical_undelivered> ::=
                @numerical_count 
                    @concept_negation
                        @concept_role_value_map_equality
                            @role_atomic_state "at"
                            @role_atomic_goal "at" false

        [policy_rules]
            { @negative_boolean_condition <boolean_robby_at_b>, @equal_numerical_condition <numerical_carry>, @greater_numerical_condition <numerical_undelivered> } 
            -> { @increase_numerical_effect <numerical_carry> }
            { @positive_boolean_condition <boolean_robby_at_b>, @equal_numerical_condition <numerical_carry>, @greater_numerical_condition <numerical_undelivered> } 
            -> { @negative_boolean_effect <boolean_robby_at_b> }
            { @positive_boolean_condition <boolean_robby_at_b>, @greater_numerical_condition <numerical_carry>, @greater_numerical_condition <numerical_undelivered> } 
            -> { @decrease_numerical_effect <numerical_carry>, @decrease_numerical_effect <numerical_undelivered> }
            { @negative_boolean_condition <boolean_robby_at_b>, @greater_numerical_condition <numerical_carry>, @greater_numerical_condition <numerical_undelivered> } 
            -> { @positive_boolean_effect <boolean_robby_at_b> }
        )");

        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem1_file = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
        const auto problem2_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

        auto context = search::GeneralizedSearchContext(domain_file, std::vector<fs::path> { problem1_file, problem2_file });

        auto options = GeneralizedStateSpace::Options();
        options.problem_options.symmetry_pruning = true;
        const auto generalized_state_space = GeneralizedStateSpace(context, options);

        auto vertex_indices =
            graphs::VertexIndexList(generalized_state_space.get_initial_vertices().begin(), generalized_state_space.get_initial_vertices().end());

        auto denotation_repositories = dl::DenotationRepositories();

        auto repositories = general_policies::Repositories();
        auto dl_repositories = dl::Repositories();

        const auto general_policy = repositories.get_or_create_general_policy(description, *context.get_domain(), dl_repositories);

        EXPECT_EQ(generalized_state_space.get_initial_vertices().size(), 2);

        EXPECT_EQ(general_policy->solves(generalized_state_space, vertex_indices, denotation_repositories),
                  general_policies::GeneralPolicyImpl::UnsolvabilityReason::NONE);
    }
}

}
