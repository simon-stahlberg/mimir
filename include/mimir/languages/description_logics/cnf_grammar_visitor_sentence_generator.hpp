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

#include <unordered_map>
#include <vector>

namespace mimir::dl::cnf_grammar
{

////////////////////////////
/// Interface
////////////////////////////

class GeneratedSentencesContainer
{
private:
    template<ConceptOrRole D>
    using GeneratedConstructorsMap = std::unordered_map<NonTerminal<D>, std::vector<dl::ConstructorList<D>>>;

    using HanaGeneratedConstructorsMap = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, GeneratedConstructorsMap<Concept>>,
                                                          boost::hana::pair<boost::hana::type<Role>, GeneratedConstructorsMap<Role>>>;

    static const ConstructorLists<Concept, Role> empty_lists;

    HanaGeneratedConstructorsMap m_generated_constructors;

public:
    template<ConceptOrRole D>
    dl::ConstructorList<D>& get(NonTerminal<D> nonterminal, size_t complexity)
    {
        auto& constructors_by_complexity = boost::hana::at_key(m_generated_constructors, boost::hana::type<D> {})[nonterminal];
        if (complexity >= constructors_by_complexity.size())
        {
            constructors_by_complexity.resize(complexity + 1);
        }
        return constructors_by_complexity[complexity];
    }

    template<ConceptOrRole D>
    const dl::ConstructorList<D>& get(NonTerminal<D> nonterminal, size_t complexity) const
    {
        const auto& container = boost::hana::at_key(m_generated_constructors, boost::hana::type<D> {});

        auto it = container.find(nonterminal);
        if (it == container.end())
        {
            return boost::hana::at_key(empty_lists, boost::hana::type<D> {});
        }

        return it->second.at(complexity);
    }

    HanaGeneratedConstructorsMap& get_hana_generated_constructors() { return m_generated_constructors; }
};

/**
 * Constructors
 */

template<ConceptOrRole D>
class GeneratorConstructorVisitor : public ConstructorVisitor<D>
{
};

/**
 * Concept
 */

template<>
class GeneratorConstructorVisitor<Concept> : public ConstructorVisitor<Concept>
{
private:
    const GeneratedSentencesContainer& m_sentences;
    dl::ConstructorRepositories& m_repositories;
    size_t m_complexity;

    dl::ConstructorList<Concept> m_result;

public:
    explicit GeneratorConstructorVisitor(const GeneratedSentencesContainer& sentences, dl::ConstructorRepositories& repositories, size_t complexity);

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

    const dl::ConstructorList<Concept>& get_result() const;
};

/**
 * Role
 */

template<>
class GeneratorConstructorVisitor<Role> : public ConstructorVisitor<Role>
{
    const GeneratedSentencesContainer& m_sentences;
    dl::ConstructorRepositories& m_repositories;
    size_t m_complexity;

    dl::ConstructorList<Role> m_result;

public:
    explicit GeneratorConstructorVisitor(const GeneratedSentencesContainer& sentences, dl::ConstructorRepositories& repositories, size_t complexity);

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

    const dl::ConstructorList<Role>& get_result() const;
};

/**
 * DerivationRule
 */

template<ConceptOrRole D>
class GeneratorDerivationRuleVisitor : public DerivationRuleVisitor<D>
{
private:
    const RefinementPruningFunction& m_pruning_function;
    GeneratedSentencesContainer& m_sentences;
    dl::ConstructorRepositories& m_repositories;
    size_t m_complexity;

public:
    explicit GeneratorDerivationRuleVisitor(const RefinementPruningFunction& pruning_function,
                                            GeneratedSentencesContainer& sentences,
                                            dl::ConstructorRepositories& repositories,
                                            size_t complexity);

    void visit(DerivationRule<D> rule) override;
};

/**
 * SubstitutionRule
 */

template<ConceptOrRole D>
class GeneratorSubstitutionRuleVisitor : public SubstitutionRuleVisitor<D>
{
private:
    GeneratedSentencesContainer& m_sentences;
    dl::ConstructorRepositories& m_repositories;
    size_t m_complexity;

public:
    explicit GeneratorSubstitutionRuleVisitor(GeneratedSentencesContainer& sentences, dl::ConstructorRepositories& repositories, size_t complexity);

    void visit(SubstitutionRule<D> rule) override;
};

/**
 * Grammar
 */

class GeneratorGrammarVisitor : public GrammarVisitor
{
private:
    const RefinementPruningFunction& m_pruning_function;
    GeneratedSentencesContainer& m_sentences;
    dl::ConstructorRepositories& m_repositories;
    size_t m_max_syntactic_complexity;

public:
    explicit GeneratorGrammarVisitor(const RefinementPruningFunction& pruning_function,
                                     GeneratedSentencesContainer& sentences,
                                     dl::ConstructorRepositories& repositories,
                                     size_t max_syntactic_complexity);

    void visit(const Grammar& grammar) override;
};

}

#endif
