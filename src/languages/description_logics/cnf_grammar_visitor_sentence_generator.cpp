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

#include "mimir/languages/description_logics/cnf_grammar_visitor_sentence_generator.hpp"

#include "mimir/languages/description_logics/cnf_grammar.hpp"
#include "mimir/languages/description_logics/cnf_grammar_constructors.hpp"
#include "mimir/languages/description_logics/cnf_grammar_sentence_pruning.hpp"
#include "mimir/languages/description_logics/constructor_visitors_formatter.hpp"

#include <unordered_map>
#include <vector>

namespace mimir::dl::cnf_grammar
{

/**
 * Concept
 */

GeneratorVisitor::GeneratorVisitor(RefinementPruningFunction& pruning_function,
                                   GeneratedSentencesContainer& sentences,
                                   dl::ConstructorRepositories& repositories,
                                   size_t max_syntactic_complexity) :
    m_pruning_function(pruning_function),
    m_sentences(sentences),
    m_repositories(repositories),
    m_max_syntactic_complexity(max_syntactic_complexity),
    m_complexity(1)
{
}

void GeneratorVisitor::visit(ConceptBot constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Concept> { m_repositories.get_or_create_concept_bot() };
}
void GeneratorVisitor::visit(ConceptTop constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Concept> { m_repositories.get_or_create_concept_top() };
}
void GeneratorVisitor::visit(ConceptAtomicState<Static> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Concept> { m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()) };
}
void GeneratorVisitor::visit(ConceptAtomicState<Fluent> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Concept> { m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()) };
}
void GeneratorVisitor::visit(ConceptAtomicState<Derived> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Concept> { m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()) };
}
void GeneratorVisitor::visit(ConceptAtomicGoal<Static> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Concept> { m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated()) };
}
void GeneratorVisitor::visit(ConceptAtomicGoal<Fluent> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Concept> { m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated()) };
}
void GeneratorVisitor::visit(ConceptAtomicGoal<Derived> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Concept> { m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated()) };
}
void GeneratorVisitor::visit(ConceptNominal constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Concept> { m_repositories.get_or_create_concept_nominal(constructor->get_object()) };
}
void GeneratorVisitor::visit(ConceptIntersection constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            if (i > j)
                continue;  ///< partially break symmetries

            auto result = dl::ConstructorList<Concept> {};

            for (const auto& left_concept : m_sentences.get(constructor->get_left_concept(), i))
            {
                for (const auto& right_concept : m_sentences.get(constructor->get_right_concept(), j))
                {
                    result.push_back(m_repositories.get_or_create_concept_intersection(left_concept, right_concept));
                }
            }

            m_result = result;
        }
    }
}
void GeneratorVisitor::visit(ConceptUnion constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            if (i > j)
                continue;  ///< partially break symmetries

            auto result = dl::ConstructorList<Concept> {};

            for (const auto& left_concept : m_sentences.get(constructor->get_left_concept(), i))
            {
                for (const auto& right_concept : m_sentences.get(constructor->get_right_concept(), j))
                {
                    result.push_back(m_repositories.get_or_create_concept_union(left_concept, right_concept));
                }
            }

            m_result = result;
        }
    }
}
void GeneratorVisitor::visit(ConceptNegation constructor)
{
    if (m_complexity >= 2)
    {
        auto result = dl::ConstructorList<Concept> {};

        for (const auto& concept_ : m_sentences.get(constructor->get_concept(), m_complexity - 1))
        {
            result.push_back(m_repositories.get_or_create_concept_negation(concept_));
        }

        m_result = result;
    }
}
void GeneratorVisitor::visit(ConceptValueRestriction constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            auto result = dl::ConstructorList<Concept> {};

            for (const auto& role : m_sentences.get(constructor->get_role(), i))
            {
                for (const auto& concept_ : m_sentences.get(constructor->get_concept(), j))
                {
                    result.push_back(m_repositories.get_or_create_concept_value_restriction(role, concept_));
                }
            }

            m_result = result;
        }
    }
}
void GeneratorVisitor::visit(ConceptExistentialQuantification constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            auto result = dl::ConstructorList<Concept> {};

            for (const auto& role : m_sentences.get(constructor->get_role(), i))
            {
                for (const auto& concept_ : m_sentences.get(constructor->get_concept(), j))
                {
                    result.push_back(m_repositories.get_or_create_concept_existential_quantification(role, concept_));
                }
            }

            m_result = result;
        }
    }
}
void GeneratorVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            auto result = dl::ConstructorList<Concept> {};

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    result.push_back(m_repositories.get_or_create_concept_role_value_map_containment(left_role, right_role));
                }
            }

            m_result = result;
        }
    }
}
void GeneratorVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            if (i > j)
                continue;  ///< partially break symmetries

            auto result = dl::ConstructorList<Concept> {};

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    result.push_back(m_repositories.get_or_create_concept_role_value_map_equality(left_role, right_role));
                }
            }

            m_result = result;
        }
    }
}

/**
 * Role
 */

void GeneratorVisitor::visit(RoleUniversal constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Role> { m_repositories.get_or_create_role_universal() };
}
void GeneratorVisitor::visit(RoleAtomicState<Static> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Role> { m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()) };
}
void GeneratorVisitor::visit(RoleAtomicState<Fluent> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Role> { m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()) };
}
void GeneratorVisitor::visit(RoleAtomicState<Derived> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Role> { m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()) };
}
void GeneratorVisitor::visit(RoleAtomicGoal<Static> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Role> { m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated()) };
}
void GeneratorVisitor::visit(RoleAtomicGoal<Fluent> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Role> { m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated()) };
}
void GeneratorVisitor::visit(RoleAtomicGoal<Derived> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Role> { m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated()) };
}
void GeneratorVisitor::visit(RoleIntersection constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            if (i > j)
                continue;  ///< partially break symmetries

            auto result = dl::ConstructorList<Role> {};

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    result.push_back(m_repositories.get_or_create_role_intersection(left_role, right_role));
                }
            }

            m_result = result;
        }
    }
}
void GeneratorVisitor::visit(RoleUnion constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            if (i > j)
                continue;  ///< partially break symmetries

            auto result = dl::ConstructorList<Role> {};

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    result.push_back(m_repositories.get_or_create_role_union(left_role, right_role));
                }
            }

            m_result = result;
        }
    }
}
void GeneratorVisitor::visit(RoleComplement constructor)
{
    if (m_complexity >= 2)
    {
        auto result = dl::ConstructorList<Role> {};

        for (const auto& role : m_sentences.get(constructor->get_role(), m_complexity - 1))
        {
            result.push_back(m_repositories.get_or_create_role_complement(role));
        }

        m_result = result;
    }
}
void GeneratorVisitor::visit(RoleInverse constructor)
{
    if (m_complexity >= 2)
    {
        auto result = dl::ConstructorList<Role> {};

        for (const auto& role : m_sentences.get(constructor->get_role(), m_complexity - 1))
        {
            result.push_back(m_repositories.get_or_create_role_inverse(role));
        }

        m_result = result;
    }
}
void GeneratorVisitor::visit(RoleComposition constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            auto result = dl::ConstructorList<Role> {};

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    result.push_back(m_repositories.get_or_create_role_composition(left_role, right_role));
                }
            }

            m_result = result;
        }
    }
}
void GeneratorVisitor::visit(RoleTransitiveClosure constructor)
{
    if (m_complexity >= 2)
    {
        auto result = dl::ConstructorList<Role> {};

        for (const auto& role : m_sentences.get(constructor->get_role(), m_complexity - 1))
        {
            result.push_back(m_repositories.get_or_create_role_transitive_closure(role));
        }

        m_result = result;
    }
}
void GeneratorVisitor::visit(RoleReflexiveTransitiveClosure constructor)
{
    if (m_complexity >= 2)
    {
        auto result = dl::ConstructorList<Role> {};

        for (const auto& role : m_sentences.get(constructor->get_role(), m_complexity - 1))
        {
            result.push_back(m_repositories.get_or_create_role_reflexive_transitive_closure(role));
        }

        m_result = result;
    }
}
void GeneratorVisitor::visit(RoleRestriction constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            auto result = dl::ConstructorList<Role> {};

            for (const auto& role : m_sentences.get(constructor->get_role(), i))
            {
                for (const auto& concept_ : m_sentences.get(constructor->get_concept(), j))
                {
                    result.push_back(m_repositories.get_or_create_role_restriction(role, concept_));
                }
            }

            m_result = result;
        }
    }
}
void GeneratorVisitor::visit(RoleIdentity constructor)
{
    if (m_complexity >= 2)
    {
        auto result = dl::ConstructorList<Role> {};

        for (const auto& concept_ : m_sentences.get(constructor->get_concept(), m_complexity - 1))
        {
            result.push_back(m_repositories.get_or_create_role_identity(concept_));
        }

        m_result = result;
    }
}

/**
 * Booleans
 */

void GeneratorVisitor::visit(BooleanAtomicState<Static> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Boolean> { m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate()) };
}
void GeneratorVisitor::visit(BooleanAtomicState<Fluent> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Boolean> { m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate()) };
}
void GeneratorVisitor::visit(BooleanAtomicState<Derived> constructor)
{
    if (m_complexity == 1)
        m_result = dl::ConstructorList<Boolean> { m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate()) };
}
void GeneratorVisitor::visit(BooleanNonempty<Concept> constructor)
{
    if (m_complexity >= 2)
    {
        auto result = dl::ConstructorList<Boolean> {};

        for (const auto& concept_ : m_sentences.get(constructor->get_nonterminal(), m_complexity - 1))
        {
            result.push_back(m_repositories.get_or_create_boolean_nonempty(concept_));
        }

        m_result = result;
    }
}
void GeneratorVisitor::visit(BooleanNonempty<Role> constructor)
{
    if (m_complexity >= 2)
    {
        auto result = dl::ConstructorList<Boolean> {};

        for (const auto& role : m_sentences.get(constructor->get_nonterminal(), m_complexity - 1))
        {
            result.push_back(m_repositories.get_or_create_boolean_nonempty(role));
        }

        m_result = result;
    }
}

/**
 * Numericals
 */

void GeneratorVisitor::visit(NumericalCount<Concept> constructor)
{
    if (m_complexity >= 2)
    {
        auto result = dl::ConstructorList<Numerical> {};

        for (const auto& concept_ : m_sentences.get(constructor->get_nonterminal(), m_complexity - 1))
        {
            result.push_back(m_repositories.get_or_create_numerical_count(concept_));
        }

        m_result = result;
    }
}

void GeneratorVisitor::visit(NumericalCount<Role> constructor)
{
    if (m_complexity >= 2)
    {
        auto result = dl::ConstructorList<Numerical> {};

        for (const auto& role : m_sentences.get(constructor->get_nonterminal(), m_complexity - 1))
        {
            result.push_back(m_repositories.get_or_create_numerical_count(role));
        }

        m_result = result;
    }
}

void GeneratorVisitor::visit(NumericalDistance constructor)
{
    if (m_complexity >= 3)
    {
        auto result = dl::ConstructorList<Numerical> {};

        for (size_t i = 1; i < m_complexity - 2; ++i)
        {
            for (size_t j = 1; j < m_complexity - i - 1; ++j)
            {
                size_t k = m_complexity - i - j - 1;

                for (const auto& concept_left : m_sentences.get(constructor->get_left_concept(), i))
                {
                    for (const auto& role : m_sentences.get(constructor->get_role(), i))
                    {
                        for (const auto& concept_right : m_sentences.get(constructor->get_right_concept(), k))
                        {
                            result.push_back(m_repositories.get_or_create_numerical_distance(concept_left, role, concept_right));
                        }
                    }
                }
            }
        }

        m_result = result;
    }
}

/**
 * DerivationRule
 */

void GeneratorVisitor::visit(DerivationRule<Concept> rule) { visit_impl(rule); }

void GeneratorVisitor::visit(DerivationRule<Role> rule) { visit_impl(rule); }

void GeneratorVisitor::visit(DerivationRule<Boolean> rule) { visit_impl(rule); }

void GeneratorVisitor::visit(DerivationRule<Numerical> rule) { visit_impl(rule); }

template<FeatureCategory D>
void GeneratorVisitor::visit_impl(DerivationRule<D> rule)
{
    rule->get_body()->accept(*this);

    auto& statistics = boost::hana::at_key(m_statistics, boost::hana::type<D> {});

    const auto& generated = std::any_cast<const dl::ConstructorList<D>&>(get_result());

    for (const auto& sentence : generated)
    {
        ++statistics.num_generated;

        if (!m_pruning_function.should_prune(sentence))
        {
            ++statistics.num_kept;

            auto& target_location = m_sentences.get(rule->get_head(), m_complexity);
            target_location.push_back(sentence);
        }
        else
        {
            ++statistics.num_pruned;
        }
    }
}

template void GeneratorVisitor::visit_impl(DerivationRule<Concept> rule);
template void GeneratorVisitor::visit_impl(DerivationRule<Role> rule);
template void GeneratorVisitor::visit_impl(DerivationRule<Boolean> rule);
template void GeneratorVisitor::visit_impl(DerivationRule<Numerical> rule);

/**
 * SubstitutionRule
 */

void GeneratorVisitor::visit(SubstitutionRule<Concept> rule) { visit_impl(rule); }

void GeneratorVisitor::visit(SubstitutionRule<Role> rule) { visit_impl(rule); }

void GeneratorVisitor::visit(SubstitutionRule<Boolean> rule) { visit_impl(rule); }

void GeneratorVisitor::visit(SubstitutionRule<Numerical> rule) { visit_impl(rule); }

template<FeatureCategory D>
void GeneratorVisitor::visit_impl(SubstitutionRule<D> rule)
{
    auto& source_location = m_sentences.get(rule->get_body(), m_complexity);
    auto& target_location = m_sentences.get(rule->get_head(), m_complexity);
    target_location.insert(target_location.end(), source_location.begin(), source_location.end());
}

template void GeneratorVisitor::visit_impl(SubstitutionRule<Concept> rule);
template void GeneratorVisitor::visit_impl(SubstitutionRule<Role> rule);
template void GeneratorVisitor::visit_impl(SubstitutionRule<Boolean> rule);
template void GeneratorVisitor::visit_impl(SubstitutionRule<Numerical> rule);

/**
 * Grammar
 */

void GeneratorVisitor::visit(const Grammar& grammar)
{
    /* Generate */
    for (size_t i = 1; i <= m_max_syntactic_complexity; ++i)
    {
        boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);

                                  for (const auto& rule : second)
                                  {
                                      rule->accept(*this);
                                  }
                              });

        boost::hana::for_each(grammar.get_substitution_rules_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);

                                  for (const auto& rule : second)
                                  {
                                      rule->accept(*this);
                                  }
                              });
    }
}

const HanaGeneratorStatistics<Concept, Role, Boolean, Numerical>& GeneratorVisitor::get_statistics() const { return m_statistics; }

const std::any& GeneratorVisitor::get_result() const { return m_result; }
}
