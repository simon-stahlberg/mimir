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
            <r_b> ::= @boolean_nonempty @concept_existential_quantification @role_atomic_goal "at" true @concept_atomic_state "at-robby"

        [numerical_features]
            <c> ::= @numerical_count @concept_existential_quantification @role_atomic_state "carry" @concept_top
            <numerical_b> ::=
                @numerical_count 
                    @concept_negation
                        @concept_role_value_map_equality
                            @role_atomic_state "at"
                            @role_atomic_goal "at" true

        [policy_rules]
            { @negative_boolean_condition <r_b>, @equal_numerical_condition <c>, @greater_numerical_condition <numerical_b> } 
            -> { @unchanged_boolean_effect <r_b>, @increase_numerical_effect <c>, @unchanged_numerical_effect <numerical_b> }
            { @positive_boolean_condition <r_b>, @equal_numerical_condition <c>, @greater_numerical_condition <numerical_b> } 
            -> { @negative_boolean_effect <r_b>, @unchanged_numerical_effect <c>, @unchanged_numerical_effect <numerical_b> }
            { @positive_boolean_condition <r_b>, @greater_numerical_condition <c>, @greater_numerical_condition <numerical_b> } 
            -> { @unchanged_boolean_effect <r_b>, @decrease_numerical_effect <c>, @decrease_numerical_effect <numerical_b> }
            { @negative_boolean_condition <r_b>, @greater_numerical_condition <c>, @greater_numerical_condition <numerical_b> } 
            -> { @positive_boolean_effect <r_b>, @unchanged_numerical_effect <c>, @unchanged_numerical_effect <numerical_b> }
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
            <c> ::= @numerical_count @concept_atomic_state "clear"
            <t> ::=
                @numerical_count 
                    @concept_negation
                        @concept_role_value_map_equality
                            @role_atomic_state "on"
                            @role_atomic_goal "on" true
            <bwp> ::= @numerical_count 
                @concept_value_restriction 
                    @role_transitive_closure @role_atomic_state "on"
                    @concept_role_value_map_equality
                        @role_atomic_state "on"
                        @role_atomic_goal "on" true

        [policy_rules]
            { @greater_numerical_condition <c>, @greater_numerical_condition <t> } 
            -> { @increase_numerical_effect <c>, @unchanged_numerical_effect <t>, @unchanged_numerical_effect <bwp> }
            { @greater_numerical_condition <c>, @greater_numerical_condition <t> } 
            -> { @increase_numerical_effect <c>, @increase_numerical_effect <bwp> }
            { @greater_numerical_condition <c>, @greater_numerical_condition <t> } 
            -> { @increase_numerical_effect <c>, @decrease_numerical_effect <t>, @unchanged_numerical_effect <bwp> }
            { @greater_numerical_condition <c>, @greater_numerical_condition <t> } 
            -> { @decrease_numerical_effect <c>, @decrease_numerical_effect <t>, @unchanged_numerical_effect <bwp> }
        )");

    return repositories.get_or_create_general_policy(description, domain, dl_repositories);
}

GeneralPolicy
GeneralPolicyFactory::get_or_create_general_policy_spanner(const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories)
{
    static auto description = std::string(R"(
        [boolean_features]
            <e> ::= 
                @boolean_nonempty 
                    @concept_existential_quantification
                        @role_atomic_state "at"
                        @concept_value_restriction
                            @role_inverse 
                                @role_atomic_state "at"
                            @concept_atomic_state "man"


        [numerical_features]
            <h> ::= 
                @numerical_count 
                    @concept_existential_quantification 
                        @role_atomic_state "at"
                        @concept_top

            <n> ::=
                @numerical_count 
                    @concept_intersection
                        @concept_negation
                            @concept_atomic_state "tightened"
                        @concept_atomic_goal "tightened" true

        [policy_rules]
            { @greater_numerical_condition <n>, @greater_numerical_condition <h>, @positive_boolean_condition <e> } 
            -> { @unchanged_numerical_effect <n>, @unchanged_numerical_effect <h> }
            { @greater_numerical_condition <n>, @greater_numerical_condition <h>, @negative_boolean_condition <e> } 
            -> { @unchanged_numerical_effect <n>, @decrease_numerical_effect <h> }
            { @greater_numerical_condition <n>, @greater_numerical_condition <h>, @negative_boolean_condition <e> } 
            -> { @decrease_numerical_effect <n>, @unchanged_numerical_effect <h>, @unchanged_boolean_effect <e> }
        )");

    return repositories.get_or_create_general_policy(description, domain, dl_repositories);
}

GeneralPolicy
GeneralPolicyFactory::get_or_create_general_policy_delivery(const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories)
{
    static auto description = std::string(R"(
        [boolean_features]
            <not_H> ::= 
                @boolean_nonempty @concept_atomic_state "empty"


        [numerical_features]
            <u> ::=
                @numerical_count 
                    @concept_negation
                        @concept_role_value_map_equality
                            @role_atomic_goal "at" true
                            @role_atomic_state "at"

            <t> ::= 
                @numerical_distance 
                    @concept_existential_quantification 
                        @role_inverse @role_atomic_state "at" 
                        @concept_atomic_state "truck" 
                    @role_atomic_state "adjacent" 
                    @concept_existential_quantification 
                        @role_inverse @role_atomic_goal "at" true 
                        @concept_top

            <p> ::=
                @numerical_distance 
                    @concept_existential_quantification 
                        @role_inverse @role_atomic_state "at" 
                        @concept_atomic_state "truck" 
                    @role_atomic_state "adjacent" 
                    @concept_intersection 
                        @concept_existential_quantification 
                            @role_inverse @role_atomic_state "at" 
                            @concept_atomic_state "package" 
                        @concept_value_restriction 
                            @role_inverse @role_atomic_goal "at" true 
                            @concept_bot

        [policy_rules]
            { @greater_numerical_condition <u>, @positive_boolean_condition <not_H>, @greater_numerical_condition <p> }
            -> { @unchanged_boolean_effect <not_H>, @decrease_numerical_effect <p> }
            { @greater_numerical_condition <u>, @positive_boolean_condition <not_H>, @equal_numerical_condition <p> }
            -> { @negative_boolean_effect <not_H> }
            { @greater_numerical_condition <u>, @negative_boolean_condition <not_H>, @greater_numerical_condition <t> }
            -> { @unchanged_numerical_effect <u>, @unchanged_boolean_effect <not_H>, @decrease_numerical_effect <t> }
            { @greater_numerical_condition <u>, @negative_boolean_condition <not_H>, @equal_numerical_condition <t> }
            -> { @decrease_numerical_effect <u>, @positive_boolean_effect <not_H>, @unchanged_numerical_effect <p> }
        )");

    return repositories.get_or_create_general_policy(description, domain, dl_repositories);
}

GeneralPolicy
GeneralPolicyFactory::get_or_create_general_policy_miconic(const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories)
{
    // TODO: finish this
    static auto description = std::string(R"(
        [boolean_features]

        [numerical_features]
            <b> ::=
                @numerical_count 
                    @concept_atomic_state "boarded"

            <w> ::=
                @numerical_count 
                    @concept_existential_quantification 
                        @role_atomic_state "origin"
                        @concept_top

            <rb> ::= 
                @numerical_count 
                    @concept_value_restriction 
                        @role_atomic_state "origin"
                        @concept_atomic_state "lift-at"

            <rl> ::=
                @numerical_count 
                    @concept_intersection 
                        @concept_existential_quantification
                            @role_atomic_state "destin"
                            @concept_atomic-state "lift-at"


        [policy_rules]
            { @greater_numerical_condition <u>, @positive_boolean_condition <not_H>, @greater_numerical_condition <p> }
            -> { @unchanged_boolean_effect <not_H>, @decrease_numerical_effect <p> }
            { @greater_numerical_condition <u>, @positive_boolean_condition <not_H>, @equal_numerical_condition <p> }
            -> { @negative_boolean_effect <not_H> }
            { @greater_numerical_condition <u>, @negative_boolean_condition <not_H>, @greater_numerical_condition <t> }
            -> { @unchanged_numerical_effect <u>, @unchanged_boolean_effect <not_H>, @decrease_numerical_effect <t> }
            { @greater_numerical_condition <u>, @negative_boolean_condition <not_H>, @equal_numerical_condition <t> }
            -> { @decrease_numerical_effect <u>, @positive_boolean_effect <not_H>, @unchanged_numerical_effect <p> }
        )");

    return repositories.get_or_create_general_policy(description, domain, dl_repositories);
}

}
