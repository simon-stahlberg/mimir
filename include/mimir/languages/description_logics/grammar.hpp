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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_HPP_

#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructors_interface.hpp"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace mimir::dl::grammar
{

/**
 * Grammar constructor hierarchy parallel to dl constructors.
 */

template<IsConceptOrRole D>
class Constructor
{
public:
    virtual bool test_match(const D& constructor) const = 0;
};

template<PredicateCategory P>
class ConceptPredicateState : public Constructor<Concept>
{
private:
    Predicate<P> m_predicate;

public:
    explicit ConceptPredicateState(Predicate<P> predicate);

    bool test_match(const dl::Concept& constructor) const override;
};

template<PredicateCategory P>
class ConceptPredicateGoal : public Constructor<Concept>
{
private:
    Predicate<P> m_predicate;

public:
    explicit ConceptPredicateGoal(Predicate<P> predicate);

    bool test_match(const dl::Concept& constructor) const override;
};

class ConceptAnd : public Constructor<Concept>
{
private:
    const Constructor<Concept>* m_concept_left;
    const Constructor<Concept>* m_concept_right;

public:
    ConceptAnd(const Constructor<Concept>* concept_left, const Constructor<Concept>* concept_right);

    bool test_match(const dl::Concept& constructor) const override;
};

/**
 * NonTerminal
 */

template<IsConceptOrRole D>
class DerivationRule;

template<IsConceptOrRole D>
class NonTerminal
{
protected:
    const DerivationRule<D>* m_rule;

public:
    bool test_match(const D& constructor) const { return m_rule->test_match(constructor); }
};

/**
 * Choice
 */

template<IsConceptOrRole D>
using Choice = std::variant<const Constructor<D>*, const NonTerminal<D>*>;

/**
 * DerivationRule
 */

template<IsConceptOrRole D>
class DerivationRule
{
protected:
    std::vector<Choice<D>> m_choices;

public:
    bool test_match(const D& constructor) const
    {
        return std::any_of(m_choices.begin(),
                           m_choices.end(),
                           [&constructor](const Choice<D>& choice)
                           { return std::visit([&constructor](const auto& arg) -> bool { return arg->test_match(constructor); }, choice); });
    }
};

/**
 * Grammar
 */

class Grammar
{
private:
    // Memory, the components cannot be shared due to cycles in grammars.
    std::vector<std::unique_ptr<Constructor<Concept>>> m_concept_storage;
    std::vector<std::unique_ptr<Constructor<Role>>> m_role_storage;
    std::vector<std::unique_ptr<NonTerminal<Concept>>> m_concept_non_terminal_storage;
    std::vector<std::unique_ptr<NonTerminal<Role>>> m_role_non_terminal_storage;
    std::vector<std::unique_ptr<DerivationRule<Concept>>> m_concept_rules_storage;
    std::vector<std::unique_ptr<DerivationRule<Role>>> m_role_rules_storage;

    /* The rules of the grammar. */
    std::vector<const DerivationRule<Concept>*> m_concept_rules;
    std::vector<const DerivationRule<Role>*> m_role_rules;

public:
    /// @brief Tests whether a dl concept constructor satisfies the grammar specification.
    /// @param constructor is the dl concept constructor to test.
    /// @return true iff the dl concept constructor satisfies the grammar specification, and false otherwise.
    bool test_match(const dl::Concept& constructor) const
    {
        return std::any_of(m_concept_rules.begin(),
                           m_concept_rules.end(),
                           [&constructor](const DerivationRule<Concept>* rule) { return rule->test_match(constructor); });
    }

    /// @brief Tests whether a dl role constructor satisfies the grammar specfication.
    /// @param constructor is the dl role constructor to test.
    /// @return true iff the dl concept constructor satisfies the grammar specification, and false otherwise.
    bool test_match(const dl::Role& constructor) const
    {
        return std::any_of(m_role_rules.begin(),
                           m_role_rules.end(),
                           [&constructor](const DerivationRule<Role>* rule) { return rule->test_match(constructor); });
    }
};

/**
 * Implementations
 */
}

#endif
