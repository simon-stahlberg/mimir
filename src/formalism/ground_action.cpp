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

#include "mimir/common/concepts.hpp"
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
                                   GroundConjunctiveEffect conjunctive_effect,
                                   GroundConditionalEffectList conditional_effects) :
    m_index(index),
    m_action(action),
    m_objects(std::move(objects)),
    m_conjunctive_precondition(conjunctive_precondition),
    m_conjunctive_effect(conjunctive_effect),
    m_conditional_effects(std::move(conditional_effects))
{
}

Index GroundActionImpl::get_index() const { return m_index; }

Action GroundActionImpl::get_action() const { return m_action; }

const ObjectList& GroundActionImpl::get_objects() const { return m_objects; }

GroundConjunctiveCondition GroundActionImpl::get_conjunctive_condition() const { return m_conjunctive_precondition; }

GroundConjunctiveEffect GroundActionImpl::get_conjunctive_effect() const { return m_conjunctive_effect; }

const GroundConditionalEffectList& GroundActionImpl::get_conditional_effects() const { return m_conditional_effects; }

}

namespace mimir
{

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os,
                         const std::tuple<formalism::GroundAction, const formalism::ProblemImpl&, formalism::GroundActionImpl::FullFormatterTag>& data)
{
    const auto& [action, problem, tag] = data;

    const auto& conjunctive_condition = action->get_conjunctive_condition();
    const auto& conjunctive_effect = action->get_conjunctive_effect();
    const auto& conditional_effects = action->get_conditional_effects();

    os << "Action("                                                           //
       << "index=" << action->get_index() << ", "                             //
       << "name=" << action->get_action()->get_name() << ", "                 //
       << "binding=" << action->get_objects() << ", "                         //
       << std::make_tuple(conjunctive_condition, std::cref(problem)) << ", "  //
       << std::make_tuple(conjunctive_effect, std::cref(problem))             //
       << ", " << "conditional_effects=[";
    for (const auto& cond_effect : conditional_effects)
    {
        os << "[" << std::make_tuple(cond_effect, std::cref(problem)) << "], ";
    }
    os << "])";

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os,
                         const std::tuple<formalism::GroundAction, const formalism::ProblemImpl&, formalism::GroundActionImpl::PlanFormatterTag>& data)
{
    const auto& [action, problem, tag] = data;
    ;
    os << "(" << action->get_action()->get_name();
    // Only take objects w.r.t. to the original action parameters
    for (auto it = action->get_objects().begin(); it != action->get_objects().begin() + action->get_action()->get_original_arity(); ++it)
    {
        os << " " << **it;
    }
    os << ")";
    return os;
}
}