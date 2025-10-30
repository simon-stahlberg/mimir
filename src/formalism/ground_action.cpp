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

#include "mimir/formalism/ground_action.hpp"

#include "mimir/common/declarations.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/problem.hpp"

#include <ostream>
#include <tuple>

namespace mimir::formalism
{

/* GroundActionImpl */

GroundActionImpl::GroundActionImpl(Index index,
                                   Action action,
                                   ObjectList objects,
                                   GroundConjunctiveCondition conjunctive_precondition,
                                   GroundConditionalEffectList conditional_effects) :
    m_index(index),
    m_action(action),
    m_objects(std::move(objects)),
    m_conjunctive_precondition(conjunctive_precondition),
    m_conditional_effects(std::move(conditional_effects))
{
}

Index GroundActionImpl::get_index() const { return m_index; }

Action GroundActionImpl::get_action() const { return m_action; }

const ObjectList& GroundActionImpl::get_objects() const { return m_objects; }

GroundConjunctiveCondition GroundActionImpl::get_conjunctive_condition() const { return m_conjunctive_precondition; }

const GroundConditionalEffectList& GroundActionImpl::get_conditional_effects() const { return m_conditional_effects; }

}
