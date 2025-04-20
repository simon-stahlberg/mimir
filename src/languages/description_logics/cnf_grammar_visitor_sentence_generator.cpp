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
#include "mimir/languages/description_logics/constructor_visitor_formatter.hpp"

#include <unordered_map>
#include <vector>

using namespace mimir::formalism;

namespace mimir::languages::dl::cnf_grammar
{

/**
 * GeneratedSentencesContainer::
 */

const dl::ConstructorLists GeneratedSentencesContainer::empty_lists = dl::ConstructorLists();

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
const GeneratedSentencesContainer::GeneratedConstructorsMap<D>& GeneratedSentencesContainer::get() const
{
    return boost::hana::at_key(m_generated_constructors, boost::hana::type<D> {});
}

template const GeneratedSentencesContainer::GeneratedConstructorsMap<ConceptTag>& GeneratedSentencesContainer::get() const;
template const GeneratedSentencesContainer::GeneratedConstructorsMap<RoleTag>& GeneratedSentencesContainer::get() const;
template const GeneratedSentencesContainer::GeneratedConstructorsMap<BooleanTag>& GeneratedSentencesContainer::get() const;
template const GeneratedSentencesContainer::GeneratedConstructorsMap<NumericalTag>& GeneratedSentencesContainer::get() const;

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
dl::ConstructorList<D>& GeneratedSentencesContainer::get(NonTerminal<D> nonterminal, size_t complexity)
{
    auto& constructors_by_complexity = boost::hana::at_key(m_generated_constructors, boost::hana::type<D> {})[nonterminal];
    if (complexity >= constructors_by_complexity.size())
    {
        constructors_by_complexity.resize(complexity + 1);
    }
    return constructors_by_complexity[complexity];
}

template dl::ConstructorList<ConceptTag>& GeneratedSentencesContainer::get(NonTerminal<ConceptTag> nonterminal, size_t complexity);
template dl::ConstructorList<RoleTag>& GeneratedSentencesContainer::get(NonTerminal<RoleTag> nonterminal, size_t complexity);
template dl::ConstructorList<BooleanTag>& GeneratedSentencesContainer::get(NonTerminal<BooleanTag> nonterminal, size_t complexity);
template dl::ConstructorList<NumericalTag>& GeneratedSentencesContainer::get(NonTerminal<NumericalTag> nonterminal, size_t complexity);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
const dl::ConstructorList<D>& GeneratedSentencesContainer::get(NonTerminal<D> nonterminal, size_t complexity) const
{
    const auto& container = boost::hana::at_key(m_generated_constructors, boost::hana::type<D> {});

    auto it = container.find(nonterminal);
    if (it == container.end() || complexity >= it->second.size())
    {
        return boost::hana::at_key(empty_lists, boost::hana::type<D> {});
    }

    return it->second.at(complexity);
}

template const dl::ConstructorList<ConceptTag>& GeneratedSentencesContainer::get(NonTerminal<ConceptTag> nonterminal, size_t complexity) const;
template const dl::ConstructorList<RoleTag>& GeneratedSentencesContainer::get(NonTerminal<RoleTag> nonterminal, size_t complexity) const;
template const dl::ConstructorList<BooleanTag>& GeneratedSentencesContainer::get(NonTerminal<BooleanTag> nonterminal, size_t complexity) const;
template const dl::ConstructorList<NumericalTag>& GeneratedSentencesContainer::get(NonTerminal<NumericalTag> nonterminal, size_t complexity) const;

GeneratedSentencesContainer::HanaGeneratedConstructorsMaps& GeneratedSentencesContainer::get_hana_generated_constructors() { return m_generated_constructors; }

/**
 * Concept
 */

GeneratorVisitor::GeneratorVisitor(IRefinementPruningFunction& pruning_function,
                                   GeneratedSentencesContainer& sentences,
                                   dl::Repositories& repositories,
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
        boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {}).push_back(m_repositories.get_or_create_concept_bot());
}
void GeneratorVisitor::visit(ConceptTop constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {}).push_back(m_repositories.get_or_create_concept_top());
}
void GeneratorVisitor::visit(ConceptAtomicState<StaticTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {})
            .push_back(m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()));
}
void GeneratorVisitor::visit(ConceptAtomicState<FluentTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {})
            .push_back(m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()));
}
void GeneratorVisitor::visit(ConceptAtomicState<DerivedTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {})
            .push_back(m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()));
}
void GeneratorVisitor::visit(ConceptAtomicGoal<StaticTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {})
            .push_back(m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->get_polarity()));
}
void GeneratorVisitor::visit(ConceptAtomicGoal<FluentTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {})
            .push_back(m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->get_polarity()));
}
void GeneratorVisitor::visit(ConceptAtomicGoal<DerivedTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {})
            .push_back(m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->get_polarity()));
}
void GeneratorVisitor::visit(ConceptNominal constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {}).push_back(m_repositories.get_or_create_concept_nominal(constructor->get_object()));
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

            auto& generated = boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {});

            for (const auto& left_concept : m_sentences.get(constructor->get_left_concept(), i))
            {
                for (const auto& right_concept : m_sentences.get(constructor->get_right_concept(), j))
                {
                    generated.push_back(m_repositories.get_or_create_concept_intersection(left_concept, right_concept));
                }
            }
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

            auto& generated = boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {});

            for (const auto& left_concept : m_sentences.get(constructor->get_left_concept(), i))
            {
                for (const auto& right_concept : m_sentences.get(constructor->get_right_concept(), j))
                {
                    generated.push_back(m_repositories.get_or_create_concept_union(left_concept, right_concept));
                }
            }
        }
    }
}
void GeneratorVisitor::visit(ConceptNegation constructor)
{
    if (m_complexity >= 2)
    {
        auto& generated = boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {});

        for (const auto& concept_ : m_sentences.get(constructor->get_concept(), m_complexity - 1))
        {
            generated.push_back(m_repositories.get_or_create_concept_negation(concept_));
        }
    }
}
void GeneratorVisitor::visit(ConceptValueRestriction constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            auto& generated = boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {});

            for (const auto& role : m_sentences.get(constructor->get_role(), i))
            {
                for (const auto& concept_ : m_sentences.get(constructor->get_concept(), j))
                {
                    generated.push_back(m_repositories.get_or_create_concept_value_restriction(role, concept_));
                }
            }
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

            auto& generated = boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {});

            for (const auto& role : m_sentences.get(constructor->get_role(), i))
            {
                for (const auto& concept_ : m_sentences.get(constructor->get_concept(), j))
                {
                    generated.push_back(m_repositories.get_or_create_concept_existential_quantification(role, concept_));
                }
            }
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

            auto& generated = boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {});

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    generated.push_back(m_repositories.get_or_create_concept_role_value_map_containment(left_role, right_role));
                }
            }
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

            auto& generated = boost::hana::at_key(m_generated, boost::hana::type<ConceptTag> {});

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    generated.push_back(m_repositories.get_or_create_concept_role_value_map_equality(left_role, right_role));
                }
            }
        }
    }
}

/**
 * Role
 */

void GeneratorVisitor::visit(RoleUniversal constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {}).push_back(m_repositories.get_or_create_role_universal());
}
void GeneratorVisitor::visit(RoleAtomicState<StaticTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {}).push_back(m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()));
}
void GeneratorVisitor::visit(RoleAtomicState<FluentTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {}).push_back(m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()));
}
void GeneratorVisitor::visit(RoleAtomicState<DerivedTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {}).push_back(m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()));
}
void GeneratorVisitor::visit(RoleAtomicGoal<StaticTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {})
            .push_back(m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->get_polarity()));
}
void GeneratorVisitor::visit(RoleAtomicGoal<FluentTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {})
            .push_back(m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->get_polarity()));
}
void GeneratorVisitor::visit(RoleAtomicGoal<DerivedTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {})
            .push_back(m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->get_polarity()));
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

            auto& generated = boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {});

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    generated.push_back(m_repositories.get_or_create_role_intersection(left_role, right_role));
                }
            }
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

            auto& generated = boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {});

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    generated.push_back(m_repositories.get_or_create_role_union(left_role, right_role));
                }
            }
        }
    }
}
void GeneratorVisitor::visit(RoleComplement constructor)
{
    if (m_complexity >= 2)
    {
        auto& generated = boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {});

        for (const auto& role : m_sentences.get(constructor->get_role(), m_complexity - 1))
        {
            generated.push_back(m_repositories.get_or_create_role_complement(role));
        }
    }
}
void GeneratorVisitor::visit(RoleInverse constructor)
{
    if (m_complexity >= 2)
    {
        auto& generated = boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {});

        for (const auto& role : m_sentences.get(constructor->get_role(), m_complexity - 1))
        {
            generated.push_back(m_repositories.get_or_create_role_inverse(role));
        }
    }
}
void GeneratorVisitor::visit(RoleComposition constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            auto& generated = boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {});

            for (const auto& left_role : m_sentences.get(constructor->get_left_role(), i))
            {
                for (const auto& right_role : m_sentences.get(constructor->get_right_role(), j))
                {
                    generated.push_back(m_repositories.get_or_create_role_composition(left_role, right_role));
                }
            }
        }
    }
}
void GeneratorVisitor::visit(RoleTransitiveClosure constructor)
{
    if (m_complexity >= 2)
    {
        auto& generated = boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {});

        for (const auto& role : m_sentences.get(constructor->get_role(), m_complexity - 1))
        {
            generated.push_back(m_repositories.get_or_create_role_transitive_closure(role));
        }
    }
}
void GeneratorVisitor::visit(RoleReflexiveTransitiveClosure constructor)
{
    if (m_complexity >= 2)
    {
        auto& generated = boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {});

        for (const auto& role : m_sentences.get(constructor->get_role(), m_complexity - 1))
        {
            generated.push_back(m_repositories.get_or_create_role_reflexive_transitive_closure(role));
        }
    }
}
void GeneratorVisitor::visit(RoleRestriction constructor)
{
    if (m_complexity >= 3)
    {
        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            size_t j = m_complexity - i - 1;

            auto& generated = boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {});

            for (const auto& role : m_sentences.get(constructor->get_role(), i))
            {
                for (const auto& concept_ : m_sentences.get(constructor->get_concept(), j))
                {
                    generated.push_back(m_repositories.get_or_create_role_restriction(role, concept_));
                }
            }
        }
    }
}
void GeneratorVisitor::visit(RoleIdentity constructor)
{
    if (m_complexity >= 2)
    {
        auto& generated = boost::hana::at_key(m_generated, boost::hana::type<RoleTag> {});

        for (const auto& concept_ : m_sentences.get(constructor->get_concept(), m_complexity - 1))
        {
            generated.push_back(m_repositories.get_or_create_role_identity(concept_));
        }
    }
}

/**
 * Booleans
 */

void GeneratorVisitor::visit(BooleanAtomicState<StaticTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<BooleanTag> {})
            .push_back(m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate()));
}
void GeneratorVisitor::visit(BooleanAtomicState<FluentTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<BooleanTag> {})
            .push_back(m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate()));
}
void GeneratorVisitor::visit(BooleanAtomicState<DerivedTag> constructor)
{
    if (m_complexity == 1)
        boost::hana::at_key(m_generated, boost::hana::type<BooleanTag> {})
            .push_back(m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate()));
}
void GeneratorVisitor::visit(BooleanNonempty<ConceptTag> constructor)
{
    if (m_complexity >= 2)
    {
        auto& generated = boost::hana::at_key(m_generated, boost::hana::type<BooleanTag> {});

        for (const auto& concept_ : m_sentences.get(constructor->get_nonterminal(), m_complexity - 1))
        {
            generated.push_back(m_repositories.get_or_create_boolean_nonempty(concept_));
        }
    }
}
void GeneratorVisitor::visit(BooleanNonempty<RoleTag> constructor)
{
    if (m_complexity >= 2)
    {
        auto& generated = boost::hana::at_key(m_generated, boost::hana::type<BooleanTag> {});

        for (const auto& role : m_sentences.get(constructor->get_nonterminal(), m_complexity - 1))
        {
            generated.push_back(m_repositories.get_or_create_boolean_nonempty(role));
        }
    }
}

/**
 * Numericals
 */

void GeneratorVisitor::visit(NumericalCount<ConceptTag> constructor)
{
    if (m_complexity >= 2)
    {
        auto& generated = boost::hana::at_key(m_generated, boost::hana::type<NumericalTag> {});

        for (const auto& concept_ : m_sentences.get(constructor->get_nonterminal(), m_complexity - 1))
        {
            generated.push_back(m_repositories.get_or_create_numerical_count(concept_));
        }
    }
}

void GeneratorVisitor::visit(NumericalCount<RoleTag> constructor)
{
    if (m_complexity >= 2)
    {
        auto& generated = boost::hana::at_key(m_generated, boost::hana::type<NumericalTag> {});

        for (const auto& role : m_sentences.get(constructor->get_nonterminal(), m_complexity - 1))
        {
            generated.push_back(m_repositories.get_or_create_numerical_count(role));
        }
    }
}

void GeneratorVisitor::visit(NumericalDistance constructor)
{
    if (m_complexity >= 3)
    {
        auto& generated = boost::hana::at_key(m_generated, boost::hana::type<NumericalTag> {});

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
                            generated.push_back(m_repositories.get_or_create_numerical_distance(concept_left, role, concept_right));
                        }
                    }
                }
            }
        }
    }
}

/**
 * DerivationRule
 */

void GeneratorVisitor::visit(DerivationRule<ConceptTag> rule) { visit_impl(rule); }

void GeneratorVisitor::visit(DerivationRule<RoleTag> rule) { visit_impl(rule); }

void GeneratorVisitor::visit(DerivationRule<BooleanTag> rule) { visit_impl(rule); }

void GeneratorVisitor::visit(DerivationRule<NumericalTag> rule) { visit_impl(rule); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void GeneratorVisitor::visit_impl(DerivationRule<D> rule)
{
    auto& generated = boost::hana::at_key(m_generated, boost::hana::type<D> {});
    generated.clear();

    rule->get_body()->accept(*this);

    auto& statistics = boost::hana::at_key(m_statistics, boost::hana::type<D> {});

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

template void GeneratorVisitor::visit_impl(DerivationRule<ConceptTag> rule);
template void GeneratorVisitor::visit_impl(DerivationRule<RoleTag> rule);
template void GeneratorVisitor::visit_impl(DerivationRule<BooleanTag> rule);
template void GeneratorVisitor::visit_impl(DerivationRule<NumericalTag> rule);

/**
 * SubstitutionRule
 */

void GeneratorVisitor::visit(SubstitutionRule<ConceptTag> rule) { visit_impl(rule); }

void GeneratorVisitor::visit(SubstitutionRule<RoleTag> rule) { visit_impl(rule); }

void GeneratorVisitor::visit(SubstitutionRule<BooleanTag> rule) { visit_impl(rule); }

void GeneratorVisitor::visit(SubstitutionRule<NumericalTag> rule) { visit_impl(rule); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void GeneratorVisitor::visit_impl(SubstitutionRule<D> rule)
{
    auto& source_location = m_sentences.get(rule->get_body(), m_complexity);
    auto& target_location = m_sentences.get(rule->get_head(), m_complexity);
    target_location.insert(target_location.end(), source_location.begin(), source_location.end());
}

template void GeneratorVisitor::visit_impl(SubstitutionRule<ConceptTag> rule);
template void GeneratorVisitor::visit_impl(SubstitutionRule<RoleTag> rule);
template void GeneratorVisitor::visit_impl(SubstitutionRule<BooleanTag> rule);
template void GeneratorVisitor::visit_impl(SubstitutionRule<NumericalTag> rule);

/**
 * Grammar
 */

void GeneratorVisitor::visit(const Grammar& grammar)
{
    /* Generate */
    for (; m_complexity <= m_max_syntactic_complexity; ++m_complexity)
    {
        boost::hana::for_each(grammar.get_hana_derivation_rules(),
                              [&](auto&& pair)
                              {
                                  const auto& second = boost::hana::second(pair);

                                  for (const auto& rule : second)
                                  {
                                      rule->accept(*this);
                                  }
                              });

        boost::hana::for_each(grammar.get_hana_substitution_rules(),
                              [&](auto&& pair)
                              {
                                  const auto& second = boost::hana::second(pair);

                                  for (const auto& rule : second)
                                  {
                                      rule->accept(*this);
                                  }
                              });
    }
}

const HanaGeneratorStatistics& GeneratorVisitor::get_statistics() const { return m_statistics; }

const std::any& GeneratorVisitor::get_result() const { return m_result; }
}
