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
<concept_predicate1_state> ::= @concept_predicate_state "predicate1"
<concept_predicate1_goal> ::= @concept_predicate_goal "predicate1"
<concept_and> ::= @concept_and <concept> <concept_predicate1_goal>
<concept> ::= <concept_predicate1_state> | <concept_predicate1_goal> | <concept_and>
<role_predicate1_state> ::= @role_predicate_state "predicate1"
<role_predicate1_goal> ::= @role_predicate_goal "predicate1"
<role_and> ::= @role_and <role> <role_predicate1_goal>
<role> ::= <role_predicate1_state> | <role_predicate1_goal> | <role_and>
)");

    EXPECT_NO_THROW(dl::parse_ast(text, dl::grammar_parser(), ast));
}

TEST(MimirTests, LanguagesDescriptionLogicsParser2Test)
{
    auto ast = dl::ast::Grammar();

    auto text = std::string(R"(
<concept_x> ::=
    @concept_and
        @concept_and
            @concept_predicate_state "predicate1"
            @concept_predicate_goal "predicate1"
        @concept_predicate_state "predicate2"
)");

    EXPECT_NO_THROW(dl::parse_ast(text, dl::grammar_parser(), ast));
}

}
