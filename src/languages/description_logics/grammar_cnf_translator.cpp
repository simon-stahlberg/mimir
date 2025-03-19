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

using namespace mimir::formalism;

namespace mimir::languages::dl::grammar
{

/**
 * Step 1: Collect all non terminals to assign novel names to newly generated ones.
 */

class CollectNonTerminalVisitor : public RecurseVisitor
{
private:
    ToNonTerminalMap<std::string, Concept, Role, Boolean, Numerical>& m_nonterminal_map;

public:
    CollectNonTerminalVisitor(ToNonTerminalMap<std::string, Concept, Role, Boolean, Numerical>& nonterminal_map) : m_nonterminal_map(nonterminal_map) {}

    void visit(NonTerminal<Concept> constructor) override { visit_impl(constructor); }
    void visit(NonTerminal<Role> constructor) override { visit_impl(constructor); }
    void visit(NonTerminal<Boolean> constructor) override { visit_impl(constructor); }
    void visit(NonTerminal<Numerical> constructor) override { visit_impl(constructor); }

    template<FeatureCategory D>
    void visit_impl(NonTerminal<D> constructor)
    {
        boost::hana::at_key(m_nonterminal_map, boost::hana::type<D> {}).emplace(constructor->get_name(), constructor);
    }
};

static ToNonTerminalMap<std::string, Concept, Role, Boolean, Numerical> collect_nonterminals_from_grammar(const Grammar& grammar)
{
    auto nonterminal_map = ToNonTerminalMap<std::string, Concept, Role, Boolean, Numerical>();

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

    void visit(DerivationRule<Concept> constructor) override { visit_impl(constructor); }
    void visit(DerivationRule<Role> constructor) override { visit_impl(constructor); }
    void visit(DerivationRule<Boolean> constructor) override { visit_impl(constructor); }
    void visit(DerivationRule<Numerical> constructor) override { visit_impl(constructor); }

    template<FeatureCategory D>
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
        boost::hana::for_each(grammar.get_start_symbols(),
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

        boost::hana::for_each(grammar.get_derivation_rules(),
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
    ToNonTerminalMap<std::string, Concept, Role, Boolean, Numerical>& m_existing_nonterminals;

    size_t m_next_index;
    size_t m_level;

    template<FeatureCategory D>
    std::string get_free_nonterminal_name()
    {
        std::string candidate_name;
        do
        {
            candidate_name = "<" + D::name + "_" + std::to_string(m_next_index++) + ">";
        } while (boost::hana::at_key(m_existing_nonterminals, boost::hana::type<D> {}).contains(candidate_name));

        return candidate_name;
    }

public:
    EliminateNestedConstructorsVisitor(Repositories& repositories,
                                       OptionalNonTerminals& start_symbols,
                                       DerivationRuleSets& derivation_rules,
                                       ToNonTerminalMap<std::string, Concept, Role, Boolean, Numerical>& existing_nonterminals) :
        CopyVisitor(repositories, start_symbols, derivation_rules),
        m_existing_nonterminals(existing_nonterminals),
        m_next_index(0),
        m_level(0)
    {
    }

    void visit(ConstructorOrNonTerminal<Concept> constructor) override { visit_impl(constructor); }
    void visit(ConstructorOrNonTerminal<Role> constructor) override { visit_impl(constructor); }
    void visit(ConstructorOrNonTerminal<Boolean> constructor) override { visit_impl(constructor); }
    void visit(ConstructorOrNonTerminal<Numerical> constructor) override { visit_impl(constructor); }

    template<FeatureCategory D>
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
                    static_assert(dependent_false<D>::value,
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
                           cnf_grammar::DerivationRulesContainer& derivation_rules,
                           cnf_grammar::SubstitutionRulesContainer& substitution_rules) :
    m_repositories(repositories),
    m_start_symbols(start_symbols),
    m_derivation_rules(derivation_rules),
    m_substitution_rules(substitution_rules)
{
}

void ToCNFVisitor::visit(ConceptBot constructor) { m_result = m_repositories.get_or_create_concept_bot(); }
void ToCNFVisitor::visit(ConceptTop constructor) { m_result = m_repositories.get_or_create_concept_top(); }
void ToCNFVisitor::visit(ConceptAtomicState<Static> constructor) { m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()); }
void ToCNFVisitor::visit(ConceptAtomicState<Fluent> constructor) { m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()); }
void ToCNFVisitor::visit(ConceptAtomicState<Derived> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void ToCNFVisitor::visit(ConceptAtomicGoal<Static> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void ToCNFVisitor::visit(ConceptAtomicGoal<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void ToCNFVisitor::visit(ConceptAtomicGoal<Derived> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void ToCNFVisitor::visit(ConceptIntersection constructor)
{
    constructor->get_left_concept_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    constructor->get_right_concept_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    m_result = m_repositories.get_or_create_concept_intersection(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(ConceptUnion constructor)
{
    constructor->get_left_concept_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    constructor->get_right_concept_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    m_result = m_repositories.get_or_create_concept_union(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(ConceptNegation constructor)
{
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto nonterminal = std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    m_result = m_repositories.get_or_create_concept_negation(nonterminal);
}
void ToCNFVisitor::visit(ConceptValueRestriction constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    m_result = m_repositories.get_or_create_concept_value_restriction(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(ConceptExistentialQuantification constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    m_result = m_repositories.get_or_create_concept_existential_quantification(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    m_result = m_repositories.get_or_create_concept_role_value_map_containment(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    m_result = m_repositories.get_or_create_concept_role_value_map_equality(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(ConceptNominal constructor) { m_result = m_repositories.get_or_create_concept_nominal(constructor->get_object()); }

void ToCNFVisitor::visit(RoleUniversal constructor) { m_result = m_repositories.get_or_create_role_universal(); }
void ToCNFVisitor::visit(RoleAtomicState<Static> constructor) { m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()); }
void ToCNFVisitor::visit(RoleAtomicState<Fluent> constructor) { m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()); }
void ToCNFVisitor::visit(RoleAtomicState<Derived> constructor) { m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()); }
void ToCNFVisitor::visit(RoleAtomicGoal<Static> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void ToCNFVisitor::visit(RoleAtomicGoal<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void ToCNFVisitor::visit(RoleAtomicGoal<Derived> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void ToCNFVisitor::visit(RoleIntersection constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    m_result = m_repositories.get_or_create_role_intersection(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(RoleUnion constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    m_result = m_repositories.get_or_create_role_union(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(RoleComplement constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    m_result = m_repositories.get_or_create_role_complement(nonterminal);
}
void ToCNFVisitor::visit(RoleInverse constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    m_result = m_repositories.get_or_create_role_inverse(nonterminal);
}
void ToCNFVisitor::visit(RoleComposition constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    m_result = m_repositories.get_or_create_role_composition(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(RoleTransitiveClosure constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    m_result = m_repositories.get_or_create_role_transitive_closure(nonterminal);
}
void ToCNFVisitor::visit(RoleReflexiveTransitiveClosure constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    m_result = m_repositories.get_or_create_role_reflexive_transitive_closure(nonterminal);
}
void ToCNFVisitor::visit(RoleRestriction constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto left_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto right_nonterminal = std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    m_result = m_repositories.get_or_create_role_restriction(left_nonterminal, right_nonterminal);
}
void ToCNFVisitor::visit(RoleIdentity constructor)
{
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto nonterminal = std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    m_result = m_repositories.get_or_create_role_identity(nonterminal);
}

void ToCNFVisitor::visit(BooleanAtomicState<Static> constructor) { m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate()); }
void ToCNFVisitor::visit(BooleanAtomicState<Fluent> constructor) { m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate()); }
void ToCNFVisitor::visit(BooleanAtomicState<Derived> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void ToCNFVisitor::visit(BooleanNonempty<Concept> constructor)
{
    constructor->get_constructor_or_nonterminal()->accept(*this);
    const auto concept_nonterminal = std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    m_result = m_repositories.get_or_create_boolean_nonempty(concept_nonterminal);
}
void ToCNFVisitor::visit(BooleanNonempty<Role> constructor)
{
    constructor->get_constructor_or_nonterminal()->accept(*this);
    const auto role_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    m_result = m_repositories.get_or_create_boolean_nonempty(role_nonterminal);
}

void ToCNFVisitor::visit(NumericalCount<Concept> constructor)
{
    constructor->get_constructor_or_nonterminal()->accept(*this);
    const auto concept_nonterminal = std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    m_result = m_repositories.get_or_create_numerical_count(concept_nonterminal);
}
void ToCNFVisitor::visit(NumericalCount<Role> constructor)
{
    constructor->get_constructor_or_nonterminal()->accept(*this);
    const auto role_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    m_result = m_repositories.get_or_create_numerical_count(role_nonterminal);
}
void ToCNFVisitor::visit(NumericalDistance constructor)
{
    constructor->get_left_concept_or_nonterminal()->accept(*this);
    const auto left_concept_nonterminal =
        std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    constructor->get_role_or_nonterminal()->accept(*this);
    const auto role_nonterminal = std::get<cnf_grammar::NonTerminal<Role>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Role>>(get_result()));
    constructor->get_right_concept_or_nonterminal()->accept(*this);
    const auto right_concept_nonterminal =
        std::get<cnf_grammar::NonTerminal<Concept>>(std::any_cast<cnf_grammar::ConstructorOrNonTerminal<Concept>>(get_result()));
    m_result = m_repositories.get_or_create_numerical_distance(left_concept_nonterminal, role_nonterminal, right_concept_nonterminal);
}

/**
 * NonTerminal
 */

void ToCNFVisitor::visit(NonTerminal<Concept> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(NonTerminal<Role> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(NonTerminal<Boolean> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(NonTerminal<Numerical> constructor) { visit_impl(constructor); }

template<FeatureCategory D>
void ToCNFVisitor::visit_impl(NonTerminal<D> constructor)
{
    m_result = m_repositories.template get_or_create_nonterminal<D>(constructor->get_name());
}

template void ToCNFVisitor::visit_impl(NonTerminal<Concept> constructor);
template void ToCNFVisitor::visit_impl(NonTerminal<Role> constructor);
template void ToCNFVisitor::visit_impl(NonTerminal<Boolean> constructor);
template void ToCNFVisitor::visit_impl(NonTerminal<Numerical> constructor);

/**
 * ConstructorOrNonTerminal
 */

void ToCNFVisitor::visit(ConstructorOrNonTerminal<Concept> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(ConstructorOrNonTerminal<Role> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(ConstructorOrNonTerminal<Boolean> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(ConstructorOrNonTerminal<Numerical> constructor) { visit_impl(constructor); }

template<FeatureCategory D>
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
                static_assert(dependent_false<D>::value,
                              "ConstructorOrNonTerminalVisitor<D>::visit(constructor): Missing implementation for ConstructorOrNonTerminal type.");
            }
        },
        constructor->get_constructor_or_non_terminal());
}

template void ToCNFVisitor::visit_impl(ConstructorOrNonTerminal<Concept> constructor);
template void ToCNFVisitor::visit_impl(ConstructorOrNonTerminal<Role> constructor);
template void ToCNFVisitor::visit_impl(ConstructorOrNonTerminal<Boolean> constructor);
template void ToCNFVisitor::visit_impl(ConstructorOrNonTerminal<Numerical> constructor);

/**
 * DerivationRule
 */

void ToCNFVisitor::visit(DerivationRule<Concept> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(DerivationRule<Role> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(DerivationRule<Boolean> constructor) { visit_impl(constructor); }

void ToCNFVisitor::visit(DerivationRule<Numerical> constructor) { visit_impl(constructor); }

template<FeatureCategory D>
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

template void ToCNFVisitor::visit_impl(DerivationRule<Concept> constructor);
template void ToCNFVisitor::visit_impl(DerivationRule<Role> constructor);
template void ToCNFVisitor::visit_impl(DerivationRule<Boolean> constructor);
template void ToCNFVisitor::visit_impl(DerivationRule<Numerical> constructor);

/**
 * Grammar
 */

void ToCNFVisitor::visit(const Grammar& grammar)
{
    boost::hana::for_each(grammar.get_start_symbols(),
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
        grammar.get_derivation_rules(),
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
                        if constexpr (std::is_same_v<T, cnf_grammar::DerivationRule<Concept>> || std::is_same_v<T, cnf_grammar::DerivationRule<Role>>
                                      || std::is_same_v<T, cnf_grammar::DerivationRule<Boolean>> || std::is_same_v<T, cnf_grammar::DerivationRule<Numerical>>)
                        {
                            m_derivation_rules.push_back(arg);
                        }
                        else if constexpr (std::is_same_v<T, cnf_grammar::SubstitutionRule<Concept>> || std::is_same_v<T, cnf_grammar::SubstitutionRule<Role>>
                                           || std::is_same_v<T, cnf_grammar::SubstitutionRule<Boolean>>
                                           || std::is_same_v<T, cnf_grammar::SubstitutionRule<Numerical>>)
                        {
                            m_substitution_rules.push_back(arg);
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
    auto derivation_rules = cnf_grammar::DerivationRulesContainer();
    auto substitution_rules = cnf_grammar::SubstitutionRulesContainer();

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
