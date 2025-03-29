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
            <boolean_r_b> ::= @boolean_nonempty @concept_existential_quantification @role_atomic_goal "at" true @concept_atomic_state "at-robby"

        [numerical_features]
            <numerical_c> ::= @numerical_count @concept_existential_quantification @role_atomic_state "carry" @concept_top
            <numerical_b> ::=
                @numerical_count 
                    @concept_negation
                        @concept_role_value_map_equality
                            @role_atomic_state "at"
                            @role_atomic_goal "at" true

        [policy_rules]
            { @negative_boolean_condition <boolean_r_b>, @equal_numerical_condition <numerical_c>, @greater_numerical_condition <numerical_b> } 
            -> { @unchanged_boolean_effect <boolean_r_b>, @unchanged_numerical_effect <numerical_b>, @increase_numerical_effect <numerical_c> }
            { @positive_boolean_condition <boolean_r_b>, @equal_numerical_condition <numerical_c>, @greater_numerical_condition <numerical_b> } 
            -> { @negative_boolean_effect <boolean_r_b>, @unchanged_numerical_effect <numerical_b>, @unchanged_numerical_effect <numerical_c> }
            { @positive_boolean_condition <boolean_r_b>, @greater_numerical_condition <numerical_c>, @greater_numerical_condition <numerical_b> } 
            -> { @unchanged_boolean_effect <boolean_r_b>, @decrease_numerical_effect <numerical_c>, @decrease_numerical_effect <numerical_b> }
            { @negative_boolean_condition <boolean_r_b>, @greater_numerical_condition <numerical_c>, @greater_numerical_condition <numerical_b> } 
            -> { @positive_boolean_effect <boolean_r_b>, @unchanged_numerical_effect <numerical_b>, @unchanged_numerical_effect <numerical_c> }
        )");

    return repositories.get_or_create_general_policy(description, domain, dl_repositories);
}

GeneralPolicy GeneralPolicyFactory::get_or_create_general_policy_blocks3ops(const formalism::DomainImpl& domain,
                                                                            Repositories& repositories,
                                                                            dl::Repositories& dl_repositories)
{
    static auto description = std::string(R"(
        [boolean_features]

        [numerical_features]
            <numerical_c> ::= @numerical_count @concept_atomic_state "clear"
            <numerical_t> ::=
                @numerical_count 
                    @concept_negation
                        @concept_role_value_map_equality
                            @role_atomic_state "on"
                            @role_atomic_goal "on" true
            <numerical_bwp> ::= @numerical_count 
                @concept_value_restriction 
                    @role_transitive_closure @role_atomic_state "on"
                    @concept_role_value_map_equality
                        @role_atomic_state "on"
                        @role_atomic_goal "on" true

        [policy_rules]
            { @greater_numerical_condition <numerical_c>, @greater_numerical_condition <numerical_t> } 
            -> { @increase_numerical_effect <numerical_c>, @unchanged_numerical_effect <numerical_t>, @unchanged_numerical_effect <numerical_bwp> }
            { @greater_numerical_condition <numerical_c>, @greater_numerical_condition <numerical_t> } 
            -> { @increase_numerical_effect <numerical_c>, @increase_numerical_effect <numerical_bwp> }
            { @greater_numerical_condition <numerical_c>, @greater_numerical_condition <numerical_t> } 
            -> { @increase_numerical_effect <numerical_c>, @decrease_numerical_effect <numerical_t>, @unchanged_numerical_effect <numerical_bwp> }
            { @greater_numerical_condition <numerical_c>, @greater_numerical_condition <numerical_t> } 
            -> { @decrease_numerical_effect <numerical_c>, @decrease_numerical_effect <numerical_t>, @unchanged_numerical_effect <numerical_bwp> }
        )");

    return repositories.get_or_create_general_policy(description, domain, dl_repositories);
}

GeneralPolicy
GeneralPolicyFactory::get_or_create_general_policy_spanner(const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories)
{
    static auto description = std::string(R"(
        [boolean_features]
            <boolean_e> ::= 
                @boolean_nonempty 
                    @concept_existential_quantification
                        @role_atomic_state "at"
                        @concept_value_restriction
                            @role_inverse 
                                @role_atomic_state "at"
                            @concept_atomic_state "man"


        [numerical_features]
            <numerical_h> ::= 
                @numerical_count 
                    @concept_existential_quantification 
                        @role_atomic_state "at"
                        @concept_top

            <numerical_n> ::=
                @numerical_count 
                    @concept_intersection
                        @concept_negation
                            @concept_atomic_state "tightened"
                        @concept_atomic_goal "tightened" true

        [policy_rules]
            { @greater_numerical_condition <numerical_n>, @greater_numerical_condition <numerical_h>, @positive_boolean_condition <boolean_e> } 
            -> { @unchanged_numerical_effect <numerical_n>, @unchanged_numerical_effect <numerical_h> }
            { @greater_numerical_condition <numerical_n>, @greater_numerical_condition <numerical_h>, @negative_boolean_condition <boolean_e> } 
            -> { @unchanged_numerical_effect <numerical_n>, @decrease_numerical_effect <numerical_h> }
            { @greater_numerical_condition <numerical_n>, @greater_numerical_condition <numerical_h>, @negative_boolean_condition <boolean_e> } 
            -> { @decrease_numerical_effect <numerical_n>, @unchanged_numerical_effect <numerical_h>, @unchanged_boolean_effect <boolean_e> }
        )");

    return repositories.get_or_create_general_policy(description, domain, dl_repositories);
}

}
