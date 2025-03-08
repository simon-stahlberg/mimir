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

const ConstructorLists<Concept, Role> GeneratedSentencesContainer::empty_lists =
    boost::hana::make_map(boost::hana::make_pair(boost::hana::type_c<Concept>, dl::ConstructorList<Concept> {}),
                          boost::hana::make_pair(boost::hana::type_c<Role>, dl::ConstructorList<Role> {}));

/**
 * Concept
 */

GeneratorConstructorVisitor<Concept>::GeneratorConstructorVisitor(const GeneratedSentencesContainer& sentences,
                                                                  dl::ConstructorRepositories& repositories,
                                                                  size_t complexity) :
    m_sentences(sentences),
    m_repositories(repositories),
    m_complexity(complexity)
{
}

void GeneratorConstructorVisitor<Concept>::visit(ConceptBot constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_concept_bot());
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptTop constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_concept_top());
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptAtomicState<Static> constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()));
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptAtomicState<Fluent> constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()));
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptAtomicState<Derived> constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()));
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Static> constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated()));
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Fluent> constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated()));
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Derived> constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated()));
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptNominal constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_concept_nominal(constructor->get_object()));
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptIntersection constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            if (i > j)
                continue;  ///< partially break symmetries

            for (const auto& left_concept : m_sentences.get(constructor->get_left_concept(), i))
            {
                for (const auto& right_concept : m_sentences.get(constructor->get_right_concept(), j))
                {
                    m_result.push_back(m_repositories.get_or_create_concept_intersection(left_concept, right_concept));
                }
            }
        }
    }
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            if (i > j)
                continue;  ///< partially break symmetries

            for (const auto& left_concept : m_sentences.get(constructor->get_left_concept(), i))
            {
                for (const auto& right_concept : m_sentences.get(constructor->get_right_concept(), j))
                {
                    m_result.push_back(m_repositories.get_or_create_concept_union(left_concept, right_concept));
                }
            }
        }
    }
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptNegation constructor)
{
    if (m_complexity >= 2)
    {
        for (const auto& concept_ : m_sentences.get(constructor->get_concept(), m_complexity - 1))
        {
            m_result.push_back(m_repositories.get_or_create_concept_negation(concept_));
        }
    }
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            for (const auto& role : m_sentences.get(constructor->get_role(), i))
            {
                for (const auto& concept_ : m_sentences.get(constructor->get_concept(), j))
                {
                    m_result.push_back(m_repositories.get_or_create_concept_value_restriction(role, concept_));
                }
            }
        }
    }
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            for (const auto& role : m_sentences.get(constructor->get_role(), i))
            {
                for (const auto& concept_ : m_sentences.get(constructor->get_concept(), j))
                {
                    m_result.push_back(m_repositories.get_or_create_concept_existential_quantification(role, concept_));
                }
            }
        }
    }
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    m_result.push_back(m_repositories.get_or_create_concept_role_value_map_containment(left_role, right_role));
                }
            }
        }
    }
}
void GeneratorConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            if (i > j)
                continue;  ///< partially break symmetries

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    m_result.push_back(m_repositories.get_or_create_concept_role_value_map_equality(left_role, right_role));
                }
            }
        }
    }
}

const dl::ConstructorList<Concept>& GeneratorConstructorVisitor<Concept>::get_result() const { return m_result; }

/**
 * Role
 */

GeneratorConstructorVisitor<Role>::GeneratorConstructorVisitor(const GeneratedSentencesContainer& sentences,
                                                               dl::ConstructorRepositories& repositories,
                                                               size_t complexity) :
    m_sentences(sentences),
    m_repositories(repositories),
    m_complexity(complexity)
{
}

void GeneratorConstructorVisitor<Role>::visit(RoleUniversal constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_role_universal());
}
void GeneratorConstructorVisitor<Role>::visit(RoleAtomicState<Static> constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()));
}
void GeneratorConstructorVisitor<Role>::visit(RoleAtomicState<Fluent> constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()));
}
void GeneratorConstructorVisitor<Role>::visit(RoleAtomicState<Derived> constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()));
}
void GeneratorConstructorVisitor<Role>::visit(RoleAtomicGoal<Static> constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated()));
}
void GeneratorConstructorVisitor<Role>::visit(RoleAtomicGoal<Fluent> constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated()));
}
void GeneratorConstructorVisitor<Role>::visit(RoleAtomicGoal<Derived> constructor)
{
    if (m_complexity == 1)
        m_result.push_back(m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated()));
}
void GeneratorConstructorVisitor<Role>::visit(RoleIntersection constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            if (i > j)
                continue;  ///< partially break symmetries

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    m_result.push_back(m_repositories.get_or_create_role_intersection(left_role, right_role));
                }
            }
        }
    }
}
void GeneratorConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            if (i > j)
                continue;  ///< partially break symmetries

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    m_result.push_back(m_repositories.get_or_create_role_union(left_role, right_role));
                }
            }
        }
    }
}
void GeneratorConstructorVisitor<Role>::visit(RoleComplement constructor)
{
    if (m_complexity >= 2)
    {
        for (const auto& role : m_sentences.get(constructor->get_role(), m_complexity - 1))
        {
            m_result.push_back(m_repositories.get_or_create_role_complement(role));
        }
    }
}
void GeneratorConstructorVisitor<Role>::visit(RoleInverse constructor)
{
    if (m_complexity >= 2)
    {
        for (const auto& role : m_sentences.get(constructor->get_role(), m_complexity - 1))
        {
            m_result.push_back(m_repositories.get_or_create_role_inverse(role));
        }
    }
}
void GeneratorConstructorVisitor<Role>::visit(RoleComposition constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    m_result.push_back(m_repositories.get_or_create_role_composition(left_role, right_role));
                }
            }
        }
    }
}
void GeneratorConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor)
{
    if (m_complexity >= 2)
    {
        for (const auto& role : m_sentences.get(constructor->get_role(), m_complexity - 1))
        {
            m_result.push_back(m_repositories.get_or_create_role_transitive_closure(role));
        }
    }
}
void GeneratorConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor)
{
    if (m_complexity >= 2)
    {
        for (const auto& role : m_sentences.get(constructor->get_role(), m_complexity - 1))
        {
            m_result.push_back(m_repositories.get_or_create_role_reflexive_transitive_closure(role));
        }
    }
}
void GeneratorConstructorVisitor<Role>::visit(RoleRestriction constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            for (const auto& role : m_sentences.get(constructor->get_role(), i))
            {
                for (const auto& concept_ : m_sentences.get(constructor->get_concept(), j))
                {
                    m_result.push_back(m_repositories.get_or_create_role_restriction(role, concept_));
                }
            }
        }
    }
}
void GeneratorConstructorVisitor<Role>::visit(RoleIdentity constructor)
{
    if (m_complexity >= 2)
    {
        for (const auto& concept_ : m_sentences.get(constructor->get_concept(), m_complexity - 1))
        {
            m_result.push_back(m_repositories.get_or_create_role_identity(concept_));
        }
    }
}

const dl::ConstructorList<Role>& GeneratorConstructorVisitor<Role>::get_result() const { return m_result; }

/**
 * DerivationRule
 */

template<FeatureCategory D>
GeneratorDerivationRuleVisitor<D>::GeneratorDerivationRuleVisitor(RefinementPruningFunction& pruning_function,
                                                                  GeneratedSentencesContainer& sentences,
                                                                  dl::ConstructorRepositories& repositories,
                                                                  size_t complexity) :
    m_pruning_function(pruning_function),
    m_sentences(sentences),
    m_repositories(repositories),
    m_complexity(complexity),
    m_statistics()
{
}

template<FeatureCategory D>
void GeneratorDerivationRuleVisitor<D>::visit(DerivationRule<D> rule)
{
    auto visitor = GeneratorConstructorVisitor<D>(m_sentences, m_repositories, m_complexity);
    rule->get_body()->accept(visitor);
    const auto& generated = visitor.get_result();
    for (const auto& sentence : generated)
    {
        ++m_statistics.num_generated;

        if (!m_pruning_function.should_prune(sentence))
        {
            ++m_statistics.num_kept;

            auto& target_location = m_sentences.get(rule->get_head(), m_complexity);
            target_location.push_back(sentence);
        }
        else
        {
            ++m_statistics.num_pruned;
        }
    }
}

template<FeatureCategory D>
const GeneratorStatistics<D>& GeneratorDerivationRuleVisitor<D>::get_statistics() const
{
    return m_statistics;
}

template class GeneratorDerivationRuleVisitor<Concept>;
template class GeneratorDerivationRuleVisitor<Role>;

/**
 * SubstitutionRule
 */

template<FeatureCategory D>
GeneratorSubstitutionRuleVisitor<D>::GeneratorSubstitutionRuleVisitor(GeneratedSentencesContainer& sentences,
                                                                      dl::ConstructorRepositories& repositories,
                                                                      size_t complexity) :
    m_sentences(sentences),
    m_repositories(repositories),
    m_complexity(complexity)
{
}

template<FeatureCategory D>
void GeneratorSubstitutionRuleVisitor<D>::visit(SubstitutionRule<D> rule)
{
    auto& source_location = m_sentences.get(rule->get_body(), m_complexity);
    auto& target_location = m_sentences.get(rule->get_head(), m_complexity);
    target_location.insert(target_location.end(), source_location.begin(), source_location.end());
}

template class GeneratorSubstitutionRuleVisitor<Concept>;
template class GeneratorSubstitutionRuleVisitor<Role>;

/**
 * Grammar
 */

GeneratorGrammarVisitor::GeneratorGrammarVisitor(RefinementPruningFunction& pruning_function,
                                                 GeneratedSentencesContainer& sentences,
                                                 dl::ConstructorRepositories& repositories,
                                                 size_t max_syntactic_complexity) :
    m_pruning_function(pruning_function),
    m_sentences(sentences),
    m_repositories(repositories),
    m_max_syntactic_complexity(max_syntactic_complexity),
    m_statistics()
{
}

void GeneratorGrammarVisitor::visit(const Grammar& grammar)
{
    /* Generate */
    for (size_t i = 1; i < m_max_syntactic_complexity; ++i)
    {
        boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using ConstructorType = typename decltype(+key)::type;

                                  for (const auto& rule : second)
                                  {
                                      auto derivation_rule_visitor =
                                          GeneratorDerivationRuleVisitor<ConstructorType>(m_pruning_function, m_sentences, m_repositories, i);
                                      rule->accept(derivation_rule_visitor);

                                      boost::hana::at_key(m_statistics, key) += derivation_rule_visitor.get_statistics();
                                  }
                              });

        boost::hana::for_each(grammar.get_substitution_rules().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using ConstructorType = typename decltype(+key)::type;

                                  for (const auto& rule : second)
                                  {
                                      auto substitution_rule_visitor = GeneratorSubstitutionRuleVisitor<ConstructorType>(m_sentences, m_repositories, i);
                                      rule->accept(substitution_rule_visitor);
                                  }
                              });
    }
}

const HanaGeneratorStatistics<Concept, Role>& GeneratorGrammarVisitor::get_statistics() const { return m_statistics; }

}
