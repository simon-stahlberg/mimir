#include "mimir/languages/description_logics/grammar.hpp"

#include "mimir/formalism/parser.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, LanguagesDescriptionLogicsGrammarTest)
{
    auto bnf_description = std::string(R"(
<concept_at-robby_state> ::= @concept_predicate_state "at-robby"
<concept_at-robby_goal> ::= @concept_predicate_goal "at-robby"
<concept_and> ::= @concept_and <concept> <concept_at-robby_goal>
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
}
}