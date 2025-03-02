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

#include <variant>

namespace mimir::dl::cnf_grammar
{

////////////////////////////
/// Recursive Visitor
////////////////////////////

template<ConceptOrRole D, PrimitiveOrComposite C>
class FormatterConstructorVisitor;

template<ConceptOrRole D>
class FormatterNonTerminalVisitor;

template<ConceptOrRole D, PrimitiveOrComposite C>
class FormatterDerivationRuleVisitor;

template<ConceptOrRole D>
class FormatterSubstitutionRuleVisitor;

class FormatterGrammarVisitor;

/**
 * Concept
 */

template<ConceptOrRole D, PrimitiveOrComposite C>
class FormatterConstructorVisitor : public ConstructorVisitor<D, C>
{
};

template<>
class FormatterConstructorVisitor<Concept, Primitive> : public ConstructorVisitor<Concept, Primitive>
{
private:
    std::ostream& m_out;

public:
    FormatterConstructorVisitor(std::ostream& out);

    void visit(ConceptBot constructor) override;
    void visit(ConceptTop constructor) override;
    void visit(ConceptAtomicState<Static> constructor) override;
    void visit(ConceptAtomicState<Fluent> constructor) override;
    void visit(ConceptAtomicState<Derived> constructor) override;
    void visit(ConceptAtomicGoal<Static> constructor) override;
    void visit(ConceptAtomicGoal<Fluent> constructor) override;
    void visit(ConceptAtomicGoal<Derived> constructor) override;
    void visit(ConceptNominal constructor) override;
};

template<>
class FormatterConstructorVisitor<Concept, Composite> : public ConstructorVisitor<Concept, Composite>
{
private:
    std::ostream& m_out;

protected:
    FormatterNonTerminalVisitor<Concept>* m_concept_or_nonterminal_visitor;
    FormatterNonTerminalVisitor<Role>* m_role_or_nonterminal_visitor;

public:
    FormatterConstructorVisitor(std::ostream& out);

    virtual void initialize(FormatterNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                            FormatterNonTerminalVisitor<Role>& role_or_nonterminal_visitor);

    void visit(ConceptIntersection constructor) override;
    void visit(ConceptUnion constructor) override;
    void visit(ConceptNegation constructor) override;
    void visit(ConceptValueRestriction constructor) override;
    void visit(ConceptExistentialQuantification constructor) override;
    void visit(ConceptRoleValueMapContainment constructor) override;
    void visit(ConceptRoleValueMapEquality constructor) override;
};

/**
 * Role
 */

template<>
class FormatterConstructorVisitor<Role, Primitive> : public ConstructorVisitor<Role, Primitive>
{
private:
    std::ostream& m_out;

public:
    FormatterConstructorVisitor(std::ostream& out);

    void visit(RoleUniversal constructor) override;
    void visit(RoleAtomicState<Static> constructor) override;
    void visit(RoleAtomicState<Fluent> constructor) override;
    void visit(RoleAtomicState<Derived> constructor) override;
    void visit(RoleAtomicGoal<Static> constructor) override;
    void visit(RoleAtomicGoal<Fluent> constructor) override;
    void visit(RoleAtomicGoal<Derived> constructor) override;
};

template<>
class FormatterConstructorVisitor<Role, Composite> : public ConstructorVisitor<Role, Composite>
{
private:
    std::ostream& m_out;

protected:
    FormatterNonTerminalVisitor<Concept>* m_concept_or_nonterminal_visitor;
    FormatterNonTerminalVisitor<Role>* m_role_or_nonterminal_visitor;

public:
    FormatterConstructorVisitor(std::ostream& out);

    virtual void initialize(FormatterNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                            FormatterNonTerminalVisitor<Role>& role_or_nonterminal_visitor);

    void visit(RoleIntersection constructor) override;
    void visit(RoleUnion constructor) override;
    void visit(RoleComplement constructor) override;
    void visit(RoleInverse constructor) override;
    void visit(RoleComposition constructor) override;
    void visit(RoleTransitiveClosure constructor) override;
    void visit(RoleReflexiveTransitiveClosure constructor) override;
    void visit(RoleRestriction constructor) override;
    void visit(RoleIdentity constructor) override;
};

/**
 * NonTerminal
 */

template<ConceptOrRole D>
class FormatterNonTerminalVisitor : public NonTerminalVisitor<D>
{
private:
    std::ostream& m_out;

public:
    FormatterNonTerminalVisitor(std::ostream& out);

    void visit(NonTerminal<D> constructor) override;
};

/**
 * DerivationRule
 */

template<ConceptOrRole D, PrimitiveOrComposite C>
class FormatterDerivationRuleVisitor : public DerivationRuleVisitor<D, C>
{
private:
    std::ostream& m_out;

protected:
    FormatterNonTerminalVisitor<D, C>* m_nonterminal_visitor;
    FormatterConstructorVisitor<D, C>* m_constructor_visitor;

public:
    FormatterDerivationRuleVisitor(std::ostream& out);

    virtual void initialize(FormatterNonTerminalVisitor<D, C>& nonterminal_visitor, FormatterConstructorVisitor<D, C>& constructor_visitor);

    void visit(DerivationRule<D> constructor) override;
};

/**
 * SubstitutionRule
 */

template<ConceptOrRole D>
class FormatterSubstitutionRuleVisitor : public SubstitutionRuleVisitor<D>
{
private:
    std::ostream& m_out;

protected:
    FormatterNonTerminalVisitor<D>* m_nonterminal_visitor;

public:
    FormatterDerivationRuleVisitor(std::ostream& out);

    virtual void initialize(FormatterNonTerminalVisitor<D>& nonterminal_visitor);

    void visit(DerivationRule<D> constructor) override;
};

/**
 * Grammar
 */

class FormatterGrammarVisitor : public GrammarVisitor
{
private:
    std::ostream& m_out;

protected:
    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, FormatterNonTerminalVisitor<Concept>*>,
                     boost::hana::pair<boost::hana::type<Role>, FormatterNonTerminalVisitor<Role>*>>
        m_start_symbol_visitor;

    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>,
                                       boost::hana::map<boost::hana::pair<boost::hana::type<Primitive>, FormatterDerivationRuleVisitor<Concept, Primitive>*>,
                                                        boost::hana::pair<boost::hana::type<Composite>, FormatterDerivationRuleVisitor<Concept, Composite>*>>>,
                     boost::hana::pair<boost::hana::type<Role>,
                                       boost::hana::map<boost::hana::pair<boost::hana::type<Primitive>, FormatterDerivationRuleVisitor<Role, Primitive>*>,
                                                        boost::hana::pair<boost::hana::type<Composite>, FormatterDerivationRuleVisitor<Role, Composite>*>>>>
        m_derivation_rule_visitor;

public:
    FormatterGrammarVisitor(std::ostream& out);

    virtual void visit(const Grammar& grammar) override;

    virtual void initialize(FormatterNonTerminalVisitor<Concept>& concept_start_symbol_visitor,
                            FormatterNonTerminalVisitor<Role>& role_start_symbol_visitor,
                            FormatterDerivationRuleVisitor<Concept, Primitive>& concept_primitive_rule_visitor,
                            FormatterDerivationRuleVisitor<Concept, Composite>& concept_composite_rule_visitor,
                            FormatterDerivationRuleVisitor<Role, Primitive>& role_primitive_rule_visitor,
                            FormatterDerivationRuleVisitor<Role, Composite>& role_composite_rule_visitor);
};

/**
 * Printing
 */

template<ConceptOrRole D, PrimitiveOrComposite C>
extern std::ostream& operator<<(std::ostream& out, Constructor<D, C> element);

template<ConceptOrRole D>
extern std::ostream& operator<<(std::ostream& out, NonTerminal<D> element);

template<ConceptOrRole D, PrimitiveOrComposite C>
extern std::ostream& operator<<(std::ostream& out, DerivationRule<D, C> element);

template<ConceptOrRole D>
extern std::ostream& operator<<(std::ostream& out, SubstitutionRule<D> element);

extern std::ostream& operator<<(std::ostream& out, const Grammar& element);

}

#endif
