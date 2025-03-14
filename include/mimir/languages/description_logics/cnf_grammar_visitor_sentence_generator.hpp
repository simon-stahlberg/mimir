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
    template<FeatureCategory D>
    using GeneratedConstructorsMap = std::unordered_map<NonTerminal<D>, std::vector<dl::ConstructorList<D>>>;

    template<FeatureCategory... Ds>
    using HanaGeneratedConstructorsMaps = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, GeneratedConstructorsMap<Ds>>...>;

    static const ConstructorLists<Concept, Role, Boolean, Numerical> empty_lists;

    HanaGeneratedConstructorsMaps<Concept, Role, Boolean, Numerical> m_generated_constructors;

public:
    template<FeatureCategory D>
    dl::ConstructorList<D>& get(NonTerminal<D> nonterminal, size_t complexity)
    {
        auto& constructors_by_complexity = boost::hana::at_key(m_generated_constructors, boost::hana::type<D> {})[nonterminal];
        if (complexity >= constructors_by_complexity.size())
        {
            constructors_by_complexity.resize(complexity + 1);
        }
        return constructors_by_complexity[complexity];
    }

    template<FeatureCategory D>
    const dl::ConstructorList<D>& get(NonTerminal<D> nonterminal, size_t complexity) const
    {
        const auto& container = boost::hana::at_key(m_generated_constructors, boost::hana::type<D> {});

        auto it = container.find(nonterminal);
        if (it == container.end() || complexity >= it->second.size())
        {
            return boost::hana::at_key(empty_lists, boost::hana::type<D> {});
        }

        return it->second.at(complexity);
    }

    HanaGeneratedConstructorsMaps<Concept, Role, Boolean, Numerical>& get_hana_generated_constructors() { return m_generated_constructors; }
};

template<FeatureCategory D>
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

template<FeatureCategory... Ds>
using HanaGeneratorStatistics = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, GeneratorStatistics<Ds>>...>;

/**
 * Concept
 */

class GeneratorVisitor : public RecurseVisitor
{
private:
    RefinementPruningFunction& m_pruning_function;
    GeneratedSentencesContainer& m_sentences;
    dl::ConstructorRepositories& m_repositories;
    size_t m_max_syntactic_complexity;

    size_t m_complexity;

    dl::ConstructorLists<Concept, Role, Boolean, Numerical> m_generated;
    HanaGeneratorStatistics<Concept, Role, Boolean, Numerical> m_statistics;

    std::any m_result;

public:
    GeneratorVisitor(RefinementPruningFunction& pruning_function,
                     GeneratedSentencesContainer& sentences,
                     dl::ConstructorRepositories& repositories,
                     size_t max_syntactic_complexity);

    void visit(ConceptBot constructor) override;
    void visit(ConceptTop constructor) override;
    void visit(ConceptAtomicState<Static> constructor) override;
    void visit(ConceptAtomicState<Fluent> constructor) override;
    void visit(ConceptAtomicState<Derived> constructor) override;
    void visit(ConceptAtomicGoal<Static> constructor) override;
    void visit(ConceptAtomicGoal<Fluent> constructor) override;
    void visit(ConceptAtomicGoal<Derived> constructor) override;
    void visit(ConceptNominal constructor) override;
    void visit(ConceptIntersection constructor) override;
    void visit(ConceptUnion constructor) override;
    void visit(ConceptNegation constructor) override;
    void visit(ConceptValueRestriction constructor) override;
    void visit(ConceptExistentialQuantification constructor) override;
    void visit(ConceptRoleValueMapContainment constructor) override;
    void visit(ConceptRoleValueMapEquality constructor) override;

    void visit(RoleUniversal constructor) override;
    void visit(RoleAtomicState<Static> constructor) override;
    void visit(RoleAtomicState<Fluent> constructor) override;
    void visit(RoleAtomicState<Derived> constructor) override;
    void visit(RoleAtomicGoal<Static> constructor) override;
    void visit(RoleAtomicGoal<Fluent> constructor) override;
    void visit(RoleAtomicGoal<Derived> constructor) override;
    void visit(RoleIntersection constructor) override;
    void visit(RoleUnion constructor) override;
    void visit(RoleComplement constructor) override;
    void visit(RoleInverse constructor) override;
    void visit(RoleComposition constructor) override;
    void visit(RoleTransitiveClosure constructor) override;
    void visit(RoleReflexiveTransitiveClosure constructor) override;
    void visit(RoleRestriction constructor) override;
    void visit(RoleIdentity constructor) override;

    void visit(BooleanAtomicState<Static> constructor) override;
    void visit(BooleanAtomicState<Fluent> constructor) override;
    void visit(BooleanAtomicState<Derived> constructor) override;
    void visit(BooleanNonempty<Concept> constructor) override;
    void visit(BooleanNonempty<Role> constructor) override;

    void visit(NumericalCount<Concept> constructor) override;
    void visit(NumericalCount<Role> constructor) override;
    void visit(NumericalDistance constructor) override;

    void visit(DerivationRule<Concept> rule) override;
    void visit(DerivationRule<Role> rule) override;
    void visit(DerivationRule<Boolean> rule) override;
    void visit(DerivationRule<Numerical> rule) override;

    void visit(SubstitutionRule<Concept> rule) override;
    void visit(SubstitutionRule<Role> rule) override;
    void visit(SubstitutionRule<Boolean> rule) override;
    void visit(SubstitutionRule<Numerical> rule) override;

    void visit(const Grammar& grammar) override;

    const HanaGeneratorStatistics<Concept, Role, Boolean, Numerical>& get_statistics() const;

private:
    template<FeatureCategory D>
    void visit_impl(DerivationRule<D> rule);

    template<FeatureCategory D>
    void visit_impl(SubstitutionRule<D> rule);

    const std::any& get_result() const;
};

}

#endif
