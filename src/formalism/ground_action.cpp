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
#include "mimir/common/hash_cista.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/dense_state.hpp"
#include "mimir/search/state.hpp"

#include <ostream>
#include <tuple>

namespace mimir
{

/* GroundActionImpl */

Index& GroundActionImpl::get_index() { return m_index; }

Index GroundActionImpl::get_index() const { return m_index; }

Index& GroundActionImpl::get_action_index() { return m_action_index; }

Index GroundActionImpl::get_action_index() const { return m_action_index; }

FlatIndexList& GroundActionImpl::get_objects() { return m_objects; }

const FlatIndexList& GroundActionImpl::get_object_indices() const { return m_objects; }

GroundConjunctiveCondition& GroundActionImpl::get_strips_precondition() { return m_strips_precondition; }

const GroundConjunctiveCondition& GroundActionImpl::get_strips_precondition() const { return m_strips_precondition; }

GroundEffectStrips& GroundActionImpl::get_strips_effect() { return m_strips_effect; }

const GroundEffectStrips& GroundActionImpl::get_strips_effect() const { return m_strips_effect; }

GroundEffectConditionalList& GroundActionImpl::get_conditional_effects() { return m_conditional_effects; }

const GroundEffectConditionalList& GroundActionImpl::get_conditional_effects() const { return m_conditional_effects; }

bool GroundActionImpl::is_dynamically_applicable(const DenseState& dense_state) const
{
    return get_strips_precondition().is_dynamically_applicable(dense_state);
}

bool GroundActionImpl::is_statically_applicable(const FlatBitset& static_positive_atoms) const
{
    return get_strips_precondition().is_statically_applicable(static_positive_atoms);
}

bool GroundActionImpl::is_applicable(Problem problem, const DenseState& dense_state) const
{
    return get_strips_precondition().is_applicable(problem, dense_state);
}

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAction, const PDDLRepositories&, FullActionFormatterTag>& data)
{
    const auto& [action, pddl_repositories, tag] = data;

    auto binding = ObjectList {};
    for (const auto object_index : action->get_object_indices())
    {
        binding.push_back(pddl_repositories.get_object(object_index));
    }

    const auto& strips_precondition = action->get_strips_precondition();
    const auto& strips_effect = action->get_strips_effect();
    const auto& cond_effects = action->get_conditional_effects();

    os << "Action("                                                                                //
       << "index=" << action->get_index() << ", "                                                  //
       << "name=" << pddl_repositories.get_action(action->get_action_index())->get_name() << ", "  //
       << "binding=" << binding << ", "                                                            //
       << std::make_tuple(strips_precondition, std::cref(pddl_repositories)) << ", "               //
       << std::make_tuple(strips_effect, std::cref(pddl_repositories))                             //
       << ", " << "conditional_effects=[";
    for (const auto& cond_effect : cond_effects)
    {
        os << "[" << std::make_tuple(cond_effect, std::cref(pddl_repositories)) << "], ";
    }
    os << "])";

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAction, const PDDLRepositories&, PlanActionFormatterTag>& data)
{
    const auto& [ground_action, pddl_repositories, tag] = data;

    const auto action = pddl_repositories.get_action(ground_action->get_action_index());
    os << "(" << action->get_name();
    // Only take objects w.r.t. to the original action parameters
    for (size_t i = 0; i < action->get_original_arity(); ++i)
    {
        os << " " << *pddl_repositories.get_object(ground_action->get_object_indices()[i]);
    }
    os << ")";
    return os;
}
}
