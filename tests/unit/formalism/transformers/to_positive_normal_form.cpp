/*
 * Copyright (C) 2024 Dominik Drexler and Till Hofmann
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

#include "mimir/formalism/transformers/encode_parameter_index_in_variables.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <mimir/formalism/action.hpp>
#include <mimir/formalism/literal.hpp>
#include <mimir/formalism/parser.hpp>
#include <mimir/formalism/predicate.hpp>
#include <mimir/formalism/transformers/to_positive_normal_form.hpp>

namespace mimir
{

MATCHER_P(NameMatches, name, "foobar") { return arg->get_name() == name; }

TEST(MimirTests, FormalismTransformersToPositiveNormalForm)
{
    using testing::AllOf;
    using testing::Contains;
    using testing::Property;

    const auto domain_file = fs::path(std::string(DATA_DIR) + "ferry/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "ferry/test_problem.pddl");
    const auto parser = PDDLParser(domain_file, problem_file);

    const auto problem = parser.get_problem();

    auto pnf_factories = std::make_shared<PDDLRepositories>();
    auto pnf_problem = ToPositiveNormalFormTransformer(*pnf_factories).run(*problem);
    pnf_problem = EncodeParameterIndexInVariables(*pnf_factories).run(*pnf_problem);

    const auto& orig_domain = *problem->get_domain();
    const auto cond = orig_domain.get_actions()[0]->get_conditions<Fluent>();
    // TODO also match parameter names
    EXPECT_THAT(orig_domain.get_actions(),
                Contains(AllOf(
                    Property(&ActionImpl::get_name, "sail"),
                    Property(&ActionImpl::get_conditions<Fluent>,
                             Contains(AllOf(Property(&LiteralImpl<Fluent>::is_negated, true),
                                            Property(&LiteralImpl<Fluent>::get_atom,
                                                     Property(&AtomImpl<Fluent>::get_predicate, Property(&PredicateImpl<Fluent>::get_name, "at-ferry")))))))));
    const auto& pnf_domain = *pnf_problem->get_domain();
    EXPECT_THAT(pnf_domain.get_actions(),
                Contains(Not(AllOf(
                    Property(&ActionImpl::get_name, "sail"),
                    Property(&ActionImpl::get_conditions<Fluent>,
                             Contains(AllOf(Property(&LiteralImpl<Fluent>::is_negated, true),
                                            Property(&LiteralImpl<Fluent>::get_atom,
                                                     Property(&AtomImpl<Fluent>::get_predicate, Property(&PredicateImpl<Fluent>::get_name, "at-ferry"))))))))));
    EXPECT_THAT(pnf_domain.get_actions(),
                Not(Contains(Property(&ActionImpl::get_conditions<Fluent>, Contains(Property(&LiteralImpl<Fluent>::is_negated, true))))));
    EXPECT_THAT(pnf_domain.get_actions(),
                Not(Contains(Property(&ActionImpl::get_conditions<Static>, Contains(Property(&LiteralImpl<Static>::is_negated, true))))));
    EXPECT_THAT(pnf_domain.get_actions(),
                Not(Contains(Property(&ActionImpl::get_conditions<Derived>, Contains(Property(&LiteralImpl<Derived>::is_negated, true))))));
    EXPECT_THAT(
        pnf_domain.get_actions(),
        Contains(AllOf(
            Property(&ActionImpl::get_name, "sail"),
            Property(&ActionImpl::get_conditions<Derived>,
                     Contains(AllOf(Property(&LiteralImpl<Derived>::is_negated, false),
                                    Property(&LiteralImpl<Derived>::get_atom,
                                             Property(&AtomImpl<Derived>::get_predicate, Property(&PredicateImpl<Derived>::get_name, "not at-ferry")))))))));
    // Also just the corresponding string representations.
    // EXPECT_THAT(orig_domain.str(), testing::HasSubstr("(not (at-ferry ?to_1))"));
    // EXPECT_THAT(pnf_domain.str(), testing::HasSubstr("(not at-ferry ?l)"));
    // EXPECT_THAT(pnf_domain.str(), testing::HasSubstr("(not at-ferry ?to_1_1)"));
    // EXPECT_THAT(pnf_domain.str(), testing::Not(testing::HasSubstr("(not (at-ferry ?to_1))")));
}
}
