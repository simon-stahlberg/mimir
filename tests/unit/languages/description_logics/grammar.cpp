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
#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/description_logics/constructors.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, LanguagesDescriptionLogicsGrammarTest)
{
    auto bnf_description = std::string(R"(
<concept_at-robby_state> ::= @concept_atomic_state "at-robby"
<concept_at-robby_goal> ::= @concept_atomic_goal "at-robby" false
<concept_intersection> ::= @concept_intersection <concept_at-robby_state> <concept_at-robby_goal>
<concept> ::= <concept_at-robby_state> | <concept_at-robby_goal> | <concept_intersection>
<role_at_state> ::= @role_atomic_state "at"
<role_at_goal> ::= @role_atomic_goal "at" false
<role_intersection> ::= @role_intersection <role> <role_at_goal>
<role> ::= <role_at_state> | <role_at_goal> | <role_intersection>
)");

    auto parser = PDDLParser(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"));

    auto grammar = dl::grammar::Grammar(bnf_description, parser.get_domain());

    // EXPECT_EQ(grammar.get_primitive_production_rules<dl::Concept>().size(), 4);
    // EXPECT_EQ(grammar.get_primitive_production_rules<dl::Role>().size(), 4);
    // EXPECT_EQ(grammar.get_composite_production_rules<dl::Concept>().size(), 4);
    // EXPECT_EQ(grammar.get_composite_production_rules<dl::Role>().size(), 4);
    // EXPECT_EQ(grammar.get_alternative_rules<dl::Concept>().size(), 4);
    // EXPECT_EQ(grammar.get_alternative_rules<dl::Role>().size(), 4);

    auto constructor_repositories = dl::create_default_constructor_type_to_repository();

    const auto predicate_at_robby = parser.get_domain()->get_name_to_predicate<Fluent>().at("at-robby");
    const auto concept_at_robby =
        boost::hana::at_key(constructor_repositories, boost::hana::type<dl::ConceptAtomicStateImpl<Fluent>> {}).get_or_create(predicate_at_robby);
    // EXPECT_TRUE(grammar.test_match(concept_at_robby));

    const auto predicate_ball = parser.get_domain()->get_name_to_predicate<Static>().at("ball");
    const auto concept_ball =
        boost::hana::at_key(constructor_repositories, boost::hana::type<dl::ConceptAtomicStateImpl<Static>> {}).get_or_create(predicate_ball);
    // EXPECT_FALSE(grammar.test_match(concept_ball));

    const auto concept_goal_at_robby =
        boost::hana::at_key(constructor_repositories, boost::hana::type<dl::ConceptAtomicGoalImpl<Fluent>> {}).get_or_create(predicate_at_robby, false);
    const auto concept_at_robby_intersect_goal_at_robby =
        boost::hana::at_key(constructor_repositories, boost::hana::type<dl::ConceptIntersectionImpl> {}).get_or_create(concept_at_robby, concept_goal_at_robby);
    // EXPECT_TRUE(grammar.test_match(concept_at_robby_intersect_goal_at_robby));

    const auto concept_goal_at_robby_intersect_at_robby =
        boost::hana::at_key(constructor_repositories, boost::hana::type<dl::ConceptIntersectionImpl> {}).get_or_create(concept_goal_at_robby, concept_at_robby);
    // EXPECT_FALSE(grammar.test_match(concept_goal_at_robby_intersect_at_robby));
}

}