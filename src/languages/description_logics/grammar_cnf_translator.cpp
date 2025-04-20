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

#include "grammar_cnf_translator.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/languages/description_logics/cnf_grammar.hpp"
#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/languages/description_logics/grammar_visitor_formatter.hpp"

#include <fmt/core.h>

using namespace mimir::formalism;

namespace mimir::languages::dl::grammar
{

/**
 * Step 1: Collect all non terminals to assign novel names to newly generated ones.
 */

class CollectNonTerminalVisitor : public RecurseVisitor
{
private:
    StringToNonTerminalMaps& m_nonterminal_map;

public:
    CollectNonTerminalVisitor(StringToNonTerminalMaps& nonterminal_map) : m_nonterminal_map(nonterminal_map) {}

    void visit(NonTerminal<ConceptTag> constructor) override { visit_impl(constructor); }
    void visit(NonTerminal<RoleTag> constructor) override { visit_impl(constructor); }
    void visit(NonTerminal<BooleanTag> constructor) override { visit_impl(constructor); }
    void visit(NonTerminal<NumericalTag> constructor) override { visit_impl(constructor); }

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(NonTerminal<D> constructor)
    {
        boost::hana::at_key(m_nonterminal_map, boost::hana::type<D> {}).emplace(constructor->get_name(), constructor);
    }
};

static StringToNonTerminalMaps collect_nonterminals_from_grammar(const Grammar& grammar)
{
    auto nonterminal_map = StringToNonTerminalMaps();

    auto visitor = CollectNonTerminalVisitor(nonterminal_map);

    grammar.accept(visitor);

    return nonterminal_map;
}

/**
 * Eliminate choice rules by introducing additional rules
 */

struct EliminateChoiceVisitor : public CopyVisitor
{
    EliminateChoiceVisitor(Repositories& repositories, OptionalNonTerminals& start_symbols, DerivationRuleSets& derivation_rules) :
        CopyVisitor(repositories, start_symbols, derivation_rules)
    {
    }

    void visit(DerivationRule<ConceptTag> constructor) override { visit_impl(constructor); }
    void visit(DerivationRule<RoleTag> constructor) override { visit_impl(constructor); }
    void visit(DerivationRule<BooleanTag> constructor) override { visit_impl(constructor); }
    void visit(DerivationRule<NumericalTag> constructor) override { visit_impl(constructor); }

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(DerivationRule<D> constructor)
    {
        constructor->get_non_terminal()->accept(*this);
        const auto copied_non_terminal = std::any_cast<NonTerminal<D>>(get_result());

        for (const auto& constructor_or_nonterminal : constructor->get_constructor_or_non_terminals())
        {
            constructor_or_nonterminal->accept(*this);

            boost::hana::at_key(this->m_derivation_rules, boost::hana::type<D> {})
                .insert(this->m_repositories.template get_or_create_derivation_rule<D>(
                    copied_non_terminal,
                    ConstructorOrNonTerminalList<D> { std::any_cast<ConstructorOrNonTerminal<D>>(get_result()) }));
        }
    }

    void visit(const Grammar& grammar) override
    {
        boost::hana::for_each(grammar.get_hana_start_symbols(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  if (second.has_value())
                                  {
                                      second.value()->accept(*this);
                                      boost::hana::at_key(m_start_symbols, key) = std::any_cast<NonTerminal<FeatureType>>(get_result());
                                  }
                              });

        boost::hana::for_each(grammar.get_hana_derivation_rules(),
                              [&](auto&& pair)
                              {
                                  const auto& second = boost::hana::second(pair);

                                  for (const auto& rule : second)
                                  {
                                      rule->accept(*this);
                                  }
                              });
    }
};

static Grammar eliminate_choices_in_rules(const Grammar& grammar)
{
    auto repositories = Repositories();
    auto start_symbols = OptionalNonTerminals();
    auto derivation_rules = DerivationRuleSets();

    auto visitor = EliminateChoiceVisitor(repositories, start_symbols, derivation_rules);

    grammar.accept(visitor);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), grammar.get_domain());
}

/**
 * Eliminate nested constructors by introducing additional rules
 */

class EliminateNestedConstructorsVisitor : public CopyVisitor
{
private:
    StringToNonTerminalMaps& m_existing_nonterminals;

    size_t m_next_index;
    size_t m_level;

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    std::string get_free_nonterminal_name()
    {
        std::string candidate_name;
        do
        {
            candidate_name = fmt::format("<{}_{}>", D::name, m_next_index++);
        } while (boost::hana::at_key(m_existing_nonterminals, boost::hana::type<D> {}).contains(candidate_name));

        return candidate_name;
    }

public:
    EliminateNestedConstructorsVisitor(Repositories& repositories,
                                       OptionalNonTerminals& start_symbols,
                                       DerivationRuleSets& derivation_rules,
                                       StringToNonTerminalMaps& existing_nonterminals) :
        CopyVisitor(repositories, start_symbols, derivation_rules),
        m_existing_nonterminals(existing_nonterminals),
        m_next_index(0),
        m_level(0)
    {
    }

    void visit(ConstructorOrNonTerminal<ConceptTag> constructor) override { visit_impl(constructor); }
    void visit(ConstructorOrNonTerminal<RoleTag> constructor) override { visit_impl(constructor); }
    void visit(ConstructorOrNonTerminal<BooleanTag> constructor) override { visit_impl(constructor); }
    void visit(ConstructorOrNonTerminal<NumericalTag> constructor) override { visit_impl(constructor); }

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(ConstructorOrNonTerminal<D> constructor)
    {
        ++m_level;

        std::visit(
            [this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                arg->accept(*this);

                if constexpr (std::is_same_v<T, Constructor<D>>)
                {
                    if (m_level > 1)  // The top-level constructor should not be subtituted
                    {
                        const auto non_terminal = this->m_repositories.template get_or_create_nonterminal<D>(get_free_nonterminal_name<D>());

                        const auto constructor =
                            this->m_repositories.template get_or_create_constructor_or_nonterminal<D>(std::any_cast<Constructor<D>>(get_result()));

                        const auto derivation_rule =
                            this->m_repositories.template get_or_create_derivation_rule<D>(non_terminal, ConstructorOrNonTerminalList<D> { constructor });

                        boost::hana::at_key(this->m_derivation_rules, boost::hana::type<D> {}).insert(derivation_rule);

                        this->m_result = this->m_repositories.template get_or_create_constructor_or_nonterminal<D>(non_terminal);
                    }
                    else
                    {
                        this->m_result = this->m_repositories.template get_or_create_constructor_or_nonterminal<D>(std::any_cast<Constructor<D>>(get_result()));
                    }
                }
                else if constexpr (std::is_same_v<T, NonTerminal<D>>)
                {
                    this->m_result = this->m_repositories.template get_or_create_constructor_or_nonterminal<D>(std::any_cast<NonTerminal<D>>(get_result()));
                }
                else
                {
                    static_assert(dependent_false<T>::value,
                                  "ConstructorOrNonTerminalVisitor<D>::visit(constructor): Missing implementation for ConstructorOrNonTerminal type.");
                }
            },
            constructor->get_constructor_or_non_terminal());

        --m_level;
    }
};

static Grammar eliminate_nested_constructors(const Grammar& grammar)
{
    auto repositories = Repositories();
    auto start_symbols = OptionalNonTerminals();
    auto derivation_rules = DerivationRuleSets();

    auto nonterminal_map = collect_nonterminals_from_grammar(grammar);

    auto visitor = EliminateNestedConstructorsVisitor(repositories, start_symbols, derivation_rules, nonterminal_map);

    grammar.accept(visitor);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), grammar.get_domain());
}

/**
 * ToCnfGrammar
 */

/**
 * Concept
 */

ToCNFVisitor::ToCNFVisitor(cnf_grammar::Repositories& repositories,
                           cnf_grammar::OptionalNonTerminals& start_symbols,
                           cnf_grammar::DerivationRuleLists& derivation_rules,
                           cnf_grammar::SubstitutionRuleLists& substitution_rules) :
    m_repositories(repositories),
    m_start_symbols(start_symbols),
    m_derivation_rules(derivation_rules),
    m_substitution_rules(substitution_rules)
{
}

void ToCNFVisitor::visit(ConceptBot constructor) { m_result = m_repositories.get_or_create_concept_bot(); }
void ToCNFVisitor::visit(ConceptTop constructor) { m_result = m_repositories.get_or_create_concept_top(); }
void ToCNFVisitor::visit(ConceptAtomicState<StaticTag> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void ToCNFVisitor::visit(ConceptAtomicState<FluentTag> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void ToCNFVisitor::visit(ConceptAtomicState<DerivedTag> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void ToCNFVisitor::visit(ConceptAtomicGoal<StaticTag> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->get_polarity());
}
void ToCNFVisitor::visit(ConceptAtomicGoal<FluentTag> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->get_polarity());
}
void ToCNFVisitor::visit(ConceptAtomicGoal<DerivedTag> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->get_polarity());
}
void ToCNFVisitor::visit(ConceptIntersection constructor)
{
    constructor->get_left_concept_or_non_terminal()->accept(*this);
    const auto left_nonterminal =
        std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    constructor->get_right_concept_or_non_terminal()->accept(*this);
    const auto right_nonterminal =
        std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    m_result = m_repositories.get_or_create_concept_intersection(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(ConceptUnion constructor)
{
    constructor->get_left_concept_or_non_terminal()->accept(*this);
    const auto left_nonterminal =
        std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    constructor->get_right_concept_or_non_terminal()->accept(*this);
    const auto right_nonterminal =
        std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    m_result = m_repositories.get_or_create_concept_union(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(ConceptNegation constructor)
{
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto nonterminal = std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    m_result = m_repositories.get_or_create_concept_negation(nonterminal);
}
void ToCNFVisitor::visit(ConceptValueRestriction constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto right_nonterminal =
        std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    m_result = m_repositories.get_or_create_concept_value_restriction(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(ConceptExistentialQuantification constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto right_nonterminal =
        std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    m_result = m_repositories.get_or_create_concept_existential_quantification(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    m_result = m_repositories.get_or_create_concept_role_value_map_containment(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    m_result = m_repositories.get_or_create_concept_role_value_map_equality(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(ConceptNominal constructor) { m_result = m_repositories.get_or_create_concept_nominal(constructor->get_object()); }

void ToCNFVisitor::visit(RoleUniversal constructor) { m_result = m_repositories.get_or_create_role_universal(); }
void ToCNFVisitor::visit(RoleAtomicState<StaticTag> constructor) { m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()); }
void ToCNFVisitor::visit(RoleAtomicState<FluentTag> constructor) { m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()); }
void ToCNFVisitor::visit(RoleAtomicState<DerivedTag> constructor) { m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()); }
void ToCNFVisitor::visit(RoleAtomicGoal<StaticTag> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->get_polarity());
}
void ToCNFVisitor::visit(RoleAtomicGoal<FluentTag> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->get_polarity());
}
void ToCNFVisitor::visit(RoleAtomicGoal<DerivedTag> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->get_polarity());
}
void ToCNFVisitor::visit(RoleIntersection constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    m_result = m_repositories.get_or_create_role_intersection(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(RoleUnion constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    m_result = m_repositories.get_or_create_role_union(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(RoleComplement constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    m_result = m_repositories.get_or_create_role_complement(nonterminal);
}
void ToCNFVisitor::visit(RoleInverse constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    m_result = m_repositories.get_or_create_role_inverse(nonterminal);
}
void ToCNFVisitor::visit(RoleComposition constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    m_result = m_repositories.get_or_create_role_composition(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(RoleTransitiveClosure constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    m_result = m_repositories.get_or_create_role_transitive_closure(nonterminal);
}
void ToCNFVisitor::visit(RoleReflexiveTransitiveClosure constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    m_result = m_repositories.get_or_create_role_reflexive_transitive_closure(nonterminal);
}
void ToCNFVisitor::visit(RoleRestriction constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto right_nonterminal =
        std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    m_result = m_repositories.get_or_create_role_restriction(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(RoleIdentity constructor)
{
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto nonterminal = std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    m_result = m_repositories.get_or_create_role_identity(nonterminal);
}

void ToCNFVisitor::visit(BooleanAtomicState<StaticTag> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void ToCNFVisitor::visit(BooleanAtomicState<FluentTag> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void ToCNFVisitor::visit(BooleanAtomicState<DerivedTag> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void ToCNFVisitor::visit(BooleanNonempty<ConceptTag> constructor)
{
    constructor->get_constructor_or_nonterminal()->accept(*this);
    const auto concept_nonterminal =
        std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    m_result = m_repositories.get_or_create_boolean_nonempty(concept_nonterminal);
}
void ToCNFVisitor::visit(BooleanNonempty<RoleTag> constructor)
{
    constructor->get_constructor_or_nonterminal()->accept(*this);
    const auto role_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    m_result = m_repositories.get_or_create_boolean_nonempty(role_nonterminal);
}

void ToCNFVisitor::visit(NumericalCount<ConceptTag> constructor)
{
    constructor->get_constructor_or_nonterminal()->accept(*this);
    const auto concept_nonterminal =
        std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    m_result = m_repositories.get_or_create_numerical_count(concept_nonterminal);
}
void ToCNFVisitor::visit(NumericalCount<RoleTag> constructor)
{
    constructor->get_constructor_or_nonterminal()->accept(*this);
    const auto role_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    m_result = m_repositories.get_or_create_numerical_count(role_nonterminal);
}
void ToCNFVisitor::visit(NumericalDistance constructor)
{
    constructor->get_left_concept_or_nonterminal()->accept(*this);
    const auto left_concept_nonterminal =
        std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    constructor->get_role_or_nonterminal()->accept(*this);
    const auto role_nonterminal = std::get<cnf_grammar::NonTerminal<RoleTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<RoleTag>>(get_result()));
    constructor->get_right_concept_or_nonterminal()->accept(*this);
    const auto right_concept_nonterminal =
        std::get<cnf_grammar::NonTerminal<ConceptTag>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<ConceptTag>>(get_result()));
    m_result = m_repositories.get_or_create_numerical_distance(left_concept_nonterminal, role_nonterminal, right_concept_nonterminal);
}

/**
 * NonTerminal
 */

void ToCNFVisitor::visit(NonTerminal<ConceptTag> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(NonTerminal<RoleTag> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(NonTerminal<BooleanTag> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(NonTerminal<NumericalTag> constructor) { visit_impl(constructor); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void ToCNFVisitor::visit_impl(NonTerminal<D> constructor)
{
    m_result = m_repositories.template get_or_create_nonterminal<D>(constructor->get_name());
}

template void ToCNFVisitor::visit_impl(NonTerminal<ConceptTag> constructor);
template void ToCNFVisitor::visit_impl(NonTerminal<RoleTag> constructor);
template void ToCNFVisitor::visit_impl(NonTerminal<BooleanTag> constructor);
template void ToCNFVisitor::visit_impl(NonTerminal<NumericalTag> constructor);

/**
 * ConstructorOrNonTerminal
 */

void ToCNFVisitor::visit(ConstructorOrNonTerminal<ConceptTag> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(ConstructorOrNonTerminal<RoleTag> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(ConstructorOrNonTerminal<BooleanTag> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(ConstructorOrNonTerminal<NumericalTag> constructor) { visit_impl(constructor); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void ToCNFVisitor::visit_impl(ConstructorOrNonTerminal<D> constructor)
{
    std::visit(
        [this](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;

            arg->accept(*this);

            if constexpr (std::is_same_v<T, Constructor<D>>)
            {
                m_result = cnf_grammar::ConstructorOrNonTerminal<D>(std::any_cast<cnf_grammar::Constructor<D>>(get_result()));
            }
            else if constexpr (std::is_same_v<T, NonTerminal<D>>)
            {
                m_result = cnf_grammar::ConstructorOrNonTerminal<D>(std::any_cast<cnf_grammar::NonTerminal<D>>(get_result()));
            }
            else
            {
                static_assert(dependent_false<T>::value,
                              "ConstructorOrNonTerminalVisitor<D>::visit(constructor): Missing implementation for ConstructorOrNonTerminal type.");
            }
        },
        constructor->get_constructor_or_non_terminal());
}

template void ToCNFVisitor::visit_impl(ConstructorOrNonTerminal<ConceptTag> constructor);
template void ToCNFVisitor::visit_impl(ConstructorOrNonTerminal<RoleTag> constructor);
template void ToCNFVisitor::visit_impl(ConstructorOrNonTerminal<BooleanTag> constructor);
template void ToCNFVisitor::visit_impl(ConstructorOrNonTerminal<NumericalTag> constructor);

/**
 * DerivationRule
 */

void ToCNFVisitor::visit(DerivationRule<ConceptTag> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(DerivationRule<RoleTag> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(DerivationRule<BooleanTag> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(DerivationRule<NumericalTag> constructor) { visit_impl(constructor); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void ToCNFVisitor::visit_impl(DerivationRule<D> constructor)
{
    constructor->get_non_terminal()->accept(*this);
    const auto head = std::any_cast<cnf_grammar::NonTerminal<D>>(get_result());

    if (constructor->get_constructor_or_non_terminals().size() != 1)
    {
        throw std::logic_error("ToCNFDerivationRuleVisitor<D>::visit(constructor): Expected single choice");
    }

    constructor->get_constructor_or_non_terminals().front()->accept(*this);
    const auto body_variant = std::any_cast<cnf_grammar::ConstructorOrNonTerminal<D>>(get_result());

    std::visit(
        [&](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, cnf_grammar::Constructor<D>>)
            {
                m_result = cnf_grammar::DerivationOrSubstitutionRule<D> { this->m_repositories.get_or_create_derivation_rule(head, arg) };
            }
            else if constexpr (std::is_same_v<T, cnf_grammar::NonTerminal<D>>)
            {
                m_result = cnf_grammar::DerivationOrSubstitutionRule<D> { this->m_repositories.get_or_create_substitution_rule(head, arg) };
            }
            else
            {
                static_assert(dependent_false<T>::value,
                              "ToCNFDerivationRuleVisitor<T>::visit(constructor): Missing implementation for ConstructorOrNonTerminal type.");
            }
        },
        body_variant);
}

template void ToCNFVisitor::visit_impl(DerivationRule<ConceptTag> constructor);
template void ToCNFVisitor::visit_impl(DerivationRule<RoleTag> constructor);
template void ToCNFVisitor::visit_impl(DerivationRule<BooleanTag> constructor);
template void ToCNFVisitor::visit_impl(DerivationRule<NumericalTag> constructor);

/**
 * Grammar
 */

void ToCNFVisitor::visit(const Grammar& grammar)
{
    boost::hana::for_each(grammar.get_hana_start_symbols(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);
                              using FeatureType = typename decltype(+key)::type;

                              if (second.has_value())
                              {
                                  second.value()->accept(*this);
                                  boost::hana::at_key(m_start_symbols, key) = std::any_cast<cnf_grammar::NonTerminal<FeatureType>>(get_result());
                              }
                          });

    boost::hana::for_each(
        grammar.get_hana_derivation_rules(),
        [&](auto&& pair)
        {
            auto key = boost::hana::first(pair);
            const auto& second = boost::hana::second(pair);
            using FeatureType = typename decltype(+key)::type;

            for (const auto& rule : second)
            {
                rule->accept(*this);
                const auto rule_variant = std::any_cast<cnf_grammar::DerivationOrSubstitutionRule<FeatureType>>(get_result());

                std::visit(
                    [&](auto&& arg)
                    {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, cnf_grammar::DerivationRule<ConceptTag>> || std::is_same_v<T, cnf_grammar::DerivationRule<RoleTag>>
                                      || std::is_same_v<T, cnf_grammar::DerivationRule<BooleanTag>>
                                      || std::is_same_v<T, cnf_grammar::DerivationRule<NumericalTag>>)
                        {
                            boost::hana::at_key(m_derivation_rules, key).push_back(arg);
                        }
                        else if constexpr (std::is_same_v<T, cnf_grammar::SubstitutionRule<ConceptTag>>
                                           || std::is_same_v<T, cnf_grammar::SubstitutionRule<RoleTag>>
                                           || std::is_same_v<T, cnf_grammar::SubstitutionRule<BooleanTag>>
                                           || std::is_same_v<T, cnf_grammar::SubstitutionRule<NumericalTag>>)
                        {
                            boost::hana::at_key(m_substitution_rules, key).push_back(arg);
                        }
                        else
                        {
                            static_assert(dependent_false<T>::value,
                                          "ToCNFGrammarVisitor::visit(constructor): Missing implementation for DerivationOrSubstitutionRule type.");
                        }
                    },
                    rule_variant);
            }
        });
}

const std::any& ToCNFVisitor::get_result() const { return m_result; }

static cnf_grammar::Grammar parse_cnf_grammar(const Grammar& grammar)
{
    auto repositories = cnf_grammar::Repositories();
    auto start_symbols = cnf_grammar::OptionalNonTerminals();
    auto derivation_rules = cnf_grammar::DerivationRuleLists();
    auto substitution_rules = cnf_grammar::SubstitutionRuleLists();

    auto visitor = ToCNFVisitor(repositories, start_symbols, derivation_rules, substitution_rules);

    grammar.accept(visitor);

    return cnf_grammar::Grammar(std::move(repositories),
                                std::move(start_symbols),
                                std::move(derivation_rules),
                                std::move(substitution_rules),
                                grammar.get_domain());
}

cnf_grammar::Grammar translate_to_cnf(const Grammar& grammar)
{
    auto translated_grammar = eliminate_choices_in_rules(grammar);

    translated_grammar = eliminate_nested_constructors(translated_grammar);

    return parse_cnf_grammar(translated_grammar);
}
}
