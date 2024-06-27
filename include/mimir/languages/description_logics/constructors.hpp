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

#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructors_interface.hpp"
#include "mimir/languages/description_logics/visitors_interface.hpp"

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
class ConceptPredicate : public Concept
{
private:
    Predicate<P> m_predicate;

public:
    ConceptPredicate(Predicate<P> predicate);

    void evaluate(EvaluationContext& context) const override;

    bool accept(const ConceptVisitor& symbols) const override;
};

class ConceptAll : public Concept
{
private:
    const Role* m_role;
    const Concept* m_concept;

public:
    ConceptAll(const Role* role, const Concept* concept_);

    void evaluate(EvaluationContext& context) const override;

    bool accept(const ConceptVisitor& symbols) const override;
};

/**
 * Static asserts
 */

static_assert(IsDLConstructor<ConceptPredicate<Static>>);
static_assert(IsDLConstructor<ConceptPredicate<Fluent>>);
static_assert(IsDLConstructor<ConceptPredicate<Derived>>);
static_assert(IsDLConstructor<ConceptAll>);

/**
 * Roles
 */

}

#endif
