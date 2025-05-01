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
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem1_file = fs::path(std::string(DATA_DIR) + "gripper/p-1-0.pddl");
    const auto problem2_file = fs::path(std::string(DATA_DIR) + "gripper/p-2-0.pddl");

    auto context = search::GeneralizedSearchContextImpl::create(domain_file, std::vector<fs::path> { problem1_file, problem2_file });

    auto kb_options = KnowledgeBaseImpl::Options();
    auto& state_space_options = kb_options.state_space_options;
    state_space_options.symmetry_pruning = false;
    auto& generalized_state_space_options = kb_options.generalized_state_space_options;
    generalized_state_space_options = GeneralizedStateSpaceImpl::Options();
    auto kb = KnowledgeBaseImpl::create(context, kb_options);

    {
        /**
         * COMPLETE
         */

        auto denotation_repositories = dl::DenotationRepositories();
        auto pruning_function = dl::StateListRefinementPruningFunction(kb->get_generalized_state_space().value(), denotation_repositories);
        auto sentences = dl::cnf_grammar::GeneratedSentencesContainer();
        auto repositories = dl::Repositories();
        size_t max_complexity = 5;
        auto visitor = dl::cnf_grammar::GeneratorVisitor(pruning_function, sentences, repositories, max_complexity);

        auto cnf_grammar = dl::cnf_grammar::Grammar::create(dl::cnf_grammar::GrammarSpecificationEnum::COMPLETE, kb->get_domain());
        std::cout << cnf_grammar << std::endl;

        visitor.visit(cnf_grammar);

        const auto& concept_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::ConceptTag> {});

        EXPECT_EQ(concept_statistics.num_generated, 4322);
        EXPECT_EQ(concept_statistics.num_kept, 222);
        EXPECT_EQ(concept_statistics.num_pruned, 4100);

        const auto& role_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::RoleTag> {});

        EXPECT_EQ(role_statistics.num_generated, 3986);
        EXPECT_EQ(role_statistics.num_kept, 715);
        EXPECT_EQ(role_statistics.num_pruned, 3271);

        const auto& boolean_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::BooleanTag> {});

        EXPECT_EQ(boolean_statistics.num_generated, 308);
        EXPECT_EQ(boolean_statistics.num_kept, 13);
        EXPECT_EQ(boolean_statistics.num_pruned, 295);

        const auto& numerical_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::NumericalTag> {});

        EXPECT_EQ(numerical_statistics.num_generated, 2504);
        EXPECT_EQ(numerical_statistics.num_kept, 138);
        EXPECT_EQ(numerical_statistics.num_pruned, 2366);
    }

    {
        /**
         * FRANCES_ET_AL_AAAI2021
         */

        auto denotation_repositories = dl::DenotationRepositories();
        auto pruning_function = dl::StateListRefinementPruningFunction(kb->get_generalized_state_space().value(), denotation_repositories);
        auto sentences = dl::cnf_grammar::GeneratedSentencesContainer();
        auto repositories = dl::Repositories();
        size_t max_complexity = 5;
        auto visitor = dl::cnf_grammar::GeneratorVisitor(pruning_function, sentences, repositories, max_complexity);

        auto cnf_grammar = dl::cnf_grammar::Grammar::create(dl::cnf_grammar::GrammarSpecificationEnum::FRANCES_ET_AL_AAAI2021, kb->get_domain());
        std::cout << cnf_grammar << std::endl;

        visitor.visit(cnf_grammar);

        const auto& concept_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::ConceptTag> {});

        EXPECT_EQ(concept_statistics.num_generated, 788);
        EXPECT_EQ(concept_statistics.num_kept, 85);
        EXPECT_EQ(concept_statistics.num_pruned, 703);

        const auto& role_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::RoleTag> {});

        EXPECT_EQ(role_statistics.num_generated, 38);
        EXPECT_EQ(role_statistics.num_kept, 9);
        EXPECT_EQ(role_statistics.num_pruned, 29);

        const auto& boolean_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::BooleanTag> {});

        EXPECT_EQ(boolean_statistics.num_generated, 61);
        EXPECT_EQ(boolean_statistics.num_kept, 12);
        EXPECT_EQ(boolean_statistics.num_pruned, 49);

        const auto& numerical_statistics = boost::hana::at_key(visitor.get_statistics(), boost::hana::type<dl::NumericalTag> {});

        EXPECT_EQ(numerical_statistics.num_generated, 61);
        EXPECT_EQ(numerical_statistics.num_kept, 42);
        EXPECT_EQ(numerical_statistics.num_pruned, 19);
    }
}

}