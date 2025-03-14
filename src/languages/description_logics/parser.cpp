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

namespace mimir::languages::dl::grammar
{

template<FeatureCategory D>
static Constructor<D> parse(const dl::ast::Constructor<D>& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return boost::apply_visitor([&](const auto& arg) -> Constructor<D> { return parse(arg, domain, ref_repositories); }, node);
}

template<FeatureCategory D>
static NonTerminal<D> parse(const dl::ast::NonTerminal<D>& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_nonterminal<D>(node.name);
}

template<FeatureCategory D>
static ConstructorOrNonTerminal<D> parse(const dl::ast::ConstructorOrNonTerminal<D>& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return boost::apply_visitor([&](const auto& arg) -> ConstructorOrNonTerminal<D>
                                { return ref_repositories.template get_or_create_constructor_or_nonterminal<D>(parse(arg, domain, ref_repositories)); },
                                node);
}

static Constructor<Concept> parse(const dl::ast::ConceptBot& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_bot();
}

static Constructor<Concept> parse(const dl::ast::ConceptTop& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_top();
}

static Constructor<Concept> parse(const dl::ast::ConceptAtomicState& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    if (domain.get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicState from predicates with arity != 1.");
        }
        return ref_repositories.template get_or_create_concept_atomic_state<Static>(predicate);
    }
    else if (domain.get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicState from predicates with arity != 1.");
        }
        return ref_repositories.template get_or_create_concept_atomic_state<Fluent>(predicate);
    }
    else if (domain.get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicState from predicates with arity != 1.");
        }
        return ref_repositories.template get_or_create_concept_atomic_state<Derived>(predicate);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Constructor<Concept> parse(const dl::ast::ConceptAtomicGoal& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    if (domain.get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicGoal from predicates with arity != 1.");
        }
        return ref_repositories.template get_or_create_concept_atomic_goal<Static>(predicate, node.is_negated);
    }
    else if (domain.get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicGoal from predicates with arity != 1.");
        }
        return ref_repositories.template get_or_create_concept_atomic_goal<Fluent>(predicate, node.is_negated);
    }
    else if (domain.get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicGoal from predicates with arity != 1.");
        }
        return ref_repositories.template get_or_create_concept_atomic_goal<Derived>(predicate, node.is_negated);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Constructor<Concept> parse(const dl::ast::ConceptIntersection& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_intersection(parse(node.left_concept_or_non_terminal, domain, ref_repositories),
                                                               parse(node.right_concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<Concept> parse(const dl::ast::ConceptUnion& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_union(parse(node.left_concept_or_non_terminal, domain, ref_repositories),
                                                        parse(node.right_concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<Concept> parse(const dl::ast::ConceptNegation& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_negation(parse(node.concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<Concept> parse(const dl::ast::ConceptValueRestriction& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_value_restriction(parse(node.role_or_non_terminal, domain, ref_repositories),
                                                                    parse(node.concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<Concept> parse(const dl::ast::ConceptExistentialQuantification& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_existential_quantification(parse(node.role_or_non_terminal, domain, ref_repositories),
                                                                             parse(node.concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<Concept> parse(const dl::ast::ConceptRoleValueMapContainment& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_role_value_map_containment(parse(node.left_role_or_non_terminal, domain, ref_repositories),
                                                                             parse(node.right_role_or_non_terminal, domain, ref_repositories));
}

static Constructor<Concept> parse(const dl::ast::ConceptRoleValueMapEquality& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_role_value_map_equality(parse(node.left_role_or_non_terminal, domain, ref_repositories),
                                                                          parse(node.right_role_or_non_terminal, domain, ref_repositories));
}

static Constructor<Concept> parse(const dl::ast::ConceptNominal& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    if (!domain.get_name_to_constant().contains(node.object_name))
    {
        throw std::runtime_error("Domain has no constant with name \"" + node.object_name + "\".");
    }

    return ref_repositories.get_or_create_concept_nominal(domain.get_name_to_constant().at(node.object_name));
}

static Constructor<Role> parse(const dl::ast::RoleUniversal& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_universal();
}

static Constructor<Role> parse(const dl::ast::RoleAtomicState& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    if (domain.get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicState from predicates with arity != 2.");
        }
        return ref_repositories.template get_or_create_role_atomic_state<Static>(predicate);
    }
    else if (domain.get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicState from predicates with arity != 2.");
        }
        return ref_repositories.template get_or_create_role_atomic_state<Fluent>(predicate);
    }
    else if (domain.get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicState from predicates with arity != 2.");
        }
        return ref_repositories.template get_or_create_role_atomic_state<Derived>(predicate);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Constructor<Role> parse(const dl::ast::RoleAtomicGoal& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    if (domain.get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicGoal from predicates with arity != 2.");
        }
        return ref_repositories.template get_or_create_role_atomic_goal<Static>(predicate, node.is_negated);
    }
    else if (domain.get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicGoal from predicates with arity != 2.");
        }
        return ref_repositories.template get_or_create_role_atomic_goal<Fluent>(predicate, node.is_negated);
    }
    else if (domain.get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicGoal from predicates with arity != 2.");
        }
        return ref_repositories.template get_or_create_role_atomic_goal<Derived>(predicate, node.is_negated);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Constructor<Role> parse(const dl::ast::RoleIntersection& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_intersection(parse(node.left_role_or_non_terminal, domain, ref_repositories),
                                                            parse(node.right_role_or_non_terminal, domain, ref_repositories));
}

static Constructor<Role> parse(const dl::ast::RoleUnion& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_union(parse(node.left_role_or_non_terminal, domain, ref_repositories),
                                                     parse(node.right_role_or_non_terminal, domain, ref_repositories));
}

static Constructor<Role> parse(const dl::ast::RoleComplement& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_complement(parse(node.role_or_non_terminal, domain, ref_repositories));
}

static Constructor<Role> parse(const dl::ast::RoleInverse& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_inverse(parse(node.role_or_non_terminal, domain, ref_repositories));
}

static Constructor<Role> parse(const dl::ast::RoleComposition& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_composition(parse(node.left_role_or_non_terminal, domain, ref_repositories),
                                                           parse(node.right_role_or_non_terminal, domain, ref_repositories));
}

static Constructor<Role> parse(const dl::ast::RoleTransitiveClosure& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_transitive_closure(parse(node.role_or_non_terminal, domain, ref_repositories));
}

static Constructor<Role> parse(const dl::ast::RoleReflexiveTransitiveClosure& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_reflexive_transitive_closure(parse(node.role_or_non_terminal, domain, ref_repositories));
}

static Constructor<Role> parse(const dl::ast::RoleRestriction& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_restriction(parse(node.role_or_non_terminal, domain, ref_repositories),
                                                           parse(node.concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<Role> parse(const dl::ast::RoleIdentity& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_identity(parse(node.concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<Boolean> parse(const dl::ast::BooleanAtomicState& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    if (domain.get_name_to_predicate<Static>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Static>().at(node.predicate_name);
        if (predicate->get_arity() != 0)
        {
            throw std::runtime_error("Cannot construct BooleanAtomicState from predicates with arity != 0.");
        }
        return ref_repositories.template get_or_create_boolean_atomic_state<Static>(predicate);
    }
    else if (domain.get_name_to_predicate<Fluent>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Fluent>().at(node.predicate_name);
        if (predicate->get_arity() != 0)
        {
            throw std::runtime_error("Cannot construct BooleanAtomicState from predicates with arity != 0.");
        }
        return ref_repositories.template get_or_create_boolean_atomic_state<Fluent>(predicate);
    }
    else if (domain.get_name_to_predicate<Derived>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<Derived>().at(node.predicate_name);
        if (predicate->get_arity() != 0)
        {
            throw std::runtime_error("Cannot construct BooleanAtomicState from predicates with arity != 0.");
        }
        return ref_repositories.template get_or_create_boolean_atomic_state<Derived>(predicate);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Constructor<Boolean> parse(const dl::ast::BooleanNonempty& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return boost::apply_visitor([&](auto&& arg) -> Constructor<Boolean>
                                { return ref_repositories.get_or_create_boolean_nonempty(parse(arg, domain, ref_repositories)); },
                                node.concept_or_role_nonterminal);
}

static Constructor<Numerical> parse(const dl::ast::NumericalCount& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return boost::apply_visitor([&](auto&& arg) -> Constructor<Numerical>
                                { return ref_repositories.get_or_create_numerical_count(parse(arg, domain, ref_repositories)); },
                                node.concept_or_role_nonterminal);
}

static Constructor<Numerical> parse(const dl::ast::NumericalDistance& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    return ref_repositories.get_or_create_numerical_distance(parse(node.left_concept_or_nonterminal, domain, ref_repositories),
                                                             parse(node.role_or_nonterminal, domain, ref_repositories),
                                                             parse(node.right_concept_or_nonterminal, domain, ref_repositories));
}

/**
 * Grammar structure specific types.
 */

template<FeatureCategory D>
static DerivationRule<D> parse(const dl::ast::DerivationRule<D>& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    auto non_terminal = parse<D>(node.non_terminal, domain, ref_repositories);
    auto constructor_or_non_terminals = ConstructorOrNonTerminalList<D> {};
    std::for_each(node.constructor_or_non_terminals.begin(),
                  node.constructor_or_non_terminals.end(),
                  [&](const auto& choice) { constructor_or_non_terminals.push_back(parse(choice, domain, ref_repositories)); });

    return ref_repositories.template get_or_create_derivation_rule<D>(non_terminal, std::move(constructor_or_non_terminals));
}

static StartSymbolsContainer parse(const dl::ast::GrammarHead& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    auto start_symbols = StartSymbolsContainer();

    if (node.concept_start)
    {
        start_symbols.insert(parse(node.concept_start.value(), domain, ref_repositories));
    }

    if (node.role_start)
    {
        start_symbols.insert(parse(node.role_start.value(), domain, ref_repositories));
    }
    if (node.boolean_start)
    {
        start_symbols.insert(parse(node.boolean_start.value(), domain, ref_repositories));
    }

    if (node.numerical_start)
    {
        start_symbols.insert(parse(node.numerical_start.value(), domain, ref_repositories));
    }

    return start_symbols;
}

static DerivationRulesContainer parse(const dl::ast::GrammarBody& node, const DomainImpl& domain, ConstructorRepositories& ref_repositories)
{
    auto derivation_rules = DerivationRulesContainer {};

    for (const auto& part : node.rules)
    {
        boost::apply_visitor([&](const auto& arg) { derivation_rules.insert(parse(arg, domain, ref_repositories)); }, part);
    }

    return derivation_rules;
}

Grammar parse(const std::string& bnf_description, Domain domain)
{
    auto ast = dl::ast::Grammar();
    dl::parse_ast(bnf_description, dl::grammar_parser(), ast);

    auto repositories = ConstructorRepositories();
    auto start_symbols = parse(ast.head, *domain, repositories);
    auto derivation_rules = parse(ast.body, *domain, repositories);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), std::move(domain));
}

}
