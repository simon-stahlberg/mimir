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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/constructor_base.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/evaluation_context.hpp"
#include "mimir/languages/description_logics/grammar_visitors_interface.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl
{

/**
 * Concepts
 */

template<PredicateCategory P>
class ConceptPredicateState : public ConstructorEvaluatorBase<Concept, ConceptPredicateState<P>>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    ConceptPredicateState(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class ConstructorRepository;

    bool is_equal_impl(const Constructor<Concept>& other) const;
    size_t hash_impl() const;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Concept>& visitor) const;

    size_t get_id_impl() const;

    friend class ConstructorEvaluatorBase<Concept, ConceptPredicateState<P>>;

public:
    // Users are not supposed to move these directly.
    ConceptPredicateState(const ConceptPredicateState& other) = delete;
    ConceptPredicateState& operator=(const ConceptPredicateState& other) = delete;
    ConceptPredicateState(ConceptPredicateState&& other) = default;
    ConceptPredicateState& operator=(ConceptPredicateState&& other) = default;

    bool operator==(const ConceptPredicateState& other) const;

    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class ConceptPredicateGoal : public ConstructorEvaluatorBase<Concept, ConceptPredicateGoal<P>>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    ConceptPredicateGoal(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class ConstructorRepository;

    bool is_equal_impl(const Constructor<Concept>& other) const;
    size_t hash_impl() const;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Concept>& visitor) const;

    size_t get_id_impl() const;

    friend class ConstructorEvaluatorBase<Concept, ConceptPredicateGoal<P>>;

public:
    // Users are not supposed to move these directly.
    ConceptPredicateGoal(const ConceptPredicateGoal& other) = delete;
    ConceptPredicateGoal& operator=(const ConceptPredicateGoal& other) = delete;
    ConceptPredicateGoal(ConceptPredicateGoal&& other) = default;
    ConceptPredicateGoal& operator=(ConceptPredicateGoal&& other) = default;

    bool operator==(const ConceptPredicateGoal& other) const;

    Predicate<P> get_predicate() const;
};

class ConceptAnd : public ConstructorEvaluatorBase<Concept, ConceptAnd>
{
private:
    size_t m_id;
    std::reference_wrapper<const Constructor<Concept>> m_concept_left;
    std::reference_wrapper<const Constructor<Concept>> m_concept_right;

    ConceptAnd(size_t id, const Constructor<Concept>& concept_left, const Constructor<Concept>& concept_right);

    template<typename T>
    friend class ConstructorRepository;

    bool is_equal_impl(const Constructor<Concept>& other) const;
    size_t hash_impl() const;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Concept>& visitor) const;

    size_t get_id_impl() const;

    friend class ConstructorEvaluatorBase<Concept, ConceptAnd>;

public:
    // Users are not supposed to move these directly.
    ConceptAnd(const ConceptAnd& other) = delete;
    ConceptAnd& operator=(const ConceptAnd& other) = delete;
    ConceptAnd(ConceptAnd&& other) = default;
    ConceptAnd& operator=(ConceptAnd&& other) = default;

    bool operator==(const ConceptAnd& other) const;

    const Constructor<Concept>& get_concept_left() const;
    const Constructor<Concept>& get_concept_right() const;
};

/**
 * Roles
 */

template<PredicateCategory P>
class RolePredicateState : public ConstructorEvaluatorBase<Role, RolePredicateState<P>>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    RolePredicateState(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class ConstructorRepository;

    bool is_equal_impl(const Constructor<Role>& other) const;
    size_t hash_impl() const;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Role>& visitor) const;

    size_t get_id_impl() const;

    friend class ConstructorEvaluatorBase<Role, RolePredicateState<P>>;

public:
    // Users are not supposed to move these directly.
    RolePredicateState(const RolePredicateState& other) = delete;
    RolePredicateState& operator=(const RolePredicateState& other) = delete;
    RolePredicateState(RolePredicateState&& other) = default;
    RolePredicateState& operator=(RolePredicateState&& other) = default;

    bool operator==(const RolePredicateState& other) const;

    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class RolePredicateGoal : public ConstructorEvaluatorBase<Role, RolePredicateGoal<P>>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    RolePredicateGoal(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class ConstructorRepository;

    bool is_equal_impl(const Constructor<Role>& other) const;
    size_t hash_impl() const;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Role>& visitor) const;

    size_t get_id_impl() const;

    friend class ConstructorEvaluatorBase<Role, RolePredicateGoal<P>>;

public:
    // Users are not supposed to move these directly.
    RolePredicateGoal(const RolePredicateGoal& other) = delete;
    RolePredicateGoal& operator=(const RolePredicateGoal& other) = delete;
    RolePredicateGoal(RolePredicateGoal&& other) = default;
    RolePredicateGoal& operator=(RolePredicateGoal&& other) = default;

    bool operator==(const RolePredicateGoal& other) const;

    Predicate<P> get_predicate() const;
};

class RoleAnd : public ConstructorEvaluatorBase<Role, RoleAnd>
{
private:
    size_t m_id;
    std::reference_wrapper<const Constructor<Role>> m_role_left;
    std::reference_wrapper<const Constructor<Role>> m_role_right;

    RoleAnd(size_t id, const Constructor<Role>& role_left, const Constructor<Role>& role_right);

    template<typename T>
    friend class ConstructorRepository;

    bool is_equal_impl(const Constructor<Role>& other) const;
    size_t hash_impl() const;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Role>& visitor) const;

    size_t get_id_impl() const;

    friend class ConstructorEvaluatorBase<Role, RoleAnd>;

public:
    // Users are not supposed to move these directly.
    RoleAnd(const RoleAnd& other) = delete;
    RoleAnd& operator=(const RoleAnd& other) = delete;
    RoleAnd(RoleAnd&& other) = default;
    RoleAnd& operator=(RoleAnd&& other) = default;

    bool operator==(const RoleAnd& other) const;

    const Constructor<Role>& get_role_left() const;
    const Constructor<Role>& get_role_right() const;
};

}

#endif
