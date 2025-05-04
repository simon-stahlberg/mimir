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

#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/datasets/knowledge_base.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/general_policies/general_policy_factory.hpp"
#include "mimir/languages/general_policies/repositories.hpp"
#include "mimir/search/generalized_search_context.hpp"

#include <gtest/gtest.h>

using namespace mimir::formalism;
using namespace mimir::languages;
using namespace mimir::search;
using namespace mimir::datasets;

namespace mimir::tests
{

TEST(MimirTests, LanguagesGeneralPoliciesGeneralPolicyGripperTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem1_file = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
        const auto problem2_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

        auto context = search::GeneralizedSearchContextImpl::create(domain_file, std::vector<fs::path> { problem1_file, problem2_file });

        auto denotation_repositories = dl::DenotationRepositories();

        auto repositories = general_policies::Repositories();
        auto dl_repositories = dl::Repositories();

        const auto general_policy =
            general_policies::GeneralPolicyFactory::get_or_create_general_policy_gripper(*context->get_domain(), repositories, dl_repositories);

        EXPECT_TRUE(general_policy->is_terminating(repositories));

        {
            /* Without symmetry reduction. */

            auto kb_options = KnowledgeBaseImpl::Options();
            auto& state_space_options = kb_options.state_space_options;
            state_space_options.symmetry_pruning = false;
            auto& generalized_state_space_options = kb_options.generalized_state_space_options;
            generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
            auto kb = KnowledgeBaseImpl::create(context, kb_options);

            const auto& generalized_state_space = kb->get_generalized_state_space();

            {
                /* Test solvability on GeneralizedStateSpace. */

                EXPECT_EQ(general_policy->solves(generalized_state_space.value(), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 0. */

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 1. */

                const auto state_space_1 = kb->get_state_spaces().at(1);

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }
        }

        {
            /* With symmetry reduction. */

            auto kb_options = KnowledgeBaseImpl::Options();
            auto& state_space_options = kb_options.state_space_options;
            state_space_options.symmetry_pruning = true;
            auto& generalized_state_space_options = kb_options.generalized_state_space_options;
            generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
            auto kb = KnowledgeBaseImpl::create(context, kb_options);

            const auto& generalized_state_space = kb->get_generalized_state_space();

            {
                /* Test solvability on GeneralizedStateSpace. */

                EXPECT_EQ(general_policy->solves(generalized_state_space.value(), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 0. */

                const auto state_space_0 = kb->get_state_spaces().at(0);

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 1. */

                const auto state_space_1 = kb->get_state_spaces().at(1);

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }
        }
    }
}

TEST(MimirTests, LanguagesGeneralPoliciesGeneralPolicyBlocks3opsTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "blocks_3/domain.pddl");
        const auto problem1_file = fs::path(std::string(DATA_DIR) + "blocks_3/test_problem.pddl");
        const auto problem2_file = fs::path(std::string(DATA_DIR) + "blocks_3/test_problem2.pddl");

        auto context = search::GeneralizedSearchContextImpl::create(domain_file, std::vector<fs::path> { problem1_file, problem2_file });

        auto denotation_repositories = dl::DenotationRepositories();

        auto repositories = general_policies::Repositories();
        auto dl_repositories = dl::Repositories();

        const auto general_policy =
            general_policies::GeneralPolicyFactory::get_or_create_general_policy_blocks3ops(*context->get_domain(), repositories, dl_repositories);

        EXPECT_FALSE(general_policy->is_terminating(repositories));

        {
            /* Without symmetry reduction. */

            auto kb_options = KnowledgeBaseImpl::Options();
            auto& state_space_options = kb_options.state_space_options;
            state_space_options.symmetry_pruning = false;
            auto& generalized_state_space_options = kb_options.generalized_state_space_options;
            generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
            auto kb = KnowledgeBaseImpl::create(context, kb_options);

            const auto& generalized_state_space = kb->get_generalized_state_space();

            {
                /* Test solvability on GeneralizedStateSpace. */

                EXPECT_EQ(general_policy->solves(generalized_state_space.value(), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 0. */

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 1. */

                const auto state_space_1 = kb->get_state_spaces().at(1);

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }
        }

        {
            /* With symmetry reduction. */

            auto kb_options = KnowledgeBaseImpl::Options();
            auto& state_space_options = kb_options.state_space_options;
            state_space_options.symmetry_pruning = true;
            auto& generalized_state_space_options = kb_options.generalized_state_space_options;
            generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
            auto kb = KnowledgeBaseImpl::create(context, kb_options);

            const auto& generalized_state_space = kb->get_generalized_state_space();

            {
                /* Test solvability on GeneralizedStateSpace. */

                EXPECT_EQ(general_policy->solves(generalized_state_space.value(), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 0. */

                const auto state_space_0 = kb->get_state_spaces().at(0);

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 1. */

                const auto state_space_1 = kb->get_state_spaces().at(1);

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }
        }
    }
}

TEST(MimirTests, LanguagesGeneralPoliciesGeneralPolicySpannerTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "spanner/domain.pddl");
        const auto problem1_file = fs::path(std::string(DATA_DIR) + "spanner/p-1-1-2-1.pddl");
        const auto problem2_file = fs::path(std::string(DATA_DIR) + "spanner/p-1-1-2-1(2).pddl");

        auto context = search::GeneralizedSearchContextImpl::create(domain_file, std::vector<fs::path> { problem1_file, problem2_file });

        auto denotation_repositories = dl::DenotationRepositories();

        auto repositories = general_policies::Repositories();
        auto dl_repositories = dl::Repositories();

        const auto general_policy =
            general_policies::GeneralPolicyFactory::get_or_create_general_policy_spanner(*context->get_domain(), repositories, dl_repositories);

        EXPECT_FALSE(general_policy->is_terminating(repositories));

        {
            /* Without symmetry reduction. */

            auto kb_options = KnowledgeBaseImpl::Options();
            auto& state_space_options = kb_options.state_space_options;
            state_space_options.symmetry_pruning = false;
            auto& generalized_state_space_options = kb_options.generalized_state_space_options;
            generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
            auto kb = KnowledgeBaseImpl::create(context, kb_options);

            const auto& generalized_state_space = kb->get_generalized_state_space();

            {
                /* Test solvability on GeneralizedStateSpace. */

                EXPECT_EQ(general_policy->solves(generalized_state_space.value(), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 0. */

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 1. */

                const auto state_space_1 = kb->get_state_spaces().at(1);

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }
        }

        {
            /* With symmetry reduction. */

            auto kb_options = KnowledgeBaseImpl::Options();
            auto& state_space_options = kb_options.state_space_options;
            state_space_options.symmetry_pruning = true;
            auto& generalized_state_space_options = kb_options.generalized_state_space_options;
            generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
            auto kb = KnowledgeBaseImpl::create(context, kb_options);

            const auto& generalized_state_space = kb->get_generalized_state_space();

            {
                /* Test solvability on GeneralizedStateSpace. */

                EXPECT_EQ(general_policy->solves(generalized_state_space.value(), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 0. */

                const auto state_space_0 = kb->get_state_spaces().at(0);

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 1. */

                const auto state_space_1 = kb->get_state_spaces().at(1);

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }
        }
    }
}

TEST(MimirTests, LanguagesGeneralPoliciesGeneralPolicyDeliveryTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "delivery/domain.pddl");
        const auto problem1_file = fs::path(std::string(DATA_DIR) + "delivery/test_problem.pddl");
        const auto problem2_file = fs::path(std::string(DATA_DIR) + "delivery/test_problem2.pddl");

        auto context = search::GeneralizedSearchContextImpl::create(domain_file, std::vector<fs::path> { problem1_file, problem2_file });

        auto denotation_repositories = dl::DenotationRepositories();

        auto repositories = general_policies::Repositories();
        auto dl_repositories = dl::Repositories();

        const auto general_policy =
            general_policies::GeneralPolicyFactory::get_or_create_general_policy_delivery(*context->get_domain(), repositories, dl_repositories);

        EXPECT_TRUE(general_policy->is_terminating(repositories));

        {
            /* Without symmetry reduction. */

            auto kb_options = KnowledgeBaseImpl::Options();
            auto& state_space_options = kb_options.state_space_options;
            state_space_options.symmetry_pruning = false;
            auto& generalized_state_space_options = kb_options.generalized_state_space_options;
            generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
            auto kb = KnowledgeBaseImpl::create(context, kb_options);

            const auto& generalized_state_space = kb->get_generalized_state_space();

            {
                /* Test solvability on GeneralizedStateSpace. */

                EXPECT_EQ(general_policy->solves(generalized_state_space.value(), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 0. */

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 1. */

                const auto state_space_1 = kb->get_state_spaces().at(1);

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }
        }

        {
            /* With symmetry reduction. */

            auto kb_options = KnowledgeBaseImpl::Options();
            auto& state_space_options = kb_options.state_space_options;
            state_space_options.symmetry_pruning = true;
            auto& generalized_state_space_options = kb_options.generalized_state_space_options;
            generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
            auto kb = KnowledgeBaseImpl::create(context, kb_options);

            const auto& generalized_state_space = kb->get_generalized_state_space();

            {
                /* Test solvability on GeneralizedStateSpace. */

                EXPECT_EQ(general_policy->solves(generalized_state_space.value(), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 0. */

                const auto state_space_0 = kb->get_state_spaces().at(0);

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }

            {
                /* Test solvability on StateSpace 1. */

                const auto state_space_1 = kb->get_state_spaces().at(1);

                EXPECT_EQ(general_policy->solves(kb->get_state_spaces().at(0), denotation_repositories), general_policies::SolvabilityStatus::SOLVED);
            }
        }
    }
}

TEST(MimirTests, LanguagesGeneralPoliciesGeneralPolicyGripperTerminationTest)
{
    {
        const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
        const auto problem1_file = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
        const auto problem2_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

        auto context = search::GeneralizedSearchContextImpl::create(domain_file, std::vector<fs::path> { problem1_file, problem2_file });

        auto denotation_repositories = dl::DenotationRepositories();

        auto repositories = general_policies::Repositories();
        auto dl_repositories = dl::Repositories();

        {
            const auto general_policy = repositories.get_or_create_general_policy(std::string(R"(
            [boolean_features]
[numerical_features]
<n2> ::= @numerical_count @role_atomic_state "at"
<n82> ::= @numerical_distance @concept_existential_quantification @role_atomic_goal "at" true @concept_atomic_state "at-robby" @role_restriction @role_atomic_goal "at" true @concept_atomic_state "at-robby" @concept_role_value_map_equality @role_atomic_state "at" @role_atomic_goal "at" true
[policy_rules]
{ @greater_numerical_condition <n2>, @greater_numerical_condition <n82> } -> { @decrease_numerical_effect <n2>, @unchanged_numerical_effect <n82> }
{ @equal_numerical_condition <n2>, @greater_numerical_condition <n82> } -> { @unchanged_numerical_effect <n2>, @decrease_numerical_effect <n82> }
{ @equal_numerical_condition <n2>, @greater_numerical_condition <n82> } -> { @increase_numerical_effect <n2>, @decrease_numerical_effect <n82> }
)"),
                                                                                  *context->get_domain(),
                                                                                  dl_repositories);

            EXPECT_TRUE(general_policy->is_terminating(repositories));
        }

        {
            const auto general_policy = repositories.get_or_create_general_policy(std::string(R"(
            [boolean_features]
<b0> ::= @boolean_nonempty @role_atomic_state "carry"
<b2> ::= @boolean_nonempty @role_atomic_state "at"
<b7> ::= @boolean_nonempty @concept_existential_quantification @role_atomic_goal "at" true @concept_atomic_state "at-robby"
[numerical_features]
[policy_rules]
{ @negative_boolean_condition <b0>, @positive_boolean_condition <b2>, @positive_boolean_condition <b7> } -> { @unchanged_boolean_effect <b0>, @unchanged_boolean_effect <b2>, @negative_boolean_effect <b7> }
{ @positive_boolean_condition <b0>, @negative_boolean_condition <b2>, @negative_boolean_condition <b7> } -> { @unchanged_boolean_effect <b0>, @unchanged_boolean_effect <b2>, @positive_boolean_effect <b7> }
{ @positive_boolean_condition <b0>, @positive_boolean_condition <b2>, @positive_boolean_condition <b7> } -> { @negative_boolean_effect <b0>, @unchanged_boolean_effect <b2>, @unchanged_boolean_effect <b7> }
{ @negative_boolean_condition <b0>, @positive_boolean_condition <b2>, @negative_boolean_condition <b7> } -> { @positive_boolean_effect <b0>, @unchanged_boolean_effect <b2>, @unchanged_boolean_effect <b7> }
{ @positive_boolean_condition <b0>, @negative_boolean_condition <b2>, @positive_boolean_condition <b7> } -> { @unchanged_boolean_effect <b0>, @positive_boolean_effect <b2>, @unchanged_boolean_effect <b7> }
{ @positive_boolean_condition <b0>, @positive_boolean_condition <b2>, @negative_boolean_condition <b7> } -> { @unchanged_boolean_effect <b0>, @unchanged_boolean_effect <b2>, @positive_boolean_effect <b7> }
)"),
                                                                                  *context->get_domain(),
                                                                                  dl_repositories);

            EXPECT_TRUE(!general_policy->is_terminating(repositories));
        }
    }
}

}
