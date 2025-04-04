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

Index& GroundActionImpl::get_index() { return m_index; }

Index GroundActionImpl::get_index() const { return m_index; }

Index& GroundActionImpl::get_action_index() { return m_action_index; }

Index GroundActionImpl::get_action_index() const { return m_action_index; }

FlatIndexList& GroundActionImpl::get_objects() { return m_object_indices; }

const FlatIndexList& GroundActionImpl::get_object_indices() const { return m_object_indices; }

GroundConjunctiveCondition& GroundActionImpl::get_conjunctive_condition() { return m_conjunctive_precondition; }

const GroundConjunctiveCondition& GroundActionImpl::get_conjunctive_condition() const { return m_conjunctive_precondition; }

GroundConjunctiveEffect& GroundActionImpl::get_conjunctive_effect() { return m_conjunctive_effect; }

const GroundConjunctiveEffect& GroundActionImpl::get_conjunctive_effect() const { return m_conjunctive_effect; }

GroundEffectConditionalList& GroundActionImpl::get_conditional_effects() { return m_conditional_effects; }

const GroundEffectConditionalList& GroundActionImpl::get_conditional_effects() const { return m_conditional_effects; }

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

    auto binding = formalism::ObjectList {};
    for (const auto object_index : action->get_object_indices())
    {
        binding.push_back(problem.get_repositories().get_object(object_index));
    }

    const auto& conjunctive_condition = action->get_conjunctive_condition();
    const auto& conjunctive_effect = action->get_conjunctive_effect();
    const auto& conditional_effects = action->get_conditional_effects();

    os << "Action("                                                                                         //
       << "index=" << action->get_index() << ", "                                                           //
       << "name=" << problem.get_repositories().get_action(action->get_action_index())->get_name() << ", "  //
       << "binding=" << binding << ", "                                                                     //
       << std::make_tuple(conjunctive_condition, std::cref(problem)) << ", "                                //
       << std::make_tuple(conjunctive_effect, std::cref(problem))                                           //
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
    const auto& [ground_action, problem, tag] = data;

    const auto action = problem.get_repositories().get_action(ground_action->get_action_index());
    os << "(" << action->get_name();
    // Only take objects w.r.t. to the original action parameters
    for (size_t i = 0; i < action->get_original_arity(); ++i)
    {
        os << " " << *problem.get_repositories().get_object(ground_action->get_object_indices()[i]);
    }
    os << ")";
    return os;
}
}