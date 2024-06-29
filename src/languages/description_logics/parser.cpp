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

static const Concept* parse(const dl::ast::Concept& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::apply_visitor([&](const auto& arg) -> const Concept* { return parse(arg, domain, ref_grammar_constructor_repos); }, node);
}

static const Concept* parse(const dl::ast::ConceptPredicateState& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    if (domain->get_name_to_static_predicate().count(node.predicate_name))
    {
        return &ref_grammar_constructor_repos.template create<ConceptPredicateState<Static>>(domain->get_name_to_static_predicate().at(node.predicate_name));
    }
    else if (domain->get_name_to_fluent_predicate().count(node.predicate_name))
    {
        return &ref_grammar_constructor_repos.create<ConceptPredicateState<Fluent>>(domain->get_name_to_fluent_predicate().at(node.predicate_name));
    }
    else if (domain->get_name_to_derived_predicate().count(node.predicate_name))
    {
        return &ref_grammar_constructor_repos.create<ConceptPredicateState<Derived>>(domain->get_name_to_derived_predicate().at(node.predicate_name));
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static const Concept* parse(const dl::ast::ConceptPredicateGoal& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    if (domain->get_name_to_static_predicate().count(node.predicate_name))
    {
        return &ref_grammar_constructor_repos.create<ConceptPredicateGoal<Static>>(domain->get_name_to_static_predicate().at(node.predicate_name));
    }
    else if (domain->get_name_to_fluent_predicate().count(node.predicate_name))
    {
        return &ref_grammar_constructor_repos.create<ConceptPredicateGoal<Fluent>>(domain->get_name_to_fluent_predicate().at(node.predicate_name));
    }
    else if (domain->get_name_to_derived_predicate().count(node.predicate_name))
    {
        return &ref_grammar_constructor_repos.create<ConceptPredicateGoal<Derived>>(domain->get_name_to_derived_predicate().at(node.predicate_name));
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static const Concept* parse(const dl::ast::ConceptAnd& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return &ref_grammar_constructor_repos.create<ConceptAnd>(*parse(node.concept_left, domain, ref_grammar_constructor_repos),
                                                             *parse(node.concept_right, domain, ref_grammar_constructor_repos));
}

static const Concept* parse(const dl::ast::ConceptNonTerminal& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos) {}

static ConceptChoice parse(const dl::ast::ConceptChoice& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos) {}

static ConceptDerivationRule* parse(const dl::ast::ConceptDerivationRule& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos) {}

static Role* parse(const dl::ast::Role& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos) {}

static Role* parse(const dl::ast::RolePredicateState& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos) {}

static Role* parse(const dl::ast::RolePredicateGoal& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos) {}

static Role* parse(const dl::ast::RoleAnd& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos) {}

static Role* parse(const dl::ast::RoleNonTerminal& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos) {}

static RoleChoice parse(const dl::ast::RoleChoice& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos) {}

static RoleDerivationRule* parse(const dl::ast::RoleDerivationRule& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos) {}

static std::tuple<std::vector<const ConceptDerivationRule*>, std::vector<const RoleDerivationRule*>>
parse(const dl::ast::Grammar& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
}

std::tuple<std::vector<const ConceptDerivationRule*>, std::vector<const RoleDerivationRule*>>
parse(const std::string& bnf_grammar_description, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    auto ast = dl::ast::Grammar();
    dl::parse_ast(bnf_grammar_description, dl::grammar_parser(), ast);

    auto concept_rules = std::vector<const ConceptDerivationRule*> {};
    auto role_rules = std::vector<const RoleDerivationRule*> {};

    return std::make_tuple(concept_rules, role_rules);
}

}