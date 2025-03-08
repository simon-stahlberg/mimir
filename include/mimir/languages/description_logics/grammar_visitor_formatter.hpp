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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_VISITOR_FORMATTER_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_VISITOR_FORMATTER_HPP_

#include "mimir/languages/description_logics/grammar_visitor_interface.hpp"

#include <variant>

namespace mimir::dl::grammar
{

////////////////////////////
/// Recursive Visitor
////////////////////////////

template<FeatureCategory D>
class FormatterConstructorVisitor;

template<FeatureCategory D>
class FormatterConstructorOrNonTerminalVisitor;

template<FeatureCategory D>
class FormatterNonTerminalVisitor;

template<FeatureCategory D>
class FormatterDerivationRuleVisitor;

class FormatterGrammarVisitor;

/**
 * Concept
 */

template<FeatureCategory D>
class FormatterConstructorVisitor : public ConstructorVisitor<D>
{
};

template<>
class FormatterConstructorVisitor<Concept> : public ConstructorVisitor<Concept>
{
private:
    std::ostream& m_out;

protected:
    FormatterConstructorOrNonTerminalVisitor<Concept>* m_concept_or_nonterminal_visitor;
    FormatterConstructorOrNonTerminalVisitor<Role>* m_role_or_nonterminal_visitor;

public:
    FormatterConstructorVisitor(std::ostream& out);

    virtual void initialize(FormatterConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                            FormatterConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor);

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
};

/**
 * Role
 */

template<>
class FormatterConstructorVisitor<Role> : public ConstructorVisitor<Role>
{
private:
    std::ostream& m_out;

protected:
    FormatterConstructorOrNonTerminalVisitor<Concept>* m_concept_or_nonterminal_visitor;
    FormatterConstructorOrNonTerminalVisitor<Role>* m_role_or_nonterminal_visitor;

public:
    FormatterConstructorVisitor(std::ostream& out);

    virtual void initialize(FormatterConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                            FormatterConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor);

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
};

/**
 * ConstructorOrRoleNonTerminal
 */

template<FeatureCategory D>
class FormatterConstructorOrNonTerminalVisitor : public ConstructorOrNonTerminalVisitor<D>
{
private:
    std::ostream& m_out;

protected:
    FormatterNonTerminalVisitor<D>* m_nonterminal_visitor;
    FormatterConstructorVisitor<D>* m_constructor_visitor;

public:
    FormatterConstructorOrNonTerminalVisitor(std::ostream& out);

    virtual void initialize(FormatterNonTerminalVisitor<D>& nonterminal_visitor, FormatterConstructorVisitor<D>& constructor_visitor);

    void visit(ConstructorOrNonTerminal<D> constructor) override;
};

/**
 * NonTerminal
 */

template<FeatureCategory D>
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

template<FeatureCategory D>
class FormatterDerivationRuleVisitor : public DerivationRuleVisitor<D>
{
private:
    std::ostream& m_out;

protected:
    FormatterNonTerminalVisitor<D>* m_nonterminal_visitor;
    FormatterConstructorOrNonTerminalVisitor<D>* m_constructor_or_nonterminal_visitor;

public:
    FormatterDerivationRuleVisitor(std::ostream& out);

    virtual void initialize(FormatterNonTerminalVisitor<D>& nonterminal_visitor,
                            FormatterConstructorOrNonTerminalVisitor<D>& constructor_or_nonterminal_visitor);

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

    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, FormatterDerivationRuleVisitor<Concept>*>,
                     boost::hana::pair<boost::hana::type<Role>, FormatterDerivationRuleVisitor<Role>*>>
        m_derivation_rule_visitor;

public:
    FormatterGrammarVisitor(std::ostream& out);

    virtual void visit(const Grammar& grammar) override;

    virtual void initialize(FormatterNonTerminalVisitor<Concept>& concept_start_symbol_visitor,
                            FormatterNonTerminalVisitor<Role>& role_start_symbol_visitor,
                            FormatterDerivationRuleVisitor<Concept>& concept_rule_visitor,
                            FormatterDerivationRuleVisitor<Role>& role_rule_visitor);
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

extern std::ostream& operator<<(std::ostream& out, const Grammar& element);

}

#endif
