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

#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"
#include "mimir/languages/description_logics/parser/ast.hpp"
#include "mimir/languages/description_logics/parser/parser.hpp"
#include "mimir/languages/description_logics/parser/parser_wrapper.hpp"

namespace mimir::dl::grammar
{

struct Context
{
    std::unordered_map<std::string, DerivationRule<Concept>> m_concept_rules_by_name;
    std::unordered_map<std::string, DerivationRule<Role>> m_role_rules_by_name;

    std::unordered_map<std::string, NonTerminal<Concept>> m_concept_non_terminal_by_name;
    std::unordered_map<std::string, NonTerminal<Role>> m_role_non_terminal_by_name;
};

static Choice<Concept> parse(const dl::ast::Concept& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    return boost::apply_visitor([&](const auto& arg) -> Choice<Concept> { return parse(arg, domain, ref_grammar_constructor_repos, context); }, node);
}

const Choice<Concept>
parse(const dl::ast::ConceptPredicateState& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    if (domain->get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptPredicateState from predicates with arity != 1.");
        }
        return ref_grammar_constructor_repos.template get<ChoiceFactory<Concept>>().template get_or_create<ChoiceImpl<Concept>>(
            ref_grammar_constructor_repos.template get<ConceptPredicateStateFactory<Static>>().template get_or_create<ConceptPredicateStateImpl<Static>>(
                predicate));
    }
    else if (domain->get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptPredicateState from predicates with arity != 1.");
        }
        return ref_grammar_constructor_repos.template get<ChoiceFactory<Concept>>().template get_or_create<ChoiceImpl<Concept>>(
            ref_grammar_constructor_repos.template get<ConceptPredicateStateFactory<Fluent>>().template get_or_create<ConceptPredicateStateImpl<Fluent>>(
                predicate));
    }
    else if (domain->get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptPredicateState from predicates with arity != 1.");
        }
        return ref_grammar_constructor_repos.template get<ChoiceFactory<Concept>>().template get_or_create<ChoiceImpl<Concept>>(
            ref_grammar_constructor_repos.template get<ConceptPredicateStateFactory<Derived>>().template get_or_create<ConceptPredicateStateImpl<Derived>>(
                predicate));
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Choice<Concept>
parse(const dl::ast::ConceptPredicateGoal& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    if (domain->get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptPredicateGoal from predicates with arity != 1.");
        }
        return ref_grammar_constructor_repos.template get<ChoiceFactory<Concept>>().template get_or_create<ChoiceImpl<Concept>>(
            ref_grammar_constructor_repos.template get<ConceptPredicateGoalFactory<Static>>().template get_or_create<ConceptPredicateGoalImpl<Static>>(
                predicate));
    }
    else if (domain->get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptPredicateGoal from predicates with arity != 1.");
        }
        return ref_grammar_constructor_repos.template get<ChoiceFactory<Concept>>().template get_or_create<ChoiceImpl<Concept>>(
            ref_grammar_constructor_repos.template get<ConceptPredicateGoalFactory<Fluent>>().template get_or_create<ConceptPredicateGoalImpl<Fluent>>(
                predicate));
    }
    else if (domain->get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptPredicateGoal from predicates with arity != 1.");
        }
        return ref_grammar_constructor_repos.template get<ChoiceFactory<Concept>>().template get_or_create<ChoiceImpl<Concept>>(
            ref_grammar_constructor_repos.template get<ConceptPredicateGoalFactory<Derived>>().template get_or_create<ConceptPredicateGoalImpl<Derived>>(
                predicate));
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Choice<Concept> parse(const dl::ast::ConceptAnd& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    return ref_grammar_constructor_repos.template get<ChoiceFactory<Concept>>().template get_or_create<ChoiceImpl<Concept>>(
        ref_grammar_constructor_repos.template get<ConceptAndFactory>().template get_or_create<ConceptAndImpl>(
            parse(node.concept_left, domain, ref_grammar_constructor_repos, context),
            parse(node.concept_right, domain, ref_grammar_constructor_repos, context)));
}

static Choice<Concept>
parse(const dl::ast::ConceptNonTerminal& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    const auto& non_terminal =
        ref_grammar_constructor_repos.template get<NonTerminalFactory<Concept>>().template get_or_create<NonTerminalImpl<Concept>>(node.name);
    context.m_concept_non_terminal_by_name.emplace(node.name, non_terminal);
    return ref_grammar_constructor_repos.template get<ChoiceFactory<Concept>>().template get_or_create<ChoiceImpl<Concept>>(non_terminal);
}

static Choice<Concept>
parse(const dl::ast::ConceptChoice& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    return boost::apply_visitor([&](const auto& arg) -> Choice<Concept> { return parse(arg, domain, ref_grammar_constructor_repos, context); }, node);
}

static DerivationRule<Concept>
parse(const dl::ast::ConceptDerivationRule& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    auto choices = ChoiceList<Concept> {};
    std::for_each(node.choices.begin(),
                  node.choices.end(),
                  [&](const auto& choice) { choices.push_back(parse(choice, domain, ref_grammar_constructor_repos, context)); });
    const auto& rule =
        ref_grammar_constructor_repos.template get<DerivationRuleFactory<Concept>>().template get_or_create<DerivationRuleImpl<Concept>>(choices);
    if (context.m_concept_rules_by_name.count(node.non_terminal.name))
    {
        throw std::runtime_error("Got multiple concept rule definitions for non terminal \"" + node.non_terminal.name + "\". Use choice rules instead.");
    }
    context.m_concept_rules_by_name.emplace(node.non_terminal.name, rule);
    return rule;
}

static Choice<Role> parse(const dl::ast::Role& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    return boost::apply_visitor([&](const auto& arg) -> Choice<Role> { return parse(arg, domain, ref_grammar_constructor_repos, context); }, node);
}

static Choice<Role>
parse(const dl::ast::RolePredicateState& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    if (domain->get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RolePredicateState from predicates with arity != 2.");
        }
        return ref_grammar_constructor_repos.template get<ChoiceFactory<Role>>().template get_or_create<ChoiceImpl<Role>>(
            ref_grammar_constructor_repos.template get<RolePredicateStateFactory<Static>>().template get_or_create<RolePredicateStateImpl<Static>>(predicate));
    }
    else if (domain->get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RolePredicateState from predicates with arity != 2.");
        }
        return ref_grammar_constructor_repos.template get<ChoiceFactory<Role>>().template get_or_create<ChoiceImpl<Role>>(
            ref_grammar_constructor_repos.template get<RolePredicateStateFactory<Fluent>>().template get_or_create<RolePredicateStateImpl<Fluent>>(predicate));
    }
    else if (domain->get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RolePredicateState from predicates with arity != 2.");
        }
        return ref_grammar_constructor_repos.template get<ChoiceFactory<Role>>().template get_or_create<ChoiceImpl<Role>>(
            ref_grammar_constructor_repos.template get<RolePredicateStateFactory<Derived>>().template get_or_create<RolePredicateStateImpl<Derived>>(
                predicate));
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Choice<Role>
parse(const dl::ast::RolePredicateGoal& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    if (domain->get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RolePredicateGoal from predicates with arity != 2.");
        }
        return ref_grammar_constructor_repos.template get<ChoiceFactory<Role>>().template get_or_create<ChoiceImpl<Role>>(
            ref_grammar_constructor_repos.template get<RolePredicateGoalFactory<Static>>().template get_or_create<RolePredicateGoalImpl<Static>>(predicate));
    }
    else if (domain->get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RolePredicateGoal from predicates with arity != 2.");
        }
        return ref_grammar_constructor_repos.template get<ChoiceFactory<Role>>().template get_or_create<ChoiceImpl<Role>>(
            ref_grammar_constructor_repos.template get<RolePredicateGoalFactory<Fluent>>().template get_or_create<RolePredicateGoalImpl<Fluent>>(predicate));
    }
    else if (domain->get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RolePredicateGoal from predicates with arity != 2.");
        }
        return ref_grammar_constructor_repos.template get<ChoiceFactory<Role>>().template get_or_create<ChoiceImpl<Role>>(
            ref_grammar_constructor_repos.template get<RolePredicateGoalFactory<Derived>>().template get_or_create<RolePredicateGoalImpl<Derived>>(predicate));
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Choice<Role> parse(const dl::ast::RoleAnd& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    return ref_grammar_constructor_repos.template get<ChoiceFactory<Role>>().template get_or_create<ChoiceImpl<Role>>(
        ref_grammar_constructor_repos.template get<RoleAndFactory>().template get_or_create<RoleAndImpl>(
            parse(node.role_left, domain, ref_grammar_constructor_repos, context),
            parse(node.role_right, domain, ref_grammar_constructor_repos, context)));
}

static Choice<Role>
parse(const dl::ast::RoleNonTerminal& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    const auto& non_terminal = ref_grammar_constructor_repos.template get<NonTerminalFactory<Role>>().template get_or_create<NonTerminalImpl<Role>>(node.name);
    context.m_role_non_terminal_by_name.emplace(node.name, non_terminal);
    return ref_grammar_constructor_repos.template get<ChoiceFactory<Role>>().template get_or_create<ChoiceImpl<Role>>(non_terminal);
}

static Choice<Role> parse(const dl::ast::RoleChoice& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    return boost::apply_visitor([&](const auto& arg) -> Choice<Role> { return parse(arg, domain, ref_grammar_constructor_repos, context); }, node);
}

static DerivationRule<Role>
parse(const dl::ast::RoleDerivationRule& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    auto choices = ChoiceList<Role> {};
    std::for_each(node.choices.begin(),
                  node.choices.end(),
                  [&](const auto& choice) { choices.push_back(parse(choice, domain, ref_grammar_constructor_repos, context)); });
    const auto& rule = ref_grammar_constructor_repos.template get<DerivationRuleFactory<Role>>().template get_or_create<DerivationRuleImpl<Role>>(choices);
    if (context.m_role_rules_by_name.count(node.non_terminal.name))
    {
        throw std::runtime_error("Got multiple role rule definitions for non terminal \"" + node.non_terminal.name + "\". Use choice rules instead.");
    }
    context.m_role_rules_by_name.emplace(node.non_terminal.name, rule);
    return rule;
}

static std::tuple<DerivationRuleList<Concept>, DerivationRuleList<Role>>
parse(const dl::ast::Grammar& node, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos, Context& context)
{
    auto concept_rules = DerivationRuleList<Concept> {};
    auto role_rules = DerivationRuleList<Role> {};

    std::for_each(node.rules.begin(),
                  node.rules.end(),
                  [&](const dl::ast::DerivationRule& rule_node)
                  {
                      boost::apply_visitor(
                          [&](const auto& arg)
                          {
                              using T = std::decay_t<decltype(arg)>;
                              if constexpr (std::is_same_v<T, dl::ast::ConceptDerivationRule>)
                              {
                                  concept_rules.push_back(parse(arg, domain, ref_grammar_constructor_repos, context));
                              }
                              else if constexpr (std::is_same_v<T, dl::ast::RoleDerivationRule>)
                              {
                                  role_rules.push_back(parse(arg, domain, ref_grammar_constructor_repos, context));
                              }
                              else
                              {
                                  throw std::runtime_error("Unknown rule type.");
                              }
                          },
                          rule_node);
                  });

    /* Deferred initialization step */
    for (auto& [non_terminal_name, non_terminal] : context.m_concept_non_terminal_by_name)
    {
        if (!context.m_concept_rules_by_name.count(non_terminal_name))
        {
            throw std::runtime_error("Missing definition for non terminal concept \"" + non_terminal_name + "\".");
        }
        const auto rule = context.m_concept_rules_by_name.at(non_terminal_name);
        non_terminal->set_rule(rule);
    }
    for (auto& [non_terminal_name, non_terminal] : context.m_role_non_terminal_by_name)
    {
        if (!context.m_role_rules_by_name.count(non_terminal_name))
        {
            throw std::runtime_error("Missing definition for non terminal role \"" + non_terminal_name + "\".");
        }
        const auto rule = context.m_role_rules_by_name.at(non_terminal_name);
        non_terminal->set_rule(rule);
    }

    return std::make_tuple(concept_rules, role_rules);
}

std::tuple<DerivationRuleList<Concept>, DerivationRuleList<Role>>
parse(const std::string& bnf_grammar_description, Domain domain, VariadicGrammarConstructorFactory& ref_grammar_constructor_repos)
{
    auto ast = dl::ast::Grammar();
    dl::parse_ast(bnf_grammar_description, dl::grammar_parser(), ast);
    auto context = Context();
    const auto [concept_rules, role_rules] = parse(ast, domain, ref_grammar_constructor_repos, context);
    return std::make_tuple(std::move(concept_rules), std::move(role_rules));
}
}
