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

#include "mimir/languages/description_logics/grammar.hpp"

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar_visitor_formatter.hpp"

#include <gtest/gtest.h>

using namespace mimir::languages;
using namespace mimir::formalism;

namespace mimir::tests
{

TEST(MimirTests, LanguagesDescriptionLogicsGrammarTestMatchTest)
{
    auto bnf_description = std::string(R"(
    [start_symbols]
        concept ::= <concept_start>
        role ::= <role_start>

    [grammar_rules]
        <concept_start> ::= <concept>
        <role_start> ::= <role>
        <concept_at-robby_state> ::= @concept_atomic_state "at-robby"
        <concept_at-robby_goal> ::= @concept_atomic_goal "at-robby" true
        <concept_intersection> ::= @concept_intersection <concept_at-robby_state> <concept_at-robby_goal>
        <concept> ::= <concept_at-robby_state> | <concept_at-robby_goal> | <concept_intersection>
        <role_at_state> ::= @role_atomic_state "at"
        <role_at_goal> ::= @role_atomic_goal "at" true
        <role_intersection> ::= @role_intersection <role> <role_at_goal>
        <role> ::= <role_at_state> | <role_at_goal> | <role_intersection>
)");

    auto problem = ProblemImpl::create(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"));

    auto grammar = dl::grammar::Grammar(bnf_description, problem->get_domain());

    auto constructor_repositories = dl::Repositories();

    const auto predicate_at_robby = problem->get_domain()->get_name_to_predicate<FluentTag>().at("at-robby");
    const auto concept_at_robby = constructor_repositories.get_or_create_concept_atomic_state(predicate_at_robby);
    EXPECT_TRUE(grammar.test_match(concept_at_robby));

    const auto predicate_ball = problem->get_domain()->get_name_to_predicate<StaticTag>().at("ball");
    const auto concept_ball = constructor_repositories.get_or_create_concept_atomic_state(predicate_ball);
    EXPECT_FALSE(grammar.test_match(concept_ball));

    const auto concept_goal_at_robby = constructor_repositories.get_or_create_concept_atomic_goal(predicate_at_robby, true);
    const auto concept_at_robby_intersect_goal_at_robby = constructor_repositories.get_or_create_concept_intersection(concept_at_robby, concept_goal_at_robby);
    EXPECT_TRUE(grammar.test_match(concept_at_robby_intersect_goal_at_robby));

    const auto concept_goal_at_robby_intersect_at_robby = constructor_repositories.get_or_create_concept_intersection(concept_goal_at_robby, concept_at_robby);
    EXPECT_FALSE(grammar.test_match(concept_goal_at_robby_intersect_at_robby));
}

}