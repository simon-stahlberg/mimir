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

#include "parser.hpp"

#include <mimir/languages/description_logics/parser/ast.hpp>
#include <mimir/languages/description_logics/parser/parser.hpp>
#include <mimir/languages/description_logics/parser/parser_wrapper.hpp>

namespace mimir::dl::grammar
{

std::tuple<std::vector<const ConceptDerivationRule*>, std::vector<const RoleDerivationRule*>>
parse(const std::string& bnf_grammar_description, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    auto ast = dl::ast::Grammar();
    dl::parse_ast(bnf_grammar_description, dl::grammar_parser(), ast);

    auto concept_rules = std::vector<const ConceptDerivationRule*> {};
    auto role_rules = std::vector<const RoleDerivationRule*> {};

    return std::make_tuple(concept_rules, role_rules);
}

}