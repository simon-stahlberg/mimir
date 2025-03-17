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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CNF_TRANSLATOR_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CNF_TRANSLATOR_HPP_

#include "mimir/languages/description_logics/cnf_grammar.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/grammar_visitor_interface.hpp"

using namespace mimir::formalism;

namespace mimir::languages::dl::cnf_grammar
{
template<FeatureCategory D>
using ConstructorOrNonTerminal = std::variant<Constructor<D>, NonTerminal<D>>;

template<FeatureCategory D>
using DerivationOrSubstitutionRule = std::variant<DerivationRule<D>, SubstitutionRule<D>>;
}

namespace mimir::languages::dl::grammar
{

class ToCNFVisitor : public IVisitor
{
protected:
    cnf_grammar::Repositories& m_repositories;
    cnf_grammar::StartSymbolsContainer& m_start_symbols;
    cnf_grammar::DerivationRulesContainer& m_derivation_rules;
    cnf_grammar::SubstitutionRulesContainer& m_substitution_rules;

    std::any m_result;  ///< the result of a visitation

public:
    ToCNFVisitor(cnf_grammar::Repositories& repositories,
                 cnf_grammar::StartSymbolsContainer& start_symbols,
                 cnf_grammar::DerivationRulesContainer& derivation_rules,
                 cnf_grammar::SubstitutionRulesContainer& substitution_rules);

    void visit(ConceptBot constructor) override;
    void visit(ConceptTop constructor) override;
    void visit(ConceptAtomicState<Static> constructor) override;
    void visit(ConceptAtomicState<Fluent> constructor) override;
    void visit(ConceptAtomicState<Derived> constructor) override;
    void visit(ConceptAtomicGoal<Static> constructor) override;
    void visit(ConceptAtomicGoal<Fluent> constructor) override;
    void visit(ConceptAtomicGoal<Derived> constructor) override;
    void visit(ConceptIntersection constructor) override;
    void visit(ConceptUnion constructor) override;
    void visit(ConceptNegation constructor) override;
    void visit(ConceptValueRestriction constructor) override;
    void visit(ConceptExistentialQuantification constructor) override;
    void visit(ConceptRoleValueMapContainment constructor) override;
    void visit(ConceptRoleValueMapEquality constructor) override;
    void visit(ConceptNominal constructor) override;

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

    void visit(NonTerminal<Concept> constructor) override;
    void visit(NonTerminal<Role> constructor) override;
    void visit(NonTerminal<Boolean> constructor) override;
    void visit(NonTerminal<Numerical> constructor) override;

    void visit(ConstructorOrNonTerminal<Concept> constructor) override;
    void visit(ConstructorOrNonTerminal<Role> constructor) override;
    void visit(ConstructorOrNonTerminal<Boolean> constructor) override;
    void visit(ConstructorOrNonTerminal<Numerical> constructor) override;

    void visit(DerivationRule<Concept> constructor) override;
    void visit(DerivationRule<Role> constructor) override;
    void visit(DerivationRule<Boolean> constructor) override;
    void visit(DerivationRule<Numerical> constructor) override;

    void visit(const Grammar& grammar) override;

    const std::any& get_result() const;

private:
    template<FeatureCategory D>
    void visit_impl(NonTerminal<D> constructor);

    template<FeatureCategory D>
    void visit_impl(ConstructorOrNonTerminal<D> constructor);

    template<FeatureCategory D>
    void visit_impl(DerivationRule<D> constructor);
};

extern cnf_grammar::Grammar translate_to_cnf(const Grammar& grammar);

}

#endif
