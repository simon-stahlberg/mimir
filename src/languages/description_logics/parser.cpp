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

struct Context
{
    std::unordered_map<std::string, const ConceptDerivationRule*> m_concept_rules_by_name;
    std::unordered_map<std::string, const RoleDerivationRule*> m_role_rules_by_name;

    // Double pointer for deferred initialization
    std::unordered_map<const ConceptNonTerminal*, const ConceptDerivationRule**> m_uninitialized_concept_rules_by_non_terminal;
    std::unordered_map<const ConceptNonTerminal*, const RoleDerivationRule**> m_uninitialized_role_rule_by_non_terminal;
};

static ConceptChoice parse(const dl::ast::Concept& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context)
{
    return boost::apply_visitor([&](const auto& arg) -> ConceptChoice { return parse(arg, domain, ref_grammar_constructor_repos, context); }, node);
}

static ConceptChoice
parse(const dl::ast::ConceptPredicateState& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context)
{
    if (domain->get_name_to_static_predicate().count(node.predicate_name))
    {
        return ref_grammar_constructor_repos.template create<ConceptChoice>(
            ref_grammar_constructor_repos.template create<ConceptPredicateState<Static>>(domain->get_name_to_static_predicate().at(node.predicate_name)));
    }
    else if (domain->get_name_to_fluent_predicate().count(node.predicate_name))
    {
        return ref_grammar_constructor_repos.template create<ConceptChoice>(
            ref_grammar_constructor_repos.create<ConceptPredicateState<Fluent>>(domain->get_name_to_fluent_predicate().at(node.predicate_name)));
    }
    else if (domain->get_name_to_derived_predicate().count(node.predicate_name))
    {
        return ref_grammar_constructor_repos.template create<ConceptChoice>(
            ref_grammar_constructor_repos.create<ConceptPredicateState<Derived>>(domain->get_name_to_derived_predicate().at(node.predicate_name)));
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static ConceptChoice
parse(const dl::ast::ConceptPredicateGoal& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context)
{
    if (domain->get_name_to_static_predicate().count(node.predicate_name))
    {
        return ref_grammar_constructor_repos.template create<ConceptChoice>(
            ref_grammar_constructor_repos.create<ConceptPredicateGoal<Static>>(domain->get_name_to_static_predicate().at(node.predicate_name)));
    }
    else if (domain->get_name_to_fluent_predicate().count(node.predicate_name))
    {
        return ref_grammar_constructor_repos.template create<ConceptChoice>(
            ref_grammar_constructor_repos.create<ConceptPredicateGoal<Fluent>>(domain->get_name_to_fluent_predicate().at(node.predicate_name)));
    }
    else if (domain->get_name_to_derived_predicate().count(node.predicate_name))
    {
        return ref_grammar_constructor_repos.template create<ConceptChoice>(
            ref_grammar_constructor_repos.create<ConceptPredicateGoal<Derived>>(domain->get_name_to_derived_predicate().at(node.predicate_name)));
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static ConceptChoice parse(const dl::ast::ConceptAnd& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context)
{
    return ref_grammar_constructor_repos.template create<ConceptChoice>(
        ref_grammar_constructor_repos.create<ConceptAnd>(parse(node.concept_left, domain, ref_grammar_constructor_repos, context),
                                                         parse(node.concept_right, domain, ref_grammar_constructor_repos, context)));
}

static ConceptChoice
parse(const dl::ast::ConceptNonTerminal& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context)
{
    auto concept_rule = std::make_unique<const ConceptDerivationRule*>(nullptr);
    // Get and store uninitialized memory for deferred initialization.
    auto uninitialized_concept_rule = concept_rule.get();
    const auto& non_terminal = ref_grammar_constructor_repos.create<ConceptNonTerminal>(node.name, std::move(concept_rule));
    context.m_uninitialized_concept_rules_by_non_terminal.emplace(&non_terminal, uninitialized_concept_rule);
    return ref_grammar_constructor_repos.template create<ConceptChoice>(non_terminal);
}

static ConceptChoice parse(const dl::ast::ConceptChoice& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context)
{
}

static ConceptDerivationRule*
parse(const dl::ast::ConceptDerivationRule& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context)
{
}

static RoleChoice parse(const dl::ast::Role& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context) {}

static RoleChoice parse(const dl::ast::RolePredicateState& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context)
{
}

static RoleChoice parse(const dl::ast::RolePredicateGoal& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context)
{
}

static RoleChoice parse(const dl::ast::RoleAnd& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context) {}

static RoleChoice parse(const dl::ast::RoleNonTerminal& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context) {}

static RoleChoice parse(const dl::ast::RoleChoice& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context) {}

static RoleDerivationRule*
parse(const dl::ast::RoleDerivationRule& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context)
{
}

static std::tuple<std::vector<const ConceptDerivationRule*>, std::vector<const RoleDerivationRule*>>
parse(const dl::ast::Grammar& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos, Context& context)
{
}

std::tuple<std::vector<const ConceptDerivationRule*>, std::vector<const RoleDerivationRule*>>
parse(const std::string& bnf_grammar_description, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    auto ast = dl::ast::Grammar();
    dl::parse_ast(bnf_grammar_description, dl::grammar_parser(), ast);

    auto concept_rules = std::vector<const ConceptDerivationRule*> {};
    auto role_rules = std::vector<const RoleDerivationRule*> {};

    auto context = Context();

    return std::make_tuple(concept_rules, role_rules);
}

}