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

namespace mimir::dl::cnf_grammar
{
class FormatterVisitor : public Visitor
{
private:
    std::ostream& m_out;

public:
    FormatterVisitor(std::ostream& out);

    /* Concepts */
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
    /* Roles */
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
    /* Booleans */
    void visit(BooleanAtomicState<Static> constructor) override;
    void visit(BooleanAtomicState<Fluent> constructor) override;
    void visit(BooleanAtomicState<Derived> constructor) override;
    void visit(BooleanNonempty<Concept> constructor) override;
    void visit(BooleanNonempty<Role> constructor) override;
    /* Numericals */
    void visit(NumericalCount<Concept> constructor) override;
    void visit(NumericalCount<Role> constructor) override;
    void visit(NumericalDistance constructor) override;
    /* Nonterminals */
    void visit(NonTerminal<Concept> constructor) override;
    void visit(NonTerminal<Role> constructor) override;
    void visit(NonTerminal<Boolean> constructor) override;
    void visit(NonTerminal<Numerical> constructor) override;
    /* DerivationRules */
    void visit(DerivationRule<Concept> constructor) override;
    void visit(DerivationRule<Role> constructor) override;
    void visit(DerivationRule<Boolean> constructor) override;
    void visit(DerivationRule<Numerical> constructor) override;
    /* SubstitutionRules */
    void visit(SubstitutionRule<Concept> constructor) override;
    void visit(SubstitutionRule<Role> constructor) override;
    void visit(SubstitutionRule<Boolean> constructor) override;
    void visit(SubstitutionRule<Numerical> constructor) override;
    /* Grammar*/
    void visit(const Grammar& grammar) override;

private:
    template<FeatureCategory D>
    void visit_impl(NonTerminal<D> constructor);

    template<FeatureCategory D>
    void visit_impl(DerivationRule<D> rule);

    template<FeatureCategory D>
    void visit_impl(SubstitutionRule<D> rule);
};

/**
 * Printing
 */

template<FeatureCategory D>
extern std::ostream& operator<<(std::ostream& out, Constructor<D> element);

template<FeatureCategory D>
extern std::ostream& operator<<(std::ostream& out, NonTerminal<D> element);

template<FeatureCategory D>
extern std::ostream& operator<<(std::ostream& out, DerivationRule<D> element);

template<FeatureCategory D>
extern std::ostream& operator<<(std::ostream& out, SubstitutionRule<D> element);

extern std::ostream& operator<<(std::ostream& out, const Grammar& element);

}

#endif
