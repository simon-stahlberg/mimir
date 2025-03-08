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

template<FeatureCategory D>
class FormatterConstructorVisitor;

template<FeatureCategory D>
class FormatterNonTerminalVisitor;

template<FeatureCategory D>
class FormatterDerivationRuleVisitor;

template<FeatureCategory D>
class FormatterSubstitutionRuleVisitor;

class FormatterGrammarVisitor;

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
class FormatterConstructorVisitor<Role> : public ConstructorVisitor<Role>
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
 * DerivationRule
 */

template<FeatureCategory D>
class FormatterDerivationRuleVisitor : public DerivationRuleVisitor<D>
{
private:
    std::ostream& m_out;

public:
    FormatterDerivationRuleVisitor(std::ostream& out);

    void visit(DerivationRule<D> constructor) override;
};

/**
 * SubstitutionRule
 */

template<FeatureCategory D>
class FormatterSubstitutionRuleVisitor : public SubstitutionRuleVisitor<D>
{
private:
    std::ostream& m_out;

public:
    FormatterSubstitutionRuleVisitor(std::ostream& out);

    void visit(SubstitutionRule<D> constructor) override;
};

/**
 * Grammar
 */

class FormatterGrammarVisitor : public GrammarVisitor
{
private:
    std::ostream& m_out;

public:
    FormatterGrammarVisitor(std::ostream& out);

    void visit(const Grammar& grammar) override;
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
