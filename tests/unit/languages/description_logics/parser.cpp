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
