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

#include "mimir/languages/general_policies/general_policy_factory.hpp"

#include "mimir/languages/general_policies/repositories.hpp"

namespace mimir::languages::general_policies
{
GeneralPolicy
GeneralPolicyFactory::get_or_create_general_policy_gripper(const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories)
{
    static auto description = std::string(R"(
        [boolean_features]
            <boolean_robby_at_b> ::= @boolean_nonempty @concept_existential_quantification @role_atomic_goal "at" false @concept_atomic_state "at-robby"

        [numerical_features]
            <numerical_carry> ::= @numerical_count @concept_existential_quantification @role_atomic_state "carry" @concept_top
            <numerical_undelivered> ::=
                @numerical_count 
                    @concept_negation
                        @concept_role_value_map_equality
                            @role_atomic_state "at"
                            @role_atomic_goal "at" false

        [policy_rules]
            { @negative_boolean_condition <boolean_robby_at_b>, @equal_numerical_condition <numerical_carry>, @greater_numerical_condition <numerical_undelivered> } 
            -> { @increase_numerical_effect <numerical_carry> }
            { @positive_boolean_condition <boolean_robby_at_b>, @equal_numerical_condition <numerical_carry>, @greater_numerical_condition <numerical_undelivered> } 
            -> { @negative_boolean_effect <boolean_robby_at_b> }
            { @positive_boolean_condition <boolean_robby_at_b>, @greater_numerical_condition <numerical_carry>, @greater_numerical_condition <numerical_undelivered> } 
            -> { @decrease_numerical_effect <numerical_carry>, @decrease_numerical_effect <numerical_undelivered> }
            { @negative_boolean_condition <boolean_robby_at_b>, @greater_numerical_condition <numerical_carry>, @greater_numerical_condition <numerical_undelivered> } 
            -> { @positive_boolean_effect <boolean_robby_at_b> }
        )");

    return repositories.get_or_create_general_policy(description, domain, dl_repositories);
}

}
