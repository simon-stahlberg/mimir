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

#include <boost/hana.hpp>

namespace mimir::dl::grammar
{

struct Context
{
    using ConstructorTagToDerivationRuleByName =
        boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_map<std::string, DerivationRule<Concept>>>,
                         boost::hana::pair<boost::hana::type<Role>, std::unordered_map<std::string, DerivationRule<Role>>>>;

    ConstructorTagToDerivationRuleByName m_derivation_rules_by_name;

    using ConstructorTagToNonTerminalByName =
        boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_map<std::string, NonTerminal<Concept>>>,
                         boost::hana::pair<boost::hana::type<Role>, std::unordered_map<std::string, NonTerminal<Role>>>>;

    ConstructorTagToNonTerminalByName m_non_terminals_by_name;
};

static Constructor<Concept>
parse(const dl::ast::Constructor<Concept>& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::apply_visitor(
        [&](const auto& arg) -> Constructor<Concept>
        {
            auto parsed_result = parse(arg, domain, ref_grammar_constructor_repos, context);

            // Check if the result is a variant
            if constexpr (
                std::is_same_v<
                    decltype(parsed_result),
                    std::variant<
                        ConceptAtomicState<Static>,
                        ConceptAtomicState<Fluent>,
                        ConceptAtomicState<
                            Derived>>> || std::is_same_v<decltype(parsed_result), std::variant<ConceptAtomicGoal<Static>, ConceptAtomicGoal<Fluent>, ConceptAtomicGoal<Derived>>>)
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

static Constructor<Role>
parse(const dl::ast::Constructor<Role>& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::apply_visitor(
        [&](const auto& arg) -> Constructor<Role>
        {
            auto parsed_result = parse(arg, domain, ref_grammar_constructor_repos, context);

            // Check if the result is a variant
            if constexpr (
                std::is_same_v<
                    decltype(parsed_result),
                    std::variant<
                        RoleAtomicState<Static>,
                        RoleAtomicState<Fluent>,
                        RoleAtomicState<
                            Derived>>> || std::is_same_v<decltype(parsed_result), std::variant<RoleAtomicGoal<Static>, RoleAtomicGoal<Fluent>, RoleAtomicGoal<Derived>>>)
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

template<ConstructorTag D>
static NonTerminal<D> parse(const dl::ast::NonTerminal<D>& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    const auto& non_terminal =
        boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<NonTerminalImpl<D>> {}).template get_or_create<NonTerminalImpl<D>>(node.name);
    boost::hana::at_key(context.m_non_terminals_by_name, boost::hana::type<D> {}).emplace(node.name, non_terminal);
    return non_terminal;
}

template<ConstructorTag D>
static Choice<D> parse(const dl::ast::Choice<D>& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::apply_visitor(
        [&](const auto& arg) -> Choice<D>
        {
            return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ChoiceImpl<D>> {})
                .template get_or_create<ChoiceImpl<D>>(ConstructorOrNonTerminalChoice<D>(parse(arg, domain, ref_grammar_constructor_repos, context)));
        },
        node);
}

static ConceptBot parse(const dl::ast::ConceptBot& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptBotImpl> {}).template get_or_create<ConceptBotImpl>();
}

static ConceptTop parse(const dl::ast::ConceptTop& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptTopImpl> {}).template get_or_create<ConceptTopImpl>();
}

static std::variant<ConceptAtomicState<Static>, ConceptAtomicState<Fluent>, ConceptAtomicState<Derived>>
parse(const dl::ast::ConceptAtomicState& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    if (domain->get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicState from predicates with arity != 1.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptAtomicStateImpl<Static>> {})
            .template get_or_create<ConceptAtomicStateImpl<Static>>(predicate);
    }
    else if (domain->get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicState from predicates with arity != 1.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptAtomicStateImpl<Fluent>> {})
            .template get_or_create<ConceptAtomicStateImpl<Fluent>>(predicate);
    }
    else if (domain->get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicState from predicates with arity != 1.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptAtomicStateImpl<Derived>> {})
            .template get_or_create<ConceptAtomicStateImpl<Derived>>(predicate);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static std::variant<ConceptAtomicGoal<Static>, ConceptAtomicGoal<Fluent>, ConceptAtomicGoal<Derived>>
parse(const dl::ast::ConceptAtomicGoal& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    if (domain->get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicGoal from predicates with arity != 1.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptAtomicGoalImpl<Static>> {})
            .template get_or_create<ConceptAtomicGoalImpl<Static>>(predicate, node.is_negated);
    }
    else if (domain->get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicGoal from predicates with arity != 1.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptAtomicGoalImpl<Fluent>> {})
            .template get_or_create<ConceptAtomicGoalImpl<Fluent>>(predicate, node.is_negated);
    }
    else if (domain->get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicGoal from predicates with arity != 1.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptAtomicGoalImpl<Derived>> {})
            .template get_or_create<ConceptAtomicGoalImpl<Derived>>(predicate, node.is_negated);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static ConceptIntersection
parse(const dl::ast::ConceptIntersection& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptIntersectionImpl> {})
        .template get_or_create<ConceptIntersectionImpl>(parse(node.concept_left, domain, ref_grammar_constructor_repos, context),
                                                         parse(node.concept_right, domain, ref_grammar_constructor_repos, context));
}

static ConceptUnion parse(const dl::ast::ConceptUnion& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptUnionImpl> {})
        .template get_or_create<ConceptUnionImpl>(parse(node.concept_left, domain, ref_grammar_constructor_repos, context),
                                                  parse(node.concept_right, domain, ref_grammar_constructor_repos, context));
}

static ConceptNegation parse(const dl::ast::ConceptNegation& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptNegationImpl> {})
        .template get_or_create<ConceptNegationImpl>(parse(node.concept_, domain, ref_grammar_constructor_repos, context));
}

static ConceptValueRestriction
parse(const dl::ast::ConceptValueRestriction& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptValueRestrictionImpl> {})
        .template get_or_create<ConceptValueRestrictionImpl>(parse(node.role_, domain, ref_grammar_constructor_repos, context),
                                                             parse(node.concept_, domain, ref_grammar_constructor_repos, context));
}

static ConceptExistentialQuantification
parse(const dl::ast::ConceptExistentialQuantification& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptExistentialQuantificationImpl> {})
        .template get_or_create<ConceptExistentialQuantificationImpl>(parse(node.role_, domain, ref_grammar_constructor_repos, context),
                                                                      parse(node.concept_, domain, ref_grammar_constructor_repos, context));
}

static ConceptRoleValueMapContainment
parse(const dl::ast::ConceptRoleValueMapContainment& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptRoleValueMapContainmentImpl> {})
        .template get_or_create<ConceptRoleValueMapContainmentImpl>(parse(node.role_left, domain, ref_grammar_constructor_repos, context),
                                                                    parse(node.role_right, domain, ref_grammar_constructor_repos, context));
}

static ConceptRoleValueMapEquality
parse(const dl::ast::ConceptRoleValueMapEquality& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptRoleValueMapEqualityImpl> {})
        .template get_or_create<ConceptRoleValueMapEqualityImpl>(parse(node.role_left, domain, ref_grammar_constructor_repos, context),
                                                                 parse(node.role_right, domain, ref_grammar_constructor_repos, context));
}

static ConceptNominal parse(const dl::ast::ConceptNominal& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    if (!domain->get_name_to_constants().contains(node.object_name))
    {
        throw std::runtime_error("Domain has no constant with name \"" + node.object_name + "\".");
    }
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<ConceptNominalImpl> {})
        .template get_or_create<ConceptNominalImpl>(domain->get_name_to_constants().at(node.object_name));
}

static RoleUniversal parse(const dl::ast::RoleUniversal& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleUniversalImpl> {}).template get_or_create<RoleUniversalImpl>();
}

static std::variant<RoleAtomicState<Static>, RoleAtomicState<Fluent>, RoleAtomicState<Derived>>
parse(const dl::ast::RoleAtomicState& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    if (domain->get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicState from predicates with arity != 2.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleAtomicStateImpl<Static>> {})
            .template get_or_create<RoleAtomicStateImpl<Static>>(predicate);
    }
    else if (domain->get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicState from predicates with arity != 2.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleAtomicStateImpl<Fluent>> {})
            .template get_or_create<RoleAtomicStateImpl<Fluent>>(predicate);
    }
    else if (domain->get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicState from predicates with arity != 2.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleAtomicStateImpl<Derived>> {})
            .template get_or_create<RoleAtomicStateImpl<Derived>>(predicate);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static std::variant<RoleAtomicGoal<Static>, RoleAtomicGoal<Fluent>, RoleAtomicGoal<Derived>>
parse(const dl::ast::RoleAtomicGoal& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    if (domain->get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicGoal from predicates with arity != 2.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleAtomicGoalImpl<Static>> {})
            .template get_or_create<RoleAtomicGoalImpl<Static>>(predicate, node.is_negated);
    }
    else if (domain->get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicGoal from predicates with arity != 2.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleAtomicGoalImpl<Fluent>> {})
            .template get_or_create<RoleAtomicGoalImpl<Fluent>>(predicate, node.is_negated);
    }
    else if (domain->get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain->get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicGoal from predicates with arity != 2.");
        }
        return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleAtomicGoalImpl<Derived>> {})
            .template get_or_create<RoleAtomicGoalImpl<Derived>>(predicate, node.is_negated);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static RoleIntersection parse(const dl::ast::RoleIntersection& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleIntersectionImpl> {})
        .template get_or_create<RoleIntersectionImpl>(parse(node.role_left, domain, ref_grammar_constructor_repos, context),
                                                      parse(node.role_right, domain, ref_grammar_constructor_repos, context));
}

static RoleUnion parse(const dl::ast::RoleUnion& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleUnionImpl> {})
        .template get_or_create<RoleUnionImpl>(parse(node.role_left, domain, ref_grammar_constructor_repos, context),
                                               parse(node.role_right, domain, ref_grammar_constructor_repos, context));
}

static RoleComplement parse(const dl::ast::RoleComplement& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleComplementImpl> {})
        .template get_or_create<RoleComplementImpl>(parse(node.role_, domain, ref_grammar_constructor_repos, context));
}

static RoleInverse parse(const dl::ast::RoleInverse& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleInverseImpl> {})
        .template get_or_create<RoleInverseImpl>(parse(node.role_, domain, ref_grammar_constructor_repos, context));
}

static RoleComposition parse(const dl::ast::RoleComposition& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleCompositionImpl> {})
        .template get_or_create<RoleCompositionImpl>(parse(node.role_left, domain, ref_grammar_constructor_repos, context),
                                                     parse(node.role_right, domain, ref_grammar_constructor_repos, context));
}

static RoleTransitiveClosure
parse(const dl::ast::RoleTransitiveClosure& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleTransitiveClosureImpl> {})
        .template get_or_create<RoleTransitiveClosureImpl>(parse(node.role_, domain, ref_grammar_constructor_repos, context));
}

static RoleReflexiveTransitiveClosure
parse(const dl::ast::RoleReflexiveTransitiveClosure& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleReflexiveTransitiveClosureImpl> {})
        .template get_or_create<RoleReflexiveTransitiveClosureImpl>(parse(node.role_, domain, ref_grammar_constructor_repos, context));
}

static RoleRestriction parse(const dl::ast::RoleRestriction& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleRestrictionImpl> {})
        .template get_or_create<RoleRestrictionImpl>(parse(node.role_, domain, ref_grammar_constructor_repos, context),
                                                     parse(node.concept_, domain, ref_grammar_constructor_repos, context));
}

static RoleIdentity parse(const dl::ast::RoleIdentity& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    return boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<RoleIdentityImpl> {})
        .template get_or_create<RoleIdentityImpl>(parse(node.concept_, domain, ref_grammar_constructor_repos, context));
}

/**
 * Grammar structure specific types.
 */

template<ConstructorTag D>
static DerivationRule<D>
parse(const dl::ast::DerivationRule<D>& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    auto non_terminal = parse(node.non_terminal, domain, ref_grammar_constructor_repos, context);
    auto choices = ChoiceList<D> {};
    std::for_each(node.choices.begin(),
                  node.choices.end(),
                  [&](const auto& choice) { choices.push_back(parse(choice, domain, ref_grammar_constructor_repos, context)); });
    const auto& rule = boost::hana::at_key(ref_grammar_constructor_repos, boost::hana::type<DerivationRuleImpl<D>> {})
                           .template get_or_create<DerivationRuleImpl<D>>(non_terminal, choices);
    if (boost::hana::at_key(context.m_derivation_rules_by_name, boost::hana::type<D> {}).count(node.non_terminal.name))
    {
        throw std::runtime_error("Got multiple rule definitions for non terminal \"" + node.non_terminal.name + "\". Use choice rules instead.");
    }
    boost::hana::at_key(context.m_derivation_rules_by_name, boost::hana::type<D> {}).emplace(node.non_terminal.name, rule);
    return rule;
}

static ConstructorTagToDerivationRuleList
parse(const dl::ast::Grammar& node, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos, Context& context)
{
    auto rules = ConstructorTagToDerivationRuleList {};

    return rules;

    std::for_each(
        node.rules.begin(),
        node.rules.end(),
        [&](const dl::ast::ConceptOrRoleDerivationRule& rule_node)
        {
            boost::apply_visitor(
                [&](const auto& arg)
                {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, dl::ast::DerivationRule<Concept>>)
                    {
                        boost::hana::at_key(rules, boost::hana::type<Concept> {}).push_back(parse(arg, domain, ref_grammar_constructor_repos, context));
                    }
                    else if constexpr (std::is_same_v<T, dl::ast::DerivationRule<Role>>)
                    {
                        boost::hana::at_key(rules, boost::hana::type<Role> {}).push_back(parse(arg, domain, ref_grammar_constructor_repos, context));
                    }
                    else
                    {
                        throw std::runtime_error("Unknown rule type.");
                    }
                },
                rule_node);
        });

    /* Deferred initialization step */
    for (auto& [non_terminal_name, non_terminal] : boost::hana::at_key(context.m_non_terminals_by_name, boost::hana::type<Concept> {}))
    {
        if (!boost::hana::at_key(context.m_derivation_rules_by_name, boost::hana::type<Concept> {}).count(non_terminal_name))
        {
            throw std::runtime_error("Missing definition for non terminal concept \"" + non_terminal_name + "\".");
        }
        const auto rule = boost::hana::at_key(context.m_derivation_rules_by_name, boost::hana::type<Concept> {}).at(non_terminal_name);
        non_terminal->set_rule(rule);
    }
    for (auto& [non_terminal_name, non_terminal] : boost::hana::at_key(context.m_non_terminals_by_name, boost::hana::type<Role> {}))
    {
        if (!boost::hana::at_key(context.m_derivation_rules_by_name, boost::hana::type<Role> {}).count(non_terminal_name))
        {
            throw std::runtime_error("Missing definition for non terminal role \"" + non_terminal_name + "\".");
        }
        const auto rule = boost::hana::at_key(context.m_derivation_rules_by_name, boost::hana::type<Role> {}).at(non_terminal_name);
        non_terminal->set_rule(rule);
    }

    return rules;
}

ConstructorTagToDerivationRuleList parse(const std::string& bnf_grammar_description, Domain domain, ConstructorTagToRepository& ref_grammar_constructor_repos)
{
    auto ast = dl::ast::Grammar();
    dl::parse_ast(bnf_grammar_description, dl::grammar_parser(), ast);
    auto context = Context();
    return parse(ast, domain, ref_grammar_constructor_repos, context);
}
}
