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

#include "mimir/languages/description_logics/parser/parser.hpp"

#include "mimir/languages/description_logics/parser/parser_wrapper.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, LanguagesDescriptionLogicsParserTest)
{
    auto ast = dl::ast::Grammar();

    auto text = std::string(R"(
<concept_predicate1_state> ::= @concept_atomic_state "predicate1"
<concept_predicate1_goal> ::= @concept_atomic_goal "predicate1" false
<concept_intersection> ::= @concept_intersection <concept> <concept_predicate1_goal>
<concept> ::= <concept_predicate1_state> | <concept_predicate1_goal> | <concept_intersection>
<role_predicate1_state> ::= @role_atomic_state "predicate1"
<role_predicate1_goal> ::= @role_atomic_goal "predicate1" false
<role_intersection> ::= @role_intersection <role> <role_predicate1_goal>
<role> ::= <role_predicate1_state> | <role_predicate1_goal> | <role_intersection>
)");

    EXPECT_NO_THROW(dl::parse_ast(text, dl::grammar_parser(), ast));
}

TEST(MimirTests, LanguagesDescriptionLogicsParser2Test)
{
    auto ast = dl::ast::Grammar();

    auto text = std::string(R"(
<concept_x> ::=
    @concept_intersection
        @concept_intersection
            @concept_atomic_state "predicate1"
            @concept_atomic_goal "predicate1" false
        @concept_atomic_state "predicate2"
)");

    EXPECT_NO_THROW(dl::parse_ast(text, dl::grammar_parser(), ast));
}

}
