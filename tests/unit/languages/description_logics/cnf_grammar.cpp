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

#include "mimir/languages/description_logics/cnf_grammar.hpp"

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/languages/description_logics/cnf_grammar_visitor_formatter.hpp"
#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar.hpp"

#include <gtest/gtest.h>

using namespace mimir::languages;
using namespace mimir::formalism;

namespace mimir::tests
{

TEST(MimirTests, LanguagesDescriptionLogicsCNFGrammarParseTest)
{
    {
        /* 1. Detect undefined body nonterminal. */
        auto bnf_description = std::string(R"(
    [start_symbols]
        concept = <concept_start>
        role = <role_start>

    [grammar_rules]
        <concept_start> ::= <concept>
        <role_start> ::= <role>
        <concept_gripper_state> ::= @concept_atomic_state "gripper"
        <concept_at-robby_state> ::= @concept_atomic_state "at-robby"
        <concept_at-robby_goal> ::= @concept_atomic_goal "at-robby" true
        <concept_intersection> ::= @concept_intersection <concept_at-robby_state> <concept_at-robby_goal>
        <concept_union> ::= @concept_union <concept> <concept>
        <concept_composite> ::= @concept_intersection @concept_intersection @concept_atomic_state "at-robby" @concept_atomic_goal "at-robby" false  @concept_atomic_state "at-robby"
        <concept> ::= <concept_at-robby_state> | <concept_at-robby_goal> | <concept_intersection> | <concept_composite> | <concept_gripper_state> | <concept_union>
        <role_at_state> ::= @role_atomic_state "at"
        <role_at_goal> ::= @role_atomic_goal "at" true
        <role_intersection> ::= @role_intersection <role> <role_at_goal>
        <role> ::= <role_at_state> | <role_at_goal> | <role_intersection>
)");

        auto problem =
            ProblemImpl::create(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"));

        auto grammar = dl::grammar::Grammar(bnf_description, problem->get_domain());

        auto cnf_grammar = dl::cnf_grammar::Grammar(grammar);

        std::cout << cnf_grammar << std::endl;
    }
}

TEST(MimirTests, LanguagesDescriptionLogicsCNFGrammarFrancesTest)
{
    auto problem = ProblemImpl::create(fs::path(std::string(DATA_DIR) + "gripper/domain.pddl"), fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl"));

    auto cnf_grammar = dl::cnf_grammar::Grammar::create(dl::cnf_grammar::GrammarSpecificationEnum::FRANCES_ET_AL_AAAI2021, problem->get_domain());

    std::cout << cnf_grammar << std::endl;
}

}