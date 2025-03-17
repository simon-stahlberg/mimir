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

using namespace mimir::languages;

namespace mimir::tests
{

TEST(MimirTests, LanguagesGeneralPoliciesParserTest)
{
    {
        /* Test parsing a grammar representing explicit features. */
        auto ast = dl::ast::Grammar();
        auto text = std::string(R"(
        [boolean_features]
            <boolean_holding> ::= @boolean_nonempty @concept_atomic_state "carry"

        [numerical_features]
            <numerical_undelivered> ::=
                @boolean_nonempty 
                    @concept_negation
                        @concept_role_value_map_equality
                            @concept_atomic_state "at"
                            @concept_atomic_goal "at" true

        [policy_rules]
            { @negative_boolean_condition <boolean_holding>, @greater_numerical_condition <numerical_undelivered> } 
            -> { @positive_boolean_effect <boolean_holding }
            { @positive_boolean_condition <boolean_holding>, @greater_numerical_condition <numerical_undelivered> } 
            -> { @negative_boolean_effect <boolean_holding>, @decrease_numerical_effect <numerical_undelivered> }
        )");

        EXPECT_NO_THROW(dl::parse_ast(text, dl::grammar_parser(), ast));
    }
}

}
