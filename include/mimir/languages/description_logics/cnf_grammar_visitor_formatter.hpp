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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_VISITOR_FORMATTER_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_VISITOR_FORMATTER_HPP_

#include "mimir/languages/description_logics/cnf_grammar_visitor_interface.hpp"

namespace mimir::languages::dl::cnf_grammar
{
class FormatterVisitor : public IVisitor
{
private:
    std::ostream& m_out;

public:
    FormatterVisitor(std::ostream& out);

    /* Concepts */
    void visit(ConceptBot constructor) override;
    void visit(ConceptTop constructor) override;
    void visit(ConceptAtomicState<formalism::StaticTag> constructor) override;
    void visit(ConceptAtomicState<formalism::FluentTag> constructor) override;
    void visit(ConceptAtomicState<formalism::DerivedTag> constructor) override;
    void visit(ConceptAtomicGoal<formalism::StaticTag> constructor) override;
    void visit(ConceptAtomicGoal<formalism::FluentTag> constructor) override;
    void visit(ConceptAtomicGoal<formalism::DerivedTag> constructor) override;
    void visit(ConceptIntersection constructor) override;
    void visit(ConceptUnion constructor) override;
    void visit(ConceptNegation constructor) override;
    void visit(ConceptValueRestriction constructor) override;
    void visit(ConceptExistentialQuantification constructor) override;
    void visit(ConceptRoleValueMapContainment constructor) override;
    void visit(ConceptRoleValueMapEquality constructor) override;
    void visit(ConceptNominal constructor) override;
    /* Roles */
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
    /* Booleans */
    void visit(BooleanAtomicState<formalism::StaticTag> constructor) override;
    void visit(BooleanAtomicState<formalism::FluentTag> constructor) override;
    void visit(BooleanAtomicState<formalism::DerivedTag> constructor) override;
    void visit(BooleanNonempty<ConceptTag> constructor) override;
    void visit(BooleanNonempty<RoleTag> constructor) override;
    /* Numericals */
    void visit(NumericalCount<ConceptTag> constructor) override;
    void visit(NumericalCount<RoleTag> constructor) override;
    void visit(NumericalDistance constructor) override;
    /* Nonterminals */
    void visit(NonTerminal<ConceptTag> constructor) override;
    void visit(NonTerminal<RoleTag> constructor) override;
    void visit(NonTerminal<BooleanTag> constructor) override;
    void visit(NonTerminal<NumericalTag> constructor) override;
    /* DerivationRules */
    void visit(DerivationRule<ConceptTag> constructor) override;
    void visit(DerivationRule<RoleTag> constructor) override;
    void visit(DerivationRule<BooleanTag> constructor) override;
    void visit(DerivationRule<NumericalTag> constructor) override;
    /* SubstitutionRules */
    void visit(SubstitutionRule<ConceptTag> constructor) override;
    void visit(SubstitutionRule<RoleTag> constructor) override;
    void visit(SubstitutionRule<BooleanTag> constructor) override;
    void visit(SubstitutionRule<NumericalTag> constructor) override;
    /* Grammar*/
    void visit(const Grammar& grammar) override;

private:
    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(NonTerminal<D> constructor);

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(DerivationRule<D> rule);

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(SubstitutionRule<D> rule);
};

/**
 * Printing
 */

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
extern std::ostream& operator<<(std::ostream& out, Constructor<D> element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
extern std::ostream& operator<<(std::ostream& out, NonTerminal<D> element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
extern std::ostream& operator<<(std::ostream& out, DerivationRule<D> element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
extern std::ostream& operator<<(std::ostream& out, SubstitutionRule<D> element);

extern std::ostream& operator<<(std::ostream& out, const Grammar& element);

}

#endif
