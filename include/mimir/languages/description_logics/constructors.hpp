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

#include "mimir/languages/description_logics/constructors_interface.hpp"

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

    bool accept(const ConceptVisitor& visitor) const override;
};

class ConceptAnd : public Concept
{
private:
    const Concept* m_concept_left;
    const Concept* m_concept_right;

public:
    ConceptAnd(size_t id, const Concept* concept_left, const Concept* concept_right);

    void evaluate(EvaluationContext& context) const override;

    bool accept(const ConceptVisitor& visitor) const override;
};

/**
 * Roles
 */

/**
 * Implementations
 */

/* ConceptPredicate */
template<PredicateCategory P>
ConceptPredicate<P>::ConceptPredicate(Predicate<P> predicate) : m_predicate(predicate)
{
}

template<PredicateCategory P>
void ConceptPredicate<P>::evaluate(EvaluationContext& context) const
{
    // TODO
}

template<PredicateCategory P>
bool ConceptPredicate<P>::accept(const ConceptVisitor& visitor) const
{
    return visitor.accept(*this);
}

}

#endif
