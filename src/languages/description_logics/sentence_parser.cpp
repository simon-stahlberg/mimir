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

#include "sentence_parser.hpp"

#include "../../common/x3/parser_wrapper.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "parser/ast.hpp"
#include "sentence_parser/parser.hpp"

using namespace mimir::formalism;

namespace mimir::languages::dl
{
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
Constructor<D> parse(const dl::ast::Constructor<D>& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return boost::apply_visitor([&](const auto& arg) -> Constructor<D> { return parse(arg, domain, ref_repositories); }, node);
}

template Constructor<ConceptTag> parse(const dl::ast::Constructor<ConceptTag>& node, const DomainImpl& domain, Repositories& ref_repositories);
template Constructor<RoleTag> parse(const dl::ast::Constructor<RoleTag>& node, const DomainImpl& domain, Repositories& ref_repositories);
template Constructor<BooleanTag> parse(const dl::ast::Constructor<BooleanTag>& node, const DomainImpl& domain, Repositories& ref_repositories);
template Constructor<NumericalTag> parse(const dl::ast::Constructor<NumericalTag>& node, const DomainImpl& domain, Repositories& ref_repositories);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
static Constructor<D> parse(const dl::ast::ConstructorOrNonTerminal<D>& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return boost::apply_visitor(
        [&](const auto& arg) -> Constructor<D>
        {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, dl::ast::NonTerminal<D>>)
            {
                throw std::logic_error("Parsing error: Encountered NonTerminal during sentence parsing.");
            }
            else if constexpr (std::is_same_v<T, dl::ast::Constructor<D>>)
            {
                return parse(arg, domain, ref_repositories);
            }
            else
            {
                static_assert(dependent_false<T>::value, "parse(node, domain, ref_repositories): Undefined ConstructorOrNonTerminal type.");
            }
        },
        node);
}

static Constructor<ConceptTag> parse(const dl::ast::ConceptBot& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_bot();
}

static Constructor<ConceptTag> parse(const dl::ast::ConceptTop& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_top();
}

static Constructor<ConceptTag> parse(const dl::ast::ConceptAtomicState& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    if (domain.get_name_to_predicate<StaticTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<StaticTag>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicState from predicates with arity != 1.");
        }
        return ref_repositories.template get_or_create_concept_atomic_state<StaticTag>(predicate);
    }
    else if (domain.get_name_to_predicate<FluentTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<FluentTag>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicState from predicates with arity != 1.");
        }
        return ref_repositories.template get_or_create_concept_atomic_state<FluentTag>(predicate);
    }
    else if (domain.get_name_to_predicate<DerivedTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<DerivedTag>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicState from predicates with arity != 1.");
        }
        return ref_repositories.template get_or_create_concept_atomic_state<DerivedTag>(predicate);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Constructor<ConceptTag> parse(const dl::ast::ConceptAtomicGoal& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    if (domain.get_name_to_predicate<StaticTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<StaticTag>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicGoal from predicates with arity != 1.");
        }
        return ref_repositories.template get_or_create_concept_atomic_goal<StaticTag>(predicate, node.polarity);
    }
    else if (domain.get_name_to_predicate<FluentTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<FluentTag>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicGoal from predicates with arity != 1.");
        }
        return ref_repositories.template get_or_create_concept_atomic_goal<FluentTag>(predicate, node.polarity);
    }
    else if (domain.get_name_to_predicate<DerivedTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<DerivedTag>().at(node.predicate_name);
        if (predicate->get_arity() != 1)
        {
            throw std::runtime_error("Cannot construct ConceptAtomicGoal from predicates with arity != 1.");
        }
        return ref_repositories.template get_or_create_concept_atomic_goal<DerivedTag>(predicate, node.polarity);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Constructor<ConceptTag> parse(const dl::ast::ConceptIntersection& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_intersection(parse(node.left_concept_or_non_terminal, domain, ref_repositories),
                                                               parse(node.right_concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<ConceptTag> parse(const dl::ast::ConceptUnion& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_union(parse(node.left_concept_or_non_terminal, domain, ref_repositories),
                                                        parse(node.right_concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<ConceptTag> parse(const dl::ast::ConceptNegation& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_negation(parse(node.concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<ConceptTag> parse(const dl::ast::ConceptValueRestriction& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_value_restriction(parse(node.role_or_non_terminal, domain, ref_repositories),
                                                                    parse(node.concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<ConceptTag> parse(const dl::ast::ConceptExistentialQuantification& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_existential_quantification(parse(node.role_or_non_terminal, domain, ref_repositories),
                                                                             parse(node.concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<ConceptTag> parse(const dl::ast::ConceptRoleValueMapContainment& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_role_value_map_containment(parse(node.left_role_or_non_terminal, domain, ref_repositories),
                                                                             parse(node.right_role_or_non_terminal, domain, ref_repositories));
}

static Constructor<ConceptTag> parse(const dl::ast::ConceptRoleValueMapEquality& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_concept_role_value_map_equality(parse(node.left_role_or_non_terminal, domain, ref_repositories),
                                                                          parse(node.right_role_or_non_terminal, domain, ref_repositories));
}

static Constructor<ConceptTag> parse(const dl::ast::ConceptNominal& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    if (!domain.get_name_to_constant().contains(node.object_name))
    {
        throw std::runtime_error("Domain has no constant with name \"" + node.object_name + "\".");
    }

    return ref_repositories.get_or_create_concept_nominal(domain.get_name_to_constant().at(node.object_name));
}

static Constructor<RoleTag> parse(const dl::ast::RoleUniversal& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_universal();
}

static Constructor<RoleTag> parse(const dl::ast::RoleAtomicState& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    if (domain.get_name_to_predicate<StaticTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<StaticTag>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicState from predicates with arity != 2.");
        }
        return ref_repositories.template get_or_create_role_atomic_state<StaticTag>(predicate);
    }
    else if (domain.get_name_to_predicate<FluentTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<FluentTag>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicState from predicates with arity != 2.");
        }
        return ref_repositories.template get_or_create_role_atomic_state<FluentTag>(predicate);
    }
    else if (domain.get_name_to_predicate<DerivedTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<DerivedTag>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicState from predicates with arity != 2.");
        }
        return ref_repositories.template get_or_create_role_atomic_state<DerivedTag>(predicate);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Constructor<RoleTag> parse(const dl::ast::RoleAtomicGoal& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    if (domain.get_name_to_predicate<StaticTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<StaticTag>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicGoal from predicates with arity != 2.");
        }
        return ref_repositories.template get_or_create_role_atomic_goal<StaticTag>(predicate, node.polarity);
    }
    else if (domain.get_name_to_predicate<FluentTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<FluentTag>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicGoal from predicates with arity != 2.");
        }
        return ref_repositories.template get_or_create_role_atomic_goal<FluentTag>(predicate, node.polarity);
    }
    else if (domain.get_name_to_predicate<DerivedTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<DerivedTag>().at(node.predicate_name);
        if (predicate->get_arity() != 2)
        {
            throw std::runtime_error("Cannot construct RoleAtomicGoal from predicates with arity != 2.");
        }
        return ref_repositories.template get_or_create_role_atomic_goal<DerivedTag>(predicate, node.polarity);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Constructor<RoleTag> parse(const dl::ast::RoleIntersection& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_intersection(parse(node.left_role_or_non_terminal, domain, ref_repositories),
                                                            parse(node.right_role_or_non_terminal, domain, ref_repositories));
}

static Constructor<RoleTag> parse(const dl::ast::RoleUnion& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_union(parse(node.left_role_or_non_terminal, domain, ref_repositories),
                                                     parse(node.right_role_or_non_terminal, domain, ref_repositories));
}

static Constructor<RoleTag> parse(const dl::ast::RoleComplement& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_complement(parse(node.role_or_non_terminal, domain, ref_repositories));
}

static Constructor<RoleTag> parse(const dl::ast::RoleInverse& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_inverse(parse(node.role_or_non_terminal, domain, ref_repositories));
}

static Constructor<RoleTag> parse(const dl::ast::RoleComposition& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_composition(parse(node.left_role_or_non_terminal, domain, ref_repositories),
                                                           parse(node.right_role_or_non_terminal, domain, ref_repositories));
}

static Constructor<RoleTag> parse(const dl::ast::RoleTransitiveClosure& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_transitive_closure(parse(node.role_or_non_terminal, domain, ref_repositories));
}

static Constructor<RoleTag> parse(const dl::ast::RoleReflexiveTransitiveClosure& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_reflexive_transitive_closure(parse(node.role_or_non_terminal, domain, ref_repositories));
}

static Constructor<RoleTag> parse(const dl::ast::RoleRestriction& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_restriction(parse(node.role_or_non_terminal, domain, ref_repositories),
                                                           parse(node.concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<RoleTag> parse(const dl::ast::RoleIdentity& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_role_identity(parse(node.concept_or_non_terminal, domain, ref_repositories));
}

static Constructor<BooleanTag> parse(const dl::ast::BooleanAtomicState& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    if (domain.get_name_to_predicate<StaticTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<StaticTag>().at(node.predicate_name);
        if (predicate->get_arity() != 0)
        {
            throw std::runtime_error("Cannot construct BooleanAtomicState from predicates with arity != 0.");
        }
        return ref_repositories.template get_or_create_boolean_atomic_state<StaticTag>(predicate);
    }
    else if (domain.get_name_to_predicate<FluentTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<FluentTag>().at(node.predicate_name);
        if (predicate->get_arity() != 0)
        {
            throw std::runtime_error("Cannot construct BooleanAtomicState from predicates with arity != 0.");
        }
        return ref_repositories.template get_or_create_boolean_atomic_state<FluentTag>(predicate);
    }
    else if (domain.get_name_to_predicate<DerivedTag>().count(node.predicate_name))
    {
        const auto predicate = domain.get_name_to_predicate<DerivedTag>().at(node.predicate_name);
        if (predicate->get_arity() != 0)
        {
            throw std::runtime_error("Cannot construct BooleanAtomicState from predicates with arity != 0.");
        }
        return ref_repositories.template get_or_create_boolean_atomic_state<DerivedTag>(predicate);
    }
    else
    {
        throw std::runtime_error("Predicate \"" + node.predicate_name + "\" is not part of the given domain.");
    }
}

static Constructor<BooleanTag> parse(const dl::ast::BooleanNonempty& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return boost::apply_visitor([&](auto&& arg) -> Constructor<BooleanTag>
                                { return ref_repositories.get_or_create_boolean_nonempty(parse(arg, domain, ref_repositories)); },
                                node.concept_or_role_nonterminal);
}

static Constructor<NumericalTag> parse(const dl::ast::NumericalCount& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return boost::apply_visitor([&](auto&& arg) -> Constructor<NumericalTag>
                                { return ref_repositories.get_or_create_numerical_count(parse(arg, domain, ref_repositories)); },
                                node.concept_or_role_nonterminal);
}

static Constructor<NumericalTag> parse(const dl::ast::NumericalDistance& node, const DomainImpl& domain, Repositories& ref_repositories)
{
    return ref_repositories.get_or_create_numerical_distance(parse(node.left_concept_or_nonterminal, domain, ref_repositories),
                                                             parse(node.role_or_nonterminal, domain, ref_repositories),
                                                             parse(node.right_concept_or_nonterminal, domain, ref_repositories));
}

template<>
Constructor<ConceptTag> parse_sentence(const std::string& sentence, const formalism::DomainImpl& domain, Repositories& repositories)
{
    auto ast = dl::ast::Constructor<ConceptTag>();
    mimir::x3::parse_ast(sentence, dl::sentence_parser::concept_root_parser(), ast);

    return parse(ast, domain, repositories);
}

template<>
Constructor<RoleTag> parse_sentence(const std::string& sentence, const formalism::DomainImpl& domain, Repositories& repositories)
{
    auto ast = dl::ast::Constructor<RoleTag>();
    mimir::x3::parse_ast(sentence, dl::sentence_parser::role_root_parser(), ast);

    return parse(ast, domain, repositories);
}

template<>
Constructor<BooleanTag> parse_sentence(const std::string& sentence, const formalism::DomainImpl& domain, Repositories& repositories)
{
    auto ast = dl::ast::Constructor<BooleanTag>();
    mimir::x3::parse_ast(sentence, dl::sentence_parser::boolean_root_parser(), ast);

    return parse(ast, domain, repositories);
}

template<>
Constructor<NumericalTag> parse_sentence(const std::string& sentence, const formalism::DomainImpl& domain, Repositories& repositories)
{
    auto ast = dl::ast::Constructor<NumericalTag>();
    mimir::x3::parse_ast(sentence, dl::sentence_parser::numerical_root_parser(), ast);

    return parse(ast, domain, repositories);
}

}
