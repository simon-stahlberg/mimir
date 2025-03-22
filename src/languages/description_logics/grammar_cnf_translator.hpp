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
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
using ConstructorOrNonTerminal = std::variant<Constructor<D>, NonTerminal<D>>;

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
using DerivationOrSubstitutionRule = std::variant<DerivationRule<D>, SubstitutionRule<D>>;
}

namespace mimir::languages::dl::grammar
{

class ToCNFVisitor : public IVisitor
{
protected:
    cnf_grammar::Repositories& m_repositories;
    cnf_grammar::OptionalNonTerminals& m_start_symbols;
    cnf_grammar::DerivationRuleLists& m_derivation_rules;
    cnf_grammar::SubstitutionRuleLists& m_substitution_rules;

    std::any m_result;  ///< the result of a visitation

public:
    ToCNFVisitor(cnf_grammar::Repositories& repositories,
                 cnf_grammar::OptionalNonTerminals& start_symbols,
                 cnf_grammar::DerivationRuleLists& derivation_rules,
                 cnf_grammar::SubstitutionRuleLists& substitution_rules);

    void visit(ConceptBot constructor) override;
    void visit(ConceptTop constructor) override;
    void visit(ConceptAtomicState<StaticTag> constructor) override;
    void visit(ConceptAtomicState<FluentTag> constructor) override;
    void visit(ConceptAtomicState<DerivedTag> constructor) override;
    void visit(ConceptAtomicGoal<StaticTag> constructor) override;
    void visit(ConceptAtomicGoal<FluentTag> constructor) override;
    void visit(ConceptAtomicGoal<DerivedTag> constructor) override;
    void visit(ConceptIntersection constructor) override;
    void visit(ConceptUnion constructor) override;
    void visit(ConceptNegation constructor) override;
    void visit(ConceptValueRestriction constructor) override;
    void visit(ConceptExistentialQuantification constructor) override;
    void visit(ConceptRoleValueMapContainment constructor) override;
    void visit(ConceptRoleValueMapEquality constructor) override;
    void visit(ConceptNominal constructor) override;

    void visit(RoleUniversal constructor) override;
    void visit(RoleAtomicState<StaticTag> constructor) override;
    void visit(RoleAtomicState<FluentTag> constructor) override;
    void visit(RoleAtomicState<DerivedTag> constructor) override;
    void visit(RoleAtomicGoal<StaticTag> constructor) override;
    void visit(RoleAtomicGoal<FluentTag> constructor) override;
    void visit(RoleAtomicGoal<DerivedTag> constructor) override;
    void visit(RoleIntersection constructor) override;
    void visit(RoleUnion constructor) override;
    void visit(RoleComplement constructor) override;
    void visit(RoleInverse constructor) override;
    void visit(RoleComposition constructor) override;
    void visit(RoleTransitiveClosure constructor) override;
    void visit(RoleReflexiveTransitiveClosure constructor) override;
    void visit(RoleRestriction constructor) override;
    void visit(RoleIdentity constructor) override;

    void visit(BooleanAtomicState<StaticTag> constructor) override;
    void visit(BooleanAtomicState<FluentTag> constructor) override;
    void visit(BooleanAtomicState<DerivedTag> constructor) override;
    void visit(BooleanNonempty<ConceptTag> constructor) override;
    void visit(BooleanNonempty<RoleTag> constructor) override;

    void visit(NumericalCount<ConceptTag> constructor) override;
    void visit(NumericalCount<RoleTag> constructor) override;
    void visit(NumericalDistance constructor) override;

    void visit(NonTerminal<ConceptTag> constructor) override;
    void visit(NonTerminal<RoleTag> constructor) override;
    void visit(NonTerminal<BooleanTag> constructor) override;
    void visit(NonTerminal<NumericalTag> constructor) override;

    void visit(ConstructorOrNonTerminal<ConceptTag> constructor) override;
    void visit(ConstructorOrNonTerminal<RoleTag> constructor) override;
    void visit(ConstructorOrNonTerminal<BooleanTag> constructor) override;
    void visit(ConstructorOrNonTerminal<NumericalTag> constructor) override;

    void visit(DerivationRule<ConceptTag> constructor) override;
    void visit(DerivationRule<RoleTag> constructor) override;
    void visit(DerivationRule<BooleanTag> constructor) override;
    void visit(DerivationRule<NumericalTag> constructor) override;

    void visit(const Grammar& grammar) override;

    const std::any& get_result() const;

private:
    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(NonTerminal<D> constructor);

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(ConstructorOrNonTerminal<D> constructor);

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(DerivationRule<D> constructor);
};

extern cnf_grammar::Grammar translate_to_cnf(const Grammar& grammar);

}

#endif
