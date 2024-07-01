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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONSTRUCTORS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONSTRUCTORS_HPP_

#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructor_ids.hpp"
#include "mimir/languages/description_logics/grammar_constructor_interface.hpp"
#include "mimir/languages/description_logics/grammar_visitors.hpp"

#include <optional>

namespace mimir::dl
{
template<typename T>
class ConstructorRepository;

namespace grammar
{

/**
 * NonTerminal
 */

template<dl::IsConceptOrRole D>
class DerivationRule;

template<dl::IsConceptOrRole D>
class NonTerminal
{
private:
    size_t m_id;
    // Use name for equality and hash since rule is deferred instantiated.
    std::string m_name;
    // Mutable for deferred instantiation.
    mutable std::optional<std::reference_wrapper<const DerivationRule<D>>> m_rule;

    NonTerminal(size_t id, std::string name);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    NonTerminal(const NonTerminal& other) = delete;
    NonTerminal& operator=(const NonTerminal& other) = delete;
    NonTerminal(NonTerminal&& other) = default;
    NonTerminal& operator=(NonTerminal&& other) = default;

    bool operator==(const NonTerminal& other) const;
    size_t hash() const;

    bool test_match(const dl::Constructor<D>& constructor) const;

    size_t get_id() const;
    const std::string& get_name() const;
    const DerivationRule<D>& get_rule() const;

    // Deferred initialization for internal usage. Users should not use it.
    void set_rule(const DerivationRule<D>& rule) const;
};

using ConceptNonTerminal = NonTerminal<Concept>;
using RoleNonTerminal = NonTerminal<Role>;

/**
 * Choice
 */

template<IsConceptOrRole D>
using ConstructorOrNonTerminalChoice = std::variant<std::reference_wrapper<const Constructor<D>>, std::reference_wrapper<const NonTerminal<D>>>;

template<dl::IsConceptOrRole D>
class Choice
{
private:
    size_t m_id;
    ConstructorOrNonTerminalChoice<D> m_choice;

    Choice(size_t id, ConstructorOrNonTerminalChoice<D> choice);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    Choice(const Choice& other) = delete;
    Choice& operator=(const Choice& other) = delete;
    Choice(Choice&& other) = default;
    Choice& operator=(Choice&& other) = default;

    bool operator==(const Choice& other) const;
    size_t hash() const;

    bool test_match(const dl::Constructor<D>& constructor) const;

    size_t get_id() const;
    const ConstructorOrNonTerminalChoice<D>& get_choice() const;
};

template<dl::IsConceptOrRole D>
using ChoiceList = std::vector<std::reference_wrapper<const Choice<D>>>;

using ConceptChoice = Choice<Concept>;
using ConceptChoiceList = ChoiceList<Concept>;
using RoleChoice = Choice<Role>;
using RoleChoiceList = ChoiceList<Role>;

/**
 * DerivationRule
 */

template<dl::IsConceptOrRole D>
class DerivationRule
{
protected:
    size_t m_id;
    ChoiceList<D> m_choices;

    DerivationRule(size_t id, ChoiceList<D> choices);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    DerivationRule(const DerivationRule& other) = delete;
    DerivationRule& operator=(const DerivationRule& other) = delete;
    DerivationRule(DerivationRule&& other) = default;
    DerivationRule& operator=(DerivationRule&& other) = default;

    bool operator==(const DerivationRule& other) const;
    size_t hash() const;

    bool test_match(const dl::Constructor<D>& constructor) const;

    size_t get_id() const;
    const ChoiceList<D>& get_choices() const;
};

template<IsConceptOrRole D>
using DerivationRuleList = std::vector<std::reference_wrapper<const DerivationRule<D>>>;

using ConceptDerivationRule = DerivationRule<Concept>;
using ConceptDerivationRuleList = DerivationRuleList<Concept>;
using RoleDerivationRule = DerivationRule<Role>;
using RoleDerivationRuleList = DerivationRuleList<Role>;

/**
 * Concepts
 */

template<PredicateCategory P>
class ConceptPredicateState : public Constructor<Concept>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    ConceptPredicateState(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    ConceptPredicateState(const ConceptPredicateState& other) = delete;
    ConceptPredicateState& operator=(const ConceptPredicateState& other) = delete;
    ConceptPredicateState(ConceptPredicateState&& other) = default;
    ConceptPredicateState& operator=(ConceptPredicateState&& other) = default;

    bool operator==(const ConceptPredicateState& other) const;
    bool is_equal(const Constructor<Concept>& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Constructor<Concept>& constructor) const override;

    size_t get_id() const override;
    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class ConceptPredicateGoal : public Constructor<Concept>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    ConceptPredicateGoal(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    ConceptPredicateGoal(const ConceptPredicateGoal& other) = delete;
    ConceptPredicateGoal& operator=(const ConceptPredicateGoal& other) = delete;
    ConceptPredicateGoal(ConceptPredicateGoal&& other) = default;
    ConceptPredicateGoal& operator=(ConceptPredicateGoal&& other) = default;

    bool operator==(const ConceptPredicateGoal& other) const;
    bool is_equal(const Constructor<Concept>& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Constructor<Concept>& constructor) const override;

    size_t get_id() const override;
    Predicate<P> get_predicate() const;
};

class ConceptAnd : public Constructor<Concept>
{
private:
    size_t m_id;
    const ConceptChoice& m_concept_left;
    const ConceptChoice& m_concept_right;

    ConceptAnd(size_t id, const ConceptChoice& concept_left, const ConceptChoice& concept_right);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    ConceptAnd(const ConceptAnd& other) = delete;
    ConceptAnd& operator=(const ConceptAnd& other) = delete;
    ConceptAnd(ConceptAnd&& other) = default;
    ConceptAnd& operator=(ConceptAnd&& other) = default;

    bool operator==(const ConceptAnd& other) const;
    bool is_equal(const Constructor<Concept>& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Constructor<Concept>& constructor) const override;

    size_t get_id() const override;
    const ConceptChoice& get_concept_left() const;
    const ConceptChoice& get_concept_right() const;
};

/**
 * Roles
 */

template<PredicateCategory P>
class RolePredicateState : public Constructor<Role>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    RolePredicateState(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    RolePredicateState(const RolePredicateState& other) = delete;
    RolePredicateState& operator=(const RolePredicateState& other) = delete;
    RolePredicateState(RolePredicateState&& other) = default;
    RolePredicateState& operator=(RolePredicateState&& other) = default;

    bool operator==(const RolePredicateState& other) const;
    bool is_equal(const Constructor<Role>& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Constructor<Role>& constructor) const override;

    size_t get_id() const override;
    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class RolePredicateGoal : public Constructor<Role>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    RolePredicateGoal(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    RolePredicateGoal(const RolePredicateGoal& other) = delete;
    RolePredicateGoal& operator=(const RolePredicateGoal& other) = delete;
    RolePredicateGoal(RolePredicateGoal&& other) = default;
    RolePredicateGoal& operator=(RolePredicateGoal&& other) = default;

    bool operator==(const RolePredicateGoal& other) const;
    bool is_equal(const Constructor<Role>& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Constructor<Role>& constructor) const override;

    size_t get_id() const override;
    Predicate<P> get_predicate() const;
};

class RoleAnd : public Constructor<Role>
{
private:
    size_t m_id;
    const RoleChoice& m_role_left;
    const RoleChoice& m_role_right;

    RoleAnd(size_t id, const RoleChoice& role_left, const RoleChoice& role_right);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    RoleAnd(const RoleAnd& other) = delete;
    RoleAnd& operator=(const RoleAnd& other) = delete;
    RoleAnd(RoleAnd&& other) = default;
    RoleAnd& operator=(RoleAnd&& other) = default;

    bool operator==(const RoleAnd& other) const;
    bool is_equal(const Constructor<Role>& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Constructor<Role>& constructor) const override;

    size_t get_id() const override;
    const RoleChoice& get_role_left() const;
    const RoleChoice& get_role_right() const;
};

/**
 * Implementations
 */

/**
 * NonTerminal
 */

template<dl::IsConceptOrRole D>
NonTerminal<D>::NonTerminal(size_t id, std::string name) : m_id(id), m_name(std::move(name)), m_rule(std::nullopt)
{
}

template<dl::IsConceptOrRole D>
bool NonTerminal<D>::operator==(const NonTerminal& other) const
{
    if (this != &other)
    {
        return (m_name == other.m_name);
    }
    return true;
}

template<dl::IsConceptOrRole D>
size_t NonTerminal<D>::hash() const
{
    return std::hash<std::string>()(m_name);
}

template<dl::IsConceptOrRole D>
bool NonTerminal<D>::test_match(const dl::Constructor<D>& constructor) const
{
    assert(m_rule.has_value());
    return m_rule.value().get().test_match(constructor);
}

template<dl::IsConceptOrRole D>
size_t NonTerminal<D>::get_id() const
{
    return m_id;
}

template<dl::IsConceptOrRole D>
const std::string& NonTerminal<D>::get_name() const
{
    return m_name;
}

template<dl::IsConceptOrRole D>
const DerivationRule<D>& NonTerminal<D>::get_rule() const
{
    assert(m_rule.has_value());
    return m_rule.value();
}

template<dl::IsConceptOrRole D>
void NonTerminal<D>::set_rule(const DerivationRule<D>& rule) const
{
    m_rule = rule;
}

/**
 * Choice
 */

template<dl::IsConceptOrRole D>
Choice<D>::Choice(size_t id, ConstructorOrNonTerminalChoice<D> choice) : m_id(id), m_choice(std::move(choice))
{
}

template<dl::IsConceptOrRole D>
bool Choice<D>::operator==(const Choice& other) const
{
    if (this != &other)
    {
        return std::visit(
            [](const auto& lhs, const auto& rhs) -> bool
            {
                using LHS = std::decay_t<decltype(lhs)>;
                using RHS = std::decay_t<decltype(rhs)>;
                if constexpr (std::is_same_v<LHS, RHS>)
                {
                    return &lhs.get() == &rhs.get();
                }
                else
                {
                    return false;
                }
            },
            m_choice,
            other.m_choice);
    }
    return true;
}

template<dl::IsConceptOrRole D>
size_t Choice<D>::hash() const
{
    return std::visit([](const auto& arg) -> size_t { return loki::hash_combine(&arg.get()); }, m_choice);
}

template<dl::IsConceptOrRole D>
bool Choice<D>::test_match(const dl::Constructor<D>& constructor) const
{
    return std::visit([&constructor](const auto& arg) -> bool { return arg.get().test_match(constructor); }, m_choice);
}

template<dl::IsConceptOrRole D>
size_t Choice<D>::get_id() const
{
    return m_id;
}

template<dl::IsConceptOrRole D>
const ConstructorOrNonTerminalChoice<D>& Choice<D>::get_choice() const
{
    return m_choice;
}

/**
 * DerivationRule
 */

template<dl::IsConceptOrRole D>
DerivationRule<D>::DerivationRule(size_t id, ChoiceList<D> choices) : m_id(id), m_choices(std::move(choices))
{
}

template<dl::IsConceptOrRole D>
bool DerivationRule<D>::operator==(const DerivationRule& other) const
{
    if (this != &other)
    {
        if (m_choices.size() != other.m_choices.size())
        {
            return false;
        }
        for (size_t i = 0; i < m_choices.size(); ++i)
        {
            if (m_choices[i].get() != other.m_choices[i].get())
            {
                return false;
            }
        }
    }
    return true;
}

template<dl::IsConceptOrRole D>
size_t DerivationRule<D>::hash() const
{
    size_t seed = m_choices.size();
    std::for_each(m_choices.begin(), m_choices.end(), [&seed](const auto& choice) { loki::hash_combine(seed, &choice.get()); });
    return seed;
}

template<dl::IsConceptOrRole D>
bool DerivationRule<D>::test_match(const dl::Constructor<D>& constructor) const
{
    return std::any_of(m_choices.begin(), m_choices.end(), [&constructor](const auto& choice) { return choice.get().test_match(constructor); });
}

template<dl::IsConceptOrRole D>
size_t DerivationRule<D>::get_id() const
{
    return m_id;
}

template<dl::IsConceptOrRole D>
const ChoiceList<D>& DerivationRule<D>::get_choices() const
{
    return m_choices;
}

/**
 * ConceptPredicateState
 */

template<PredicateCategory P>
ConceptPredicateState<P>::ConceptPredicateState(size_t id, Predicate<P> predicate) : m_id(id), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool ConceptPredicateState<P>::operator==(const ConceptPredicateState& other) const
{
    if (this != &other)
    {
        return (m_predicate == other.m_predicate);
    }
    return true;
}

template<PredicateCategory P>
bool ConceptPredicateState<P>::is_equal(const Constructor<Concept>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const ConceptPredicateState<P>&>(other);
    return (*this == otherDerived);
}

template<PredicateCategory P>
size_t ConceptPredicateState<P>::hash() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
bool ConceptPredicateState<P>::test_match(const dl::Constructor<Concept>& constructor) const
{
    return constructor.accept(ConceptPredicateStateVisitor<P>(*this));
}

template<PredicateCategory P>
size_t ConceptPredicateState<P>::get_id() const
{
    return m_id;
}

template<PredicateCategory P>
Predicate<P> ConceptPredicateState<P>::get_predicate() const
{
    return m_predicate;
}

/**
 * ConceptPredicateGoal
 */

template<PredicateCategory P>
ConceptPredicateGoal<P>::ConceptPredicateGoal(size_t id, Predicate<P> predicate) : m_id(id), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool ConceptPredicateGoal<P>::operator==(const ConceptPredicateGoal& other) const
{
    if (this != &other)
    {
        return (m_predicate == other.m_predicate);
    }
    return true;
}

template<PredicateCategory P>
bool ConceptPredicateGoal<P>::is_equal(const Constructor<Concept>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const ConceptPredicateGoal<P>&>(other);
    return (*this == otherDerived);
}

template<PredicateCategory P>
size_t ConceptPredicateGoal<P>::hash() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
bool ConceptPredicateGoal<P>::test_match(const dl::Constructor<Concept>& constructor) const
{
    return constructor.accept(ConceptPredicateGoalVisitor<P>(*this));
}

template<PredicateCategory P>
size_t ConceptPredicateGoal<P>::get_id() const
{
    return m_id;
}

template<PredicateCategory P>
Predicate<P> ConceptPredicateGoal<P>::get_predicate() const
{
    return m_predicate;
}

/**
 * RolePredicateState
 */

template<PredicateCategory P>
RolePredicateState<P>::RolePredicateState(size_t id, Predicate<P> predicate) : m_id(id), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool RolePredicateState<P>::operator==(const RolePredicateState& other) const
{
    if (this != &other)
    {
        return (m_predicate == other.m_predicate);
    }
    return true;
}

template<PredicateCategory P>
bool RolePredicateState<P>::is_equal(const Constructor<Role>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const RolePredicateState<P>&>(other);
    return (*this == otherDerived);
}

template<PredicateCategory P>
size_t RolePredicateState<P>::hash() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
bool RolePredicateState<P>::test_match(const dl::Constructor<Role>& constructor) const
{
    return constructor.accept(RolePredicateStateVisitor<P>(*this));
}

template<PredicateCategory P>
size_t RolePredicateState<P>::get_id() const
{
    return m_id;
}

template<PredicateCategory P>
Predicate<P> RolePredicateState<P>::get_predicate() const
{
    return m_predicate;
}

/**
 * RolePredicateGoal
 */

template<PredicateCategory P>
RolePredicateGoal<P>::RolePredicateGoal(size_t id, Predicate<P> predicate) : m_id(id), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool RolePredicateGoal<P>::operator==(const RolePredicateGoal& other) const
{
    if (this != &other)
    {
        return (m_predicate == other.m_predicate);
    }
    return true;
}

template<PredicateCategory P>
bool RolePredicateGoal<P>::is_equal(const Constructor<Role>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const RolePredicateGoal<P>&>(other);
    return (*this == otherDerived);
}

template<PredicateCategory P>
size_t RolePredicateGoal<P>::hash() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
bool RolePredicateGoal<P>::test_match(const dl::Constructor<Role>& constructor) const
{
    return constructor.accept(RolePredicateGoalVisitor<P>(*this));
}

template<PredicateCategory P>
size_t RolePredicateGoal<P>::get_id() const
{
    return m_id;
}

template<PredicateCategory P>
Predicate<P> RolePredicateGoal<P>::get_predicate() const
{
    return m_predicate;
}

}
}

#endif
