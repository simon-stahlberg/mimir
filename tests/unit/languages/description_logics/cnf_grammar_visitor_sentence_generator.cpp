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

#include "mimir/languages/description_logics/cnf_grammar_visitor_sentence_generator.hpp"

#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/datasets/knowledge_base.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/languages/description_logics/cnf_grammar.hpp"
#include "mimir/languages/description_logics/cnf_grammar_visitor_formatter.hpp"
#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/description_logics/constructor_visitor_formatter.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/generalized_search_context.hpp"

#include <gtest/gtest.h>

using namespace mimir::languages;
using namespace mimir::formalism;
using namespace mimir::datasets;

namespace mimir::tests
{

TEST(MimirTests, LanguagesDescriptionLogicsCNFGrammarVisitorSentenceGeneratorTest)
{
    auto bnf_description = std::string(R"(
    [start_symbols]
        concept = <concept_start>
        role = <role_start>

    [grammar_rules]
        <concept_start>                      ::= <concept>
        <role_start>                         ::= <role>
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

    /* Test two spanner problems with two locations and a single spanner each. */
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    // The spanner is at location 1.
    const auto problem1_file = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    // The spanner is at location 2.
    const auto problem2_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    auto context = search::GeneralizedSearchContextImpl::create(domain_file, std::vector<fs::path> { problem1_file, problem2_file });

    auto kb_options = KnowledgeBaseImpl::Options();

    auto& state_space_options = kb_options.state_space_options;
    state_space_options.symmetry_pruning = false;

    auto& generalized_state_space_options = kb_options.generalized_state_space_options;
    generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
    generalized_state_space_options->symmetry_pruning = false;

    auto kb = KnowledgeBaseImpl::create(context, kb_options);

    auto pruning_function = dl::RefinementStateListPruningFunction(kb->get_generalized_state_space().value());

    dl::cnf_grammar::GeneratedSentencesContainer sentences;
    dl::Repositories repositories;
    size_t max_complexity = 4;
    auto visitor = dl::cnf_grammar::GeneratorVisitor(pruning_function, sentences, repositories, max_complexity);

    auto grammar = dl::grammar::Grammar(bnf_description, kb->get_domain());

    auto cnf_grammar = dl::cnf_grammar::Grammar(grammar);

    std::cout << cnf_grammar << std::endl;

    visitor.visit(cnf_grammar);

    const auto& concept_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::ConceptTag> {});

    EXPECT_EQ(concept_statistics.num_generated, 232);
    EXPECT_EQ(concept_statistics.num_kept, 22);
    EXPECT_EQ(concept_statistics.num_pruned, 210);

    const auto& role_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::RoleTag> {});

    EXPECT_EQ(role_statistics.num_generated, 22);
    EXPECT_EQ(role_statistics.num_kept, 5);
    EXPECT_EQ(role_statistics.num_pruned, 17);
}

TEST(MimirTests, LanguagesDescriptionLogicsCNFGrammarVisitorSentenceGeneratorFrancesEtAlAAAI2021Test)
{
    /* Test two spanner problems with two locations and a single spanner each. */
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    // The spanner is at location 1.
    const auto problem1_file = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    // The spanner is at location 2.
    const auto problem2_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    auto context = search::GeneralizedSearchContextImpl::create(domain_file, std::vector<fs::path> { problem1_file, problem2_file });

    auto kb_options = KnowledgeBaseImpl::Options();

    auto& state_space_options = kb_options.state_space_options;
    state_space_options.symmetry_pruning = false;

    auto& generalized_state_space_options = kb_options.generalized_state_space_options;
    generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
    generalized_state_space_options->symmetry_pruning = false;

    auto kb = KnowledgeBaseImpl::create(context, kb_options);

    auto pruning_function = dl::RefinementStateListPruningFunction(kb->get_generalized_state_space().value());

    dl::cnf_grammar::GeneratedSentencesContainer sentences;
    dl::Repositories repositories;
    size_t max_complexity = 9;
    auto visitor = dl::cnf_grammar::GeneratorVisitor(pruning_function, sentences, repositories, max_complexity);

    auto cnf_grammar = dl::cnf_grammar::Grammar::create(dl::cnf_grammar::GrammarSpecificationEnum::FRANCES_ET_AL_AAAI2021, kb->get_domain());

    std::cout << cnf_grammar << std::endl;

    visitor.visit(cnf_grammar);

    const auto& concept_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::ConceptTag> {});

    EXPECT_EQ(concept_statistics.num_generated, 4063);
    EXPECT_EQ(concept_statistics.num_kept, 204);
    EXPECT_EQ(concept_statistics.num_pruned, 3859);

    const auto& role_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::RoleTag> {});

    EXPECT_EQ(role_statistics.num_generated, 38);
    EXPECT_EQ(role_statistics.num_kept, 9);
    EXPECT_EQ(role_statistics.num_pruned, 29);

    const auto& boolean_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::BooleanTag> {});

    EXPECT_EQ(boolean_statistics.num_generated, 173);
    EXPECT_EQ(boolean_statistics.num_kept, 38);
    EXPECT_EQ(boolean_statistics.num_pruned, 135);

    const auto& numerical_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::NumericalTag> {});

    EXPECT_EQ(numerical_statistics.num_generated, 2799);
    EXPECT_EQ(numerical_statistics.num_kept, 203);
    EXPECT_EQ(numerical_statistics.num_pruned, 2596);
}

}