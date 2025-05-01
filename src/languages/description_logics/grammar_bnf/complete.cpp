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

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
void add_start_symbol(std::stringstream& out)
{
    out << fmt::format("    <{}_start> = <{}>\n", D::name, D::name);
}

template<typename... Args>
static void add_rule(std::stringstream& out, const std::string& keyword, std::vector<std::string>& head_names, const Args&... args)  //
{
    out << fmt::format("    <{}> ::= @{} {}\n", keyword, keyword, fmt::join(std::vector<std::string> { args... }, " "));

    head_names.push_back(fmt::format("<{}>", keyword));
}

static std::string quoted(const std::string& s) { return fmt::format("\"{}\"", s); }

std::string create_complete_bnf(Domain domain)
{
    auto ss = std::stringstream {};
    ss << "[start_symbols]" << "\n";

    add_start_symbol<ConceptTag>(ss);
    add_start_symbol<RoleTag>(ss);
    add_start_symbol<BooleanTag>(ss);
    add_start_symbol<NumericalTag>(ss);

    ss << "[grammar_rules]" << "\n";

    auto concept_head_names = std::vector<std::string> {};

    add_rule(ss, dl::keywords::concept_bot, concept_head_names);
    add_rule(ss, dl::keywords::concept_top, concept_head_names);
    boost::hana::for_each(domain->get_hana_predicates(),
                          [&](auto&& pair)
                          {
                              const auto& second = boost::hana::second(pair);
                              for (const auto& predicate : second)
                              {
                                  if (predicate->get_arity() == 1)
                                  {
                                      add_rule(ss, dl::keywords::concept_atomic_state, concept_head_names, predicate->get_name());
                                      add_rule(ss, dl::keywords::concept_atomic_goal, concept_head_names, quoted(predicate->get_name()), "true");
                                      add_rule(ss, dl::keywords::concept_atomic_goal, concept_head_names, quoted(predicate->get_name()), "false");
                                  }
                              }
                          });
    add_rule(ss, dl::keywords::concept_intersection, concept_head_names, "<concept>", "<concept>");
    add_rule(ss, dl::keywords::concept_union, concept_head_names, "<concept>", "<concept>");
    add_rule(ss, dl::keywords::concept_negation, concept_head_names, "<concept>");
    add_rule(ss, dl::keywords::concept_value_restriction, concept_head_names, "<role>", "<concept>");
    add_rule(ss, dl::keywords::concept_existential_quantification, concept_head_names, "<role>", "<concept>");
    add_rule(ss, dl::keywords::concept_role_value_map_containment, concept_head_names, "<role>", "<role>");
    add_rule(ss, dl::keywords::concept_role_value_map_equality, concept_head_names, "<role>", "<role>");
    for (const auto& constant : domain->get_constants())
    {
        add_rule(ss, dl::keywords::concept_nominal, concept_head_names, quoted(constant->get_name()));
    }

    return ss.str();
}

}
