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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONCEPTS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONCEPTS_HPP_

#include <concepts>
#include <cstddef>

namespace mimir::dl
{

struct EvaluationContext;

template<typename T>
concept IsDLConstructor = requires(const T a, EvaluationContext& context) {
    {
        a.evaluate(context)
    };
};

/**
 * Concepts
 */

class Concept
{
protected:
    size_t m_identifier;

public:
    virtual ~Concept() {}

    virtual void evaluate(EvaluationContext& context) const = 0;
};

class PrimitiveConcept : public Concept
{
private:
    // TODO: add predicate

public:
    PrimitiveConcept(/* TODO add predicate */);

    void evaluate(EvaluationContext& context) const override {}
};

class AllConcept : public Concept
{
private:
    const Role* m_role;
    const Concept* m_concept;

public:
    AllConcept(const Role* role, const Concept* concept_);

    void evaluate(EvaluationContext& context) const override {}
};

class SomeConcept : public Concept
{
private:
    const Role* m_role;
    const Concept* m_concept;

public:
    SomeConcept(const Role* role, const Concept* concept_);

    void evaluate(EvaluationContext& context) const override {}
};

/**
 * Roles
 */

class Role
{
protected:
    size_t m_identifier;

public:
    virtual ~Role() {}

    virtual void evaluate(EvaluationContext& context) const = 0;
};

}

#endif
