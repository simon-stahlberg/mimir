/*
 * Copyright (C) 2023 Dominik Drexler
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

#include "mimir/formalism/translator.hpp"

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/problem.hpp"

#include <gtest/gtest.h>

using namespace mimir;
using namespace mimir::formalism;

namespace mimir::tests
{

TEST(MimirTests, MimirFormalismTranslatorTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "gripper/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "gripper/test_problem.pddl");

    auto parser = Parser(domain_file);
    const auto domain = parser.get_domain();
    std::cout << *domain << std::endl << std::endl;
    const auto problem = parser.parse_problem(problem_file);
    std::cout << *problem << std::endl;
    auto translator = Translator(domain);
    const auto translated_domain = translator.get_translated_domain();
    std::cout << *translator.get_translated_domain() << std::endl;
    const auto translated_problem = translator.translate(problem);
    std::cout << *translated_problem << std::endl;

    {
        // Check that all domain constants are in the problem
        EXPECT_EQ(translated_domain->get_constants().size(), 2);
        auto problem_objects =
            ObjectSet(translated_problem->get_problem_and_domain_objects().begin(), translated_problem->get_problem_and_domain_objects().end());
        for (const auto& constant : translated_domain->get_constants())
        {
            EXPECT_TRUE(problem_objects.contains(constant));
        }
    }

    {
        // Check that problem does not introduce object with same name as constant in domain.
        auto domain_constants = std::unordered_map<std::string, Object> {};
        for (const auto& constant : translated_domain->get_constants())
        {
            domain_constants.emplace(constant->get_name(), constant);
        }

        for (const auto& object : translated_problem->get_objects())
        {
            if (domain_constants.contains(object->get_name()))
            {
                EXPECT_EQ(object, domain_constants.at(object->get_name()));
            }
        }
    }

    {
        // Check that problem does not introduce predicate with same name as predicate in domain.

        // TODO: the iterator is empty because all static predicates are in the domain's repository. Perhaps add iteration over parent repository as well.
        for (const auto& static_predicate :
             boost::hana::at_key(translated_problem->get_repositories().get_hana_repositories(), boost::hana::type<PredicateImpl<StaticTag>> {}))
        {
            EXPECT_EQ(static_predicate, translated_domain->get_name_to_predicate<StaticTag>().at(static_predicate->get_name()));
        }
    }
    /*
                {
                    // Check that problem does not introduce function skeleton with same name as function skeleton in domain.
                    auto domain_function_skeletons = std::unordered_map<std::string, FunctionSkeleton> {};
                    for (const auto& function_skeleton : translated_domain->get_function_skeletons())
                    {
                        domain_function_skeletons.emplace(function_skeleton->get_name(), function_skeleton);
                    }

                    const auto& problem_function_skeletons =
                        boost::hana::at_key(translated_domain->get_repositories().get_hana_repositories(), boost::hana::type<FunctionSkeletonImpl> {});
                    for (const auto& function_skeleton : problem_function_skeletons)
                    {
                        if (domain_function_skeletons.contains(function_skeleton->get_name()))
                        {
                            EXPECT_EQ(function_skeleton, domain_function_skeletons.at(function_skeleton->get_name()));
                        }
                    }
                }

                {
                    // Check that all constants and objects are untyped
                    for (const auto& constant : translated_domain->get_constants())
                    {
                        EXPECT_EQ(constant->get_bases().size(), 0);
                    }
                    for (const auto& object : translated_problem->get_objects())
                    {
                        EXPECT_EQ(object->get_bases().size(), 0);
                    }
                }
                */
}

}
