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

#include "mimir/languages/description_logics/refinement.hpp"

#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/description_logics/constructor_visitors_formatter.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/search/axiom_evaluators.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, LanguagesDescriptionLogicsRefinementBrfsTest)
{
    auto bnf_description = std::string(R"(
        <concept_bot>                        ::= @concept_bot
        <concept_top>                        ::= @concept_top
        <concept_ball_state>                 ::= @concept_atomic_state "ball"
        <concept_ball_goal_false>            ::= @concept_atomic_goal "ball" false
        <concept_ball_goal_true>             ::= @concept_atomic_goal "ball" true
        <concept_gripper_state>              ::= @concept_atomic_state "gripper"
        <concept_gripper_goal_false>         ::= @concept_atomic_goal "gripper" false
        <concept_gripper_goal_true>          ::= @concept_atomic_goal "gripper" true
        <concept_at-robby_state>             ::= @concept_atomic_state "at-robby"
        <concept_at-robby_goal_false>        ::= @concept_atomic_goal "at-robby" false
        <concept_at-robby_goal_true>         ::= @concept_atomic_goal "at-robby" true
        <concept_free_state>                 ::= @concept_atomic_state "free"
        <concept_free_goal_false>            ::= @concept_atomic_goal "free" false
        <concept_free_goal_true>             ::= @concept_atomic_goal "free" true
        <concept_negation>                   ::= @concept_negation <concept>
        <concept_intersection>               ::= @concept_intersection <concept> <concept>
        <concept_union>                      ::= @concept_union <concept> <concept>
        <concept_value_restriction>          ::= @concept_value_restriction <role> <concept>
        <concept_existential_quantification> ::= @concept_existential_quantification <role> <concept>
        <concept>                            ::= <concept_bot> | <concept_top>
                                                 | <concept_ball_state> | <concept_ball_goal_false> | <concept_ball_goal_true>
                                                 | <concept_at-robby_state> | <concept_at-robby_goal_false> | <concept_at-robby_goal_true>
                                                 | <concept_gripper_state> | <concept_gripper_goal_false> | <concept_gripper_goal_true>
                                                 | <concept_free_state> | <concept_free_goal_false> | <concept_free_goal_true>
                                                 | <concept_negation> | <concept_intersection> | <concept_union> | <concept_value_restriction>
                                                 | <concept_existential_quantification>
        <role_at_state>                      ::= @role_atomic_state "at"
        <role_at_goal_false>                 ::= @role_atomic_goal "at" false
        <role_at_goal_true>                  ::= @role_atomic_goal "at" true
        <role_carry_state>                   ::= @role_atomic_state "carry"
        <role_carry_goal_false>              ::= @role_atomic_goal "carry" false
        <role_carry_goal_true>               ::= @role_atomic_goal "carry" true
        <role_intersection>                  ::= @role_intersection <role> <role>
        <role>                               ::= <role_at_state> | <role_at_goal_false> | <role_at_goal_true>
                                                 | <role_carry_state> | <role_carry_goal_false> | <role_carry_goal_true>
                                                 | <role_intersection>
)");

    auto parser = PDDLParser(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"));

    auto grammar = dl::grammar::Grammar(bnf_description, parser.get_domain());

    auto options = dl::refinement_brfs::Options();
    options.verbosity = 2;
    options.max_complexity = 5;
    boost::hana::at_key(options.max_constructors, boost::hana::type<dl::Concept> {}) = std::numeric_limits<size_t>::max();
    boost::hana::at_key(options.max_constructors, boost::hana::type<dl::Role> {}) = std::numeric_limits<size_t>::max();

    auto constructor_repositories = dl::create_default_constructor_type_to_repository();

    auto applicable_action_generator = std::make_shared<LiftedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_repositories());
    auto axiom_evaluator =
        std::dynamic_pointer_cast<IAxiomEvaluator>(std::make_shared<LiftedAxiomEvaluator>(parser.get_problem(), parser.get_pddl_repositories()));
    auto state_repository = std::make_shared<StateRepository>(axiom_evaluator);
    auto state_space = StateSpace::create(parser.get_problem(), parser.get_pddl_repositories(), applicable_action_generator, state_repository);
    auto state_list = StateList();
    for (const auto& state_vertex : state_space.value().get_vertices())
    {
        state_list.push_back(get_state(state_vertex));
    }

    auto pruning_function = dl::RefinementStateListPruningFunction(*parser.get_pddl_repositories(), parser.get_problem(), state_list);

    auto result = dl::refinement_brfs::refine(parser.get_problem(), grammar, options, constructor_repositories, pruning_function);

    for (const auto& concept_ : boost::hana::at_key(result.constructors, boost::hana::type<dl::Concept> {}))
    {
        std::cout << std::make_tuple(concept_, dl::BNFFormatterVisitorTag {}) << std::endl;
    }

    for (const auto& role_ : boost::hana::at_key(result.constructors, boost::hana::type<dl::Role> {}))
    {
        std::cout << std::make_tuple(role_, dl::BNFFormatterVisitorTag {}) << std::endl;
    }

    // EXPECT_EQ(boost::hana::at_key(result.statistics.num_generated, boost::hana::type<dl::Concept> {}), 28);
    // EXPECT_EQ(boost::hana::at_key(result.statistics.num_generated, boost::hana::type<dl::Role> {}), 5);
    // EXPECT_EQ(boost::hana::at_key(result.statistics.num_pruned, boost::hana::type<dl::Concept> {}), 634);
    // EXPECT_EQ(boost::hana::at_key(result.statistics.num_pruned, boost::hana::type<dl::Role> {}), 41);
    // EXPECT_EQ(boost::hana::at_key(result.statistics.num_rejected_by_grammar, boost::hana::type<dl::Concept> {}), 84);
    // EXPECT_EQ(boost::hana::at_key(result.statistics.num_rejected_by_grammar, boost::hana::type<dl::Role> {}), 229);
}

}