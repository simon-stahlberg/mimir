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
#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"
#include "mimir/languages/description_logics/parser/ast.hpp"
#include "mimir/languages/description_logics/parser/parser.hpp"
#include "mimir/languages/description_logics/parser/parser_wrapper.hpp"

#include <boost/hana.hpp>

namespace mimir::dl::grammar
{

static Constructor<Concept> parse(const dl::ast::Constructor<Concept>& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::apply_visitor(
        [&](const auto& arg) -> Constructor<Concept>
        {
            auto parsed_result = parse(arg, domain, ref_grammar_constructor_repos);

            // Check if the result is a variant
            if constexpr (std::is_same_v<decltype(parsed_result),
                                         std::variant<ConceptAtomicState<Static>, ConceptAtomicState<Fluent>, ConceptAtomicState<Derived>>>
                          || std::is_same_v<decltype(parsed_result),
                                            std::variant<ConceptAtomicGoal<Static>, ConceptAtomicGoal<Fluent>, ConceptAtomicGoal<Derived>>>)
            {
                // Visit the variant and process it
                return std::visit([&](const auto& resolved_arg) -> Constructor<Concept> { return resolved_arg; }, parsed_result);
            }
            else
            {
                // Directly process non-variant results
                return parsed_result;
            }
        },
        node);
}

static Constructor<Role> parse(const dl::ast::Constructor<Role>& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::apply_visitor(
        [&](const auto& arg) -> Constructor<Role>
        {
            auto parsed_result = parse(arg, domain, ref_grammar_constructor_repos);

            // Check if the result is a variant
            if constexpr (std::is_same_v<decltype(parsed_result), std::variant<RoleAtomicState<Static>, RoleAtomicState<Fluent>, RoleAtomicState<Derived>>>
                          || std::is_same_v<decltype(parsed_result), std::variant<RoleAtomicGoal<Static>, RoleAtomicGoal<Fluent>, RoleAtomicGoal<Derived>>>)
            {
                // Visit the variant and process it
                return std::visit([&](const auto& resolved_arg) -> Constructor<Role> { return resolved_arg; }, parsed_result);
            }
            else
            {
                // Directly process non-variant results
                return parsed_result;
            }
        },
        node);
}

template<ConceptOrRole D>
static NonTerminal<D> parse(const dl::ast::NonTerminal<D>& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<NonTerminalImpl<D>> {}).get_or_create(node.name);
}

template<ConceptOrRole D>
static ConstructorOrNonTerminal<D>
parse(const dl::ast::ConstructorOrNonTerminal<D>& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::apply_visitor(
        [&](const auto& arg) -> ConstructorOrNonTerminal<D>
        {
            return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConstructorOrNonTerminalImpl<D>> {})
                .get_or_create(parse(arg, domain, ref_grammar_constructor_repos));
        },
        node);
}

static ConceptBot parse(const dl::ast::ConceptBot& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptBotImpl> {}).get_or_create();
}

static ConceptTop parse(const dl::ast::ConceptTop& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptTopImpl> {}).get_or_create();
}

static std::variant<ConceptAtomicState<Static>, ConceptAtomicState<Fluent>, ConceptAtomicState<Derived>>
parse(const dl::ast::ConceptAtomicState& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    if (domain->get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicState from predicates with arity != 1.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptAtomicStateImpl<Static>> {}).get_or_create(predicate);
    }
    else if (domain->get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicState from predicates with arity != 1.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptAtomicStateImpl<Fluent>> {}).get_or_create(predicate);
    }
    else if (domain->get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicState from predicates with arity != 1.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptAtomicStateImpl<Derived>> {}).get_or_create(predicate);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static std::variant<ConceptAtomicGoal<Static>, ConceptAtomicGoal<Fluent>, ConceptAtomicGoal<Derived>>
parse(const dl::ast::ConceptAtomicGoal& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    if (domain->get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicGoal from predicates with arity != 1.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptAtomicGoalImpl<Static>> {})
            .get_or_create(predicate, node.is_negated);
    }
    else if (domain->get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicGoal from predicates with arity != 1.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptAtomicGoalImpl<Fluent>> {})
            .get_or_create(predicate, node.is_negated);
    }
    else if (domain->get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicGoal from predicates with arity != 1.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptAtomicGoalImpl<Derived>> {})
            .get_or_create(predicate, node.is_negated);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static ConceptIntersection parse(const dl::ast::ConceptIntersection& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptIntersectionImpl> {})
        .get_or_create(parse(node.concept_or_non_terminal_left, domain, ref_grammar_constructor_repos),
                       parse(node.concept_or_non_terminal_right, domain, ref_grammar_constructor_repos));
}

static ConceptUnion parse(const dl::ast::ConceptUnion& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptUnionImpl> {})
        .get_or_create(parse(node.concept_or_non_terminal_left, domain, ref_grammar_constructor_repos),
                       parse(node.concept_or_non_terminal_right, domain, ref_grammar_constructor_repos));
}

static ConceptNegation parse(const dl::ast::ConceptNegation& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptNegationImpl> {})
        .get_or_create(parse(node.concept_or_non_terminal, domain, ref_grammar_constructor_repos));
}

static ConceptValueRestriction parse(const dl::ast::ConceptValueRestriction& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptValueRestrictionImpl> {})
        .get_or_create(parse(node.role_or_non_terminal, domain, ref_grammar_constructor_repos),
                       parse(node.concept_or_non_terminal, domain, ref_grammar_constructor_repos));
}

static ConceptExistentialQuantification
parse(const dl::ast::ConceptExistentialQuantification& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptExistentialQuantificationImpl> {})
        .get_or_create(parse(node.role_or_non_terminal, domain, ref_grammar_constructor_repos),
                       parse(node.concept_or_non_terminal, domain, ref_grammar_constructor_repos));
}

static ConceptRoleValueMapContainment
parse(const dl::ast::ConceptRoleValueMapContainment& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptRoleValueMapContainmentImpl> {})
        .get_or_create(parse(node.role_or_non_terminal_left, domain, ref_grammar_constructor_repos),
                       parse(node.role_or_non_terminal_right, domain, ref_grammar_constructor_repos));
}

static ConceptRoleValueMapEquality
parse(const dl::ast::ConceptRoleValueMapEquality& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptRoleValueMapEqualityImpl> {})
        .get_or_create(parse(node.role_or_non_terminal_left, domain, ref_grammar_constructor_repos),
                       parse(node.role_or_non_terminal_right, domain, ref_grammar_constructor_repos));
}

static ConceptNominal parse(const dl::ast::ConceptNominal& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    if (!domain->get_name_to_constants().contains(node.object_name))
    {
        throw std::runtime_error("Domain has no constant with name \"" + node.object_name + "\".");
    }
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptNominalImpl> {})
        .get_or_create(domain->get_name_to_constants().at(node.object_name));
}

static RoleUniversal parse(const dl::ast::RoleUniversal& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleUniversalImpl> {}).get_or_create();
}

static std::variant<RoleAtomicState<Static>, RoleAtomicState<Fluent>, RoleAtomicState<Derived>>
parse(const dl::ast::RoleAtomicState& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    if (domain->get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicState from predicates with arity != 2.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleAtomicStateImpl<Static>> {}).get_or_create(predicate);
    }
    else if (domain->get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicState from predicates with arity != 2.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleAtomicStateImpl<Fluent>> {}).get_or_create(predicate);
    }
    else if (domain->get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicState from predicates with arity != 2.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleAtomicStateImpl<Derived>> {}).get_or_create(predicate);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static std::variant<RoleAtomicGoal<Static>, RoleAtomicGoal<Fluent>, RoleAtomicGoal<Derived>>
parse(const dl::ast::RoleAtomicGoal& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    if (domain->get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicGoal from predicates with arity != 2.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleAtomicGoalImpl<Static>> {}).get_or_create(predicate, node.is_negated);
    }
    else if (domain->get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicGoal from predicates with arity != 2.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleAtomicGoalImpl<Fluent>> {}).get_or_create(predicate, node.is_negated);
    }
    else if (domain->get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicGoal from predicates with arity != 2.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleAtomicGoalImpl<Derived>> {}).get_or_create(predicate, node.is_negated);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static RoleIntersection parse(const dl::ast::RoleIntersection& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleIntersectionImpl> {})
        .get_or_create(parse(node.role_or_non_terminal_left, domain, ref_grammar_constructor_repos),
                       parse(node.role_or_non_terminal_right, domain, ref_grammar_constructor_repos));
}

static RoleUnion parse(const dl::ast::RoleUnion& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleUnionImpl> {})
        .get_or_create(parse(node.role_or_non_terminal_left, domain, ref_grammar_constructor_repos),
                       parse(node.role_or_non_terminal_right, domain, ref_grammar_constructor_repos));
}

static RoleComplement parse(const dl::ast::RoleComplement& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleComplementImpl> {})
        .get_or_create(parse(node.role_or_non_terminal, domain, ref_grammar_constructor_repos));
}

static RoleInverse parse(const dl::ast::RoleInverse& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleInverseImpl> {})
        .get_or_create(parse(node.role_or_non_terminal, domain, ref_grammar_constructor_repos));
}

static RoleComposition parse(const dl::ast::RoleComposition& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleCompositionImpl> {})
        .get_or_create(parse(node.role_or_non_terminal_left, domain, ref_grammar_constructor_repos),
                       parse(node.role_or_non_terminal_right, domain, ref_grammar_constructor_repos));
}

static RoleTransitiveClosure parse(const dl::ast::RoleTransitiveClosure& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleTransitiveClosureImpl> {})
        .get_or_create(parse(node.role_or_non_terminal, domain, ref_grammar_constructor_repos));
}

static RoleReflexiveTransitiveClosure
parse(const dl::ast::RoleReflexiveTransitiveClosure& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleReflexiveTransitiveClosureImpl> {})
        .get_or_create(parse(node.role_or_non_terminal, domain, ref_grammar_constructor_repos));
}

static RoleRestriction parse(const dl::ast::RoleRestriction& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleRestrictionImpl> {})
        .get_or_create(parse(node.role_or_non_terminal, domain, ref_grammar_constructor_repos),
                       parse(node.concept_or_non_terminal, domain, ref_grammar_constructor_repos));
}

static RoleIdentity parse(const dl::ast::RoleIdentity& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleIdentityImpl> {})
        .get_or_create(parse(node.concept_or_non_terminal, domain, ref_grammar_constructor_repos));
}

/**
 * Grammar structure specific types.
 */

template<ConceptOrRole D>
static DerivationRule<D> parse(const dl::ast::DerivationRule<D>& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    auto non_terminal = parse(node.non_terminal, domain, ref_grammar_constructor_repos);
    auto constructor_or_non_terminals = ConstructorOrNonTerminalList<D> {};
    std::for_each(node.constructor_or_non_terminals.begin(),
                  node.constructor_or_non_terminals.end(),
                  [&](const auto& choice) { constructor_or_non_terminals.push_back(parse(choice, domain, ref_grammar_constructor_repos)); });
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<DerivationRuleImpl<D>> {})
        .get_or_create(non_terminal, constructor_or_non_terminals);
}

static StartSymbols parse(const dl::ast::GrammarHead& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    if (!node.concept_start.has_value() && !node.role_start.has_value())
    {
        throw std::runtime_error("parse(node, domain, ref_grammar_constructor_repos): No start symbols defined.");
    }

    auto start_symbols = StartSymbols();

    if (node.concept_start)
    {
        boost::hana::at_key(start_symbols, boost::hana::type<Concept> {}) = parse(node.concept_start.value(), domain, ref_grammar_constructor_repos);
    }

    if (node.role_start)
    {
        boost::hana::at_key(start_symbols, boost::hana::type<Role> {}) = parse(node.role_start.value(), domain, ref_grammar_constructor_repos);
    }

    return start_symbols;
}

static GrammarRules parse(const dl::ast::GrammarBody& node, Domain domain, GrammarConstructorRepositories& ref_grammar_constructor_repos)
{
    auto rules = GrammarRules {};

    for (const auto& part : node.rules)
    {
        boost::apply_visitor(
            [&](const auto& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, dl::ast::DerivationRule<Concept>>)
                {
                    const auto concept_derivation_rule = parse(arg, domain, ref_grammar_constructor_repos);
                    boost::hana::at_key(rules, boost::hana::type<Concept> {})[concept_derivation_rule->get_non_terminal()].insert(concept_derivation_rule);
                }
                else if constexpr (std::is_same_v<T, dl::ast::DerivationRule<Role>>)
                {
                    const auto role_derivation_rule = parse(arg, domain, ref_grammar_constructor_repos);
                    boost::hana::at_key(rules, boost::hana::type<Role> {})[role_derivation_rule->get_non_terminal()].insert(role_derivation_rule);
                }
                else
                {
                    throw std::runtime_error("Unknown rule type.");
                }
            },
            part);
    }

    return rules;
}

Grammar parse(const dl::ast::Grammar& node, Domain domain)
{
    auto repositories = GrammarConstructorRepositories();
    auto start_symbols = parse(node.head, domain, repositories);
    auto rules = parse(node.body, domain, repositories);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(rules));
}

Grammar parse(const std::string& bnf_description, Domain domain)
{
    auto ast = dl::ast::Grammar();
    dl::parse_ast(bnf_description, dl::grammar_parser(), ast);

    return parse(ast, domain);
}

}
