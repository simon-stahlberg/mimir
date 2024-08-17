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
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/generator.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, LanguagesDescriptionLogicsGrammarTest)
{
    auto bnf_description = std::string(R"(
<concept_at-robby_state> ::= @concept_predicate_state "at-robby"
<concept_at-robby_goal> ::= @concept_predicate_goal "at-robby"
<concept_and> ::= @concept_and <concept_at-robby_state> <concept_at-robby_goal>
<concept> ::= <concept_at-robby_state> | <concept_at-robby_goal> | <concept_and>
<role_at_state> ::= @role_predicate_state "at"
<role_at_goal> ::= @role_predicate_goal "at"
<role_and> ::= @role_and <role> <role_at_goal>
<role> ::= <role_at_state> | <role_at_goal> | <role_and>
)");

    auto parser = PDDLParser(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"));

    auto grammar = dl::grammar::Grammar(bnf_description, parser.get_domain());

    EXPECT_EQ(grammar.get_concept_rules().size(), 4);
    EXPECT_EQ(grammar.get_role_rules().size(), 4);

    auto constructor_repositories = dl::create_default_variadic_constructor_factory();

    const auto predicate_at_robby = parser.get_domain()->get_name_to_predicate<Fluent>().at("at-robby");
    const auto concept_at_robby =
        constructor_repositories.get<dl::ConceptPredicateStateFactory<Fluent>>().get_or_create<dl::ConceptPredicateStateImpl<Fluent>>(predicate_at_robby);
    EXPECT_TRUE(grammar.test_match(concept_at_robby));

    const auto predicate_ball = parser.get_domain()->get_name_to_predicate<Static>().at("ball");
    const auto concept_ball =
        constructor_repositories.get<dl::ConceptPredicateStateFactory<Static>>().get_or_create<dl::ConceptPredicateStateImpl<Static>>(predicate_ball);
    EXPECT_FALSE(grammar.test_match(concept_ball));

    const auto concept_goal_at_robby =
        constructor_repositories.get<dl::ConceptPredicateGoalFactory<Fluent>>().get_or_create<dl::ConceptPredicateGoalImpl<Fluent>>(predicate_at_robby);
    const auto concept_at_robby_and_goal_at_robby =
        constructor_repositories.get<dl::ConceptAndFactory>().get_or_create<dl::ConceptAndImpl>(concept_at_robby, concept_goal_at_robby);
    EXPECT_TRUE(grammar.test_match(concept_at_robby_and_goal_at_robby));

    const auto concept_goal_at_robby_and_at_robby =
        constructor_repositories.get<dl::ConceptAndFactory>().get_or_create<dl::ConceptAndImpl>(concept_goal_at_robby, concept_at_robby);
    EXPECT_FALSE(grammar.test_match(concept_goal_at_robby_and_at_robby));
}

}