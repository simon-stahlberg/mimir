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

#include <mimir/languages/description_logics/constructors.hpp>

namespace mimir::dl
{

/**
 * ConceptAnd
 */
ConceptAnd::ConceptAnd(size_t id, const Concept* concept_left, const Concept* concept_right) :
    Concept(id),
    m_concept_left(concept_left),
    m_concept_right(concept_right)
{
}

void ConceptAnd::evaluate(EvaluationContext& context) const
{
    // TODO
}

bool ConceptAnd::accept(const ConceptVisitor& visitor) const { return visitor.accept(*this); }
}
