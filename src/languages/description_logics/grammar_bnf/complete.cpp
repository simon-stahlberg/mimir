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

#include "complete.hpp"

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructor_keywords.hpp"
#include "mimir/languages/description_logics/tags.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>

using namespace mimir::formalism;

namespace mimir::languages::dl::cnf_grammar
{

static std::string quoted(const std::string& s) { return fmt::format("\"{}\"", s); }

static std::string start(const std::string& s) { return fmt::format("{}_start", s); }

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
void add_start_symbol(std::stringstream& out)
{
    out << fmt::format("    {} ::= <{}>\n", D::name, start(D::name));
}

static void add_choice_rule(std::stringstream& out, const std::string& keyword, const std::vector<std::string>& choices)  //
{
    out << fmt::format("    <{}> ::= {}\n", keyword, fmt::join(choices, " | "));
}

static void add_derivation_rule(std::stringstream& out, const std::string& keyword, const std::vector<std::string>& args)  //
{
    out << fmt::format("    <{}> ::= @{} {}\n", keyword, keyword, fmt::join(args, " "));
}

template<typename... Args>
static void add_derivation_rule(std::stringstream& out, const std::string& keyword, const Args&... args)  //
{
    add_derivation_rule(out, keyword, std::vector<std::string> { args... });
}

template<typename... Args>
static void add_derivation_rule_and_head(std::stringstream& out, const std::string& keyword, std::vector<std::string>& head_names, const Args&... args)  //
{
    add_derivation_rule(out, keyword, args...);

    head_names.push_back(fmt::format("<{}>", keyword));
}

std::string create_complete_bnf(Domain domain)
{
    auto ss = std::stringstream {};
    ss << "[start_symbols]" << "\n";

    add_start_symbol<ConceptTag>(ss);
    add_start_symbol<RoleTag>(ss);
    add_start_symbol<BooleanTag>(ss);
    add_start_symbol<NumericalTag>(ss);

    ss << "[grammar_rules]" << "\n";

    add_choice_rule(ss, start(ConceptTag::name), std::vector<std::string> { "<concept>" });
    add_choice_rule(ss, start(RoleTag::name), std::vector<std::string> { "<role>" });
    add_choice_rule(ss, start(BooleanTag::name), std::vector<std::string> { "<boolean>" });
    add_choice_rule(ss, start(NumericalTag::name), std::vector<std::string> { "<numerical>" });

    auto concept_head_names = std::vector<std::string> {};
    auto role_head_names = std::vector<std::string> {};
    auto boolean_head_names = std::vector<std::string> {};
    auto numerical_head_names = std::vector<std::string> {};

    boost::hana::for_each(
        domain->get_hana_predicates(),
        [&](auto&& pair)
        {
            const auto& second = boost::hana::second(pair);
            for (const auto& predicate : second)
            {
                if (predicate->get_arity() == 0)
                {
                    add_derivation_rule_and_head(ss, dl::keywords::boolean_atomic_state, boolean_head_names, quoted(predicate->get_name()));
                }
                else if (predicate->get_arity() == 1)
                {
                    add_derivation_rule_and_head(ss, dl::keywords::concept_atomic_state, concept_head_names, quoted(predicate->get_name()));
                    add_derivation_rule_and_head(ss, dl::keywords::concept_atomic_goal, concept_head_names, quoted(predicate->get_name()), "true");
                    add_derivation_rule_and_head(ss, dl::keywords::concept_atomic_goal, concept_head_names, quoted(predicate->get_name()), "false");
                }
                else if (predicate->get_arity() == 2)
                {
                    add_derivation_rule_and_head(ss, dl::keywords::role_atomic_state, role_head_names, quoted(predicate->get_name()));
                    add_derivation_rule_and_head(ss, dl::keywords::role_atomic_goal, role_head_names, quoted(predicate->get_name()), "true");
                    add_derivation_rule_and_head(ss, dl::keywords::role_atomic_goal, role_head_names, quoted(predicate->get_name()), "false");
                }
            }
        });

    add_derivation_rule_and_head(ss, dl::keywords::concept_bot, concept_head_names);
    add_derivation_rule_and_head(ss, dl::keywords::concept_top, concept_head_names);
    add_derivation_rule_and_head(ss, dl::keywords::concept_intersection, concept_head_names, "<concept>", "<concept>");
    add_derivation_rule_and_head(ss, dl::keywords::concept_union, concept_head_names, "<concept>", "<concept>");
    add_derivation_rule_and_head(ss, dl::keywords::concept_negation, concept_head_names, "<concept>");
    add_derivation_rule_and_head(ss, dl::keywords::concept_value_restriction, concept_head_names, "<role>", "<concept>");
    add_derivation_rule_and_head(ss, dl::keywords::concept_existential_quantification, concept_head_names, "<role>", "<concept>");
    add_derivation_rule_and_head(ss, dl::keywords::concept_role_value_map_containment, concept_head_names, "<role>", "<role>");
    add_derivation_rule_and_head(ss, dl::keywords::concept_role_value_map_equality, concept_head_names, "<role>", "<role>");
    for (const auto& constant : domain->get_constants())
    {
        add_derivation_rule_and_head(ss, dl::keywords::concept_nominal, concept_head_names, quoted(constant->get_name()));
    }

    add_derivation_rule_and_head(ss, dl::keywords::role_complement, role_head_names, "<role>");
    add_derivation_rule_and_head(ss, dl::keywords::role_composition, role_head_names, "<role>", "<role>");
    add_derivation_rule_and_head(ss, dl::keywords::role_identity, role_head_names, "<concept>");
    add_derivation_rule_and_head(ss, dl::keywords::role_intersection, role_head_names, "<role>", "<role>");
    add_derivation_rule_and_head(ss, dl::keywords::role_inverse, role_head_names, "<role>");
    add_derivation_rule_and_head(ss, dl::keywords::role_reflexive_transitive_closure, role_head_names, "<role>");
    add_derivation_rule_and_head(ss, dl::keywords::role_restriction, role_head_names, "<role>", "<concept>");
    add_derivation_rule_and_head(ss, dl::keywords::role_transitive_closure, role_head_names, "<role>");
    add_derivation_rule_and_head(ss, dl::keywords::role_union, role_head_names, "<role>", "<role>");
    add_derivation_rule_and_head(ss, dl::keywords::role_universal, role_head_names);

    add_derivation_rule_and_head(ss, dl::keywords::boolean_nonempty, boolean_head_names, "<concept>");
    add_derivation_rule_and_head(ss, dl::keywords::boolean_nonempty, boolean_head_names, "<role>");

    add_derivation_rule_and_head(ss, dl::keywords::numerical_count, numerical_head_names, "<concept>");
    add_derivation_rule_and_head(ss, dl::keywords::numerical_count, numerical_head_names, "<role>");
    add_derivation_rule_and_head(ss, dl::keywords::numerical_distance, numerical_head_names, "<concept>", "<role>", "<concept>");

    add_choice_rule(ss, "concept", concept_head_names);
    add_choice_rule(ss, "role", role_head_names);
    add_choice_rule(ss, "boolean", boolean_head_names);
    add_choice_rule(ss, "numerical", numerical_head_names);

    return ss.str();
}

}
