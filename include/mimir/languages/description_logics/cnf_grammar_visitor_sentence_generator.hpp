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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_VISITOR_SENTENCE_GENERATOR_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_VISITOR_SENTENCE_GENERATOR_HPP_

#include "mimir/languages/description_logics/cnf_grammar_sentence_pruning.hpp"
#include "mimir/languages/description_logics/cnf_grammar_visitor_interface.hpp"
#include "mimir/languages/description_logics/constructor_repositories.hpp"

#include <any>
#include <unordered_map>
#include <vector>

namespace mimir::languages::dl::cnf_grammar
{

////////////////////////////
/// Interface
////////////////////////////

class GeneratedSentencesContainer
{
private:
    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    using GeneratedConstructorsMap = std::unordered_map<NonTerminal<D>, std::vector<dl::ConstructorList<D>>>;

    using HanaGeneratedConstructorsMaps = HanaMappedContainer<GeneratedConstructorsMap, ConceptTag, RoleTag, BooleanTag, NumericalTag>;

    static const dl::ConstructorLists empty_lists;

    HanaGeneratedConstructorsMaps m_generated_constructors;

public:
    /// @brief Get all sentences generated for a given constructor tag `D`.
    /// @tparam D is the constructor tag.
    /// @return
    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    const GeneratedConstructorsMap<D>& get() const;

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    dl::ConstructorList<D>& get(NonTerminal<D> nonterminal, size_t complexity);

    /// @brief Get all sentences generated for a given `nonterminal` with `complexity` and constructor tag `D`.
    /// @tparam D
    /// @param nonterminal
    /// @param complexity
    /// @return
    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    const dl::ConstructorList<D>& get(NonTerminal<D> nonterminal, size_t complexity) const;

    HanaGeneratedConstructorsMaps& get_hana_generated_constructors();
};

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
struct GeneratorStatistics
{
    size_t num_generated = 0;
    size_t num_pruned = 0;
    size_t num_kept = 0;

    GeneratorStatistics& operator+=(const GeneratorStatistics& other)
    {
        num_generated += other.num_generated;
        num_pruned += other.num_pruned;
        num_kept += other.num_kept;

        return *this;
    }
};

using HanaGeneratorStatistics = HanaMappedContainer<GeneratorStatistics, ConceptTag, RoleTag, BooleanTag, NumericalTag>;

/**
 * Concept
 */

class GeneratorVisitor : public RecurseVisitor
{
private:
    IRefinementPruningFunction& m_pruning_function;
    GeneratedSentencesContainer& m_sentences;
    dl::Repositories& m_repositories;
    size_t m_max_syntactic_complexity;

    size_t m_complexity;

    dl::ConstructorLists m_generated;
    HanaGeneratorStatistics m_statistics;

    std::any m_result;

public:
    GeneratorVisitor(IRefinementPruningFunction& pruning_function,
                     GeneratedSentencesContainer& sentences,
                     dl::Repositories& repositories,
                     size_t max_syntactic_complexity);

    void visit(ConceptBot constructor) override;
    void visit(ConceptTop constructor) override;
    void visit(ConceptAtomicState<formalism::StaticTag> constructor) override;
    void visit(ConceptAtomicState<formalism::FluentTag> constructor) override;
    void visit(ConceptAtomicState<formalism::DerivedTag> constructor) override;
    void visit(ConceptAtomicGoal<formalism::StaticTag> constructor) override;
    void visit(ConceptAtomicGoal<formalism::FluentTag> constructor) override;
    void visit(ConceptAtomicGoal<formalism::DerivedTag> constructor) override;
    void visit(ConceptNominal constructor) override;
    void visit(ConceptIntersection constructor) override;
    void visit(ConceptUnion constructor) override;
    void visit(ConceptNegation constructor) override;
    void visit(ConceptValueRestriction constructor) override;
    void visit(ConceptExistentialQuantification constructor) override;
    void visit(ConceptRoleValueMapContainment constructor) override;
    void visit(ConceptRoleValueMapEquality constructor) override;

    void visit(RoleUniversal constructor) override;
    void visit(RoleAtomicState<formalism::StaticTag> constructor) override;
    void visit(RoleAtomicState<formalism::FluentTag> constructor) override;
    void visit(RoleAtomicState<formalism::DerivedTag> constructor) override;
    void visit(RoleAtomicGoal<formalism::StaticTag> constructor) override;
    void visit(RoleAtomicGoal<formalism::FluentTag> constructor) override;
    void visit(RoleAtomicGoal<formalism::DerivedTag> constructor) override;
    void visit(RoleIntersection constructor) override;
    void visit(RoleUnion constructor) override;
    void visit(RoleComplement constructor) override;
    void visit(RoleInverse constructor) override;
    void visit(RoleComposition constructor) override;
    void visit(RoleTransitiveClosure constructor) override;
    void visit(RoleReflexiveTransitiveClosure constructor) override;
    void visit(RoleRestriction constructor) override;
    void visit(RoleIdentity constructor) override;

    void visit(BooleanAtomicState<formalism::StaticTag> constructor) override;
    void visit(BooleanAtomicState<formalism::FluentTag> constructor) override;
    void visit(BooleanAtomicState<formalism::DerivedTag> constructor) override;
    void visit(BooleanNonempty<ConceptTag> constructor) override;
    void visit(BooleanNonempty<RoleTag> constructor) override;

    void visit(NumericalCount<ConceptTag> constructor) override;
    void visit(NumericalCount<RoleTag> constructor) override;
    void visit(NumericalDistance constructor) override;

    void visit(DerivationRule<ConceptTag> rule) override;
    void visit(DerivationRule<RoleTag> rule) override;
    void visit(DerivationRule<BooleanTag> rule) override;
    void visit(DerivationRule<NumericalTag> rule) override;

    void visit(SubstitutionRule<ConceptTag> rule) override;
    void visit(SubstitutionRule<RoleTag> rule) override;
    void visit(SubstitutionRule<BooleanTag> rule) override;
    void visit(SubstitutionRule<NumericalTag> rule) override;

    void visit(const Grammar& grammar) override;

    const HanaGeneratorStatistics& get_statistics() const;

private:
    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(DerivationRule<D> rule);

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(SubstitutionRule<D> rule);

    const std::any& get_result() const;
};

}

#endif
