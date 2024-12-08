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

#include "mimir/search/action.hpp"

#include "mimir/common/concepts.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/state.hpp"

#include <ostream>
#include <tuple>

size_t cista::storage::DerefStdHasher<mimir::GroundActionImpl>::operator()(const mimir::GroundActionImpl* ptr) const
{
    const auto action = ptr->get_action_index();
    const auto& objects = ptr->get_object_indices();
    return mimir::hash_combine(action, objects);
}

bool cista::storage::DerefStdEqualTo<mimir::GroundActionImpl>::operator()(const mimir::GroundActionImpl* lhs, const mimir::GroundActionImpl* rhs) const
{
    const auto action_left = lhs->get_action_index();
    const auto& objects_left = lhs->get_object_indices();
    const auto action_right = rhs->get_action_index();
    const auto& objects_right = rhs->get_object_indices();
    return (action_left == action_right) && (objects_left == objects_right);
}

namespace mimir
{

/* GroundConditionStrips */

template<PredicateTag P>
FlatBitset& GroundConditionStrips::get_positive_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_positive_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_positive_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_positive_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template FlatBitset& GroundConditionStrips::get_positive_precondition<Static>();
template FlatBitset& GroundConditionStrips::get_positive_precondition<Fluent>();
template FlatBitset& GroundConditionStrips::get_positive_precondition<Derived>();

template<PredicateTag P>
const FlatBitset& GroundConditionStrips::get_positive_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_positive_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_positive_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_positive_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatBitset& GroundConditionStrips::get_positive_precondition<Static>() const;
template const FlatBitset& GroundConditionStrips::get_positive_precondition<Fluent>() const;
template const FlatBitset& GroundConditionStrips::get_positive_precondition<Derived>() const;

template<PredicateTag P>
FlatBitset& GroundConditionStrips::get_negative_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_negative_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_negative_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_negative_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template FlatBitset& GroundConditionStrips::get_negative_precondition<Static>();
template FlatBitset& GroundConditionStrips::get_negative_precondition<Fluent>();
template FlatBitset& GroundConditionStrips::get_negative_precondition<Derived>();

template<PredicateTag P>
const FlatBitset& GroundConditionStrips::get_negative_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_negative_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_negative_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_negative_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatBitset& GroundConditionStrips::get_negative_precondition<Static>() const;
template const FlatBitset& GroundConditionStrips::get_negative_precondition<Fluent>() const;
template const FlatBitset& GroundConditionStrips::get_negative_precondition<Derived>() const;

template<PredicateTag P>
bool GroundConditionStrips::is_applicable(const FlatBitset& atoms) const
{
    return atoms.is_superseteq(get_positive_precondition<P>())  //
           && atoms.are_disjoint(get_negative_precondition<P>());
}

template bool GroundConditionStrips::is_applicable<Static>(const FlatBitset& atoms) const;
template bool GroundConditionStrips::is_applicable<Fluent>(const FlatBitset& atoms) const;
template bool GroundConditionStrips::is_applicable<Derived>(const FlatBitset& atoms) const;

template<DynamicPredicateTag P>
bool GroundConditionStrips::is_applicable(State state) const
{
    return is_applicable<P>(state->get_atoms<P>());
}

template bool GroundConditionStrips::is_applicable<Fluent>(State state) const;
template bool GroundConditionStrips::is_applicable<Derived>(State state) const;

bool GroundConditionStrips::is_dynamically_applicable(State state) const
{  //
    return is_applicable<Fluent>(state) && is_applicable<Derived>(state);
}

bool GroundConditionStrips::is_statically_applicable(const FlatBitset& static_positive_atoms) const { return is_applicable<Static>(static_positive_atoms); }

bool GroundConditionStrips::is_applicable(Problem problem, State state) const
{
    return is_dynamically_applicable(state) && is_statically_applicable(problem->get_static_initial_positive_atoms_bitset());
}

/* GroundEffectStrips */

FlatBitset& GroundEffectStrips::get_positive_effects() { return m_positive_effects; }

const FlatBitset& GroundEffectStrips::get_positive_effects() const { return m_positive_effects; }

FlatBitset& GroundEffectStrips::get_negative_effects() { return m_negative_effects; }

const FlatBitset& GroundEffectStrips::get_negative_effects() const { return m_negative_effects; }

ContinuousCost& GroundEffectStrips::get_cost() { return m_cost; }

const ContinuousCost& GroundEffectStrips::get_cost() const { return m_cost; }

/* GroundEffectConditional */

template<PredicateTag P>
FlatIndexList& GroundEffectConditional::get_positive_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_positive_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_positive_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_positive_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template FlatIndexList& GroundEffectConditional::get_positive_precondition<Static>();
template FlatIndexList& GroundEffectConditional::get_positive_precondition<Fluent>();
template FlatIndexList& GroundEffectConditional::get_positive_precondition<Derived>();

template<PredicateTag P>
const FlatIndexList& GroundEffectConditional::get_positive_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_positive_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_positive_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_positive_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatIndexList& GroundEffectConditional::get_positive_precondition<Static>() const;
template const FlatIndexList& GroundEffectConditional::get_positive_precondition<Fluent>() const;
template const FlatIndexList& GroundEffectConditional::get_positive_precondition<Derived>() const;

template<PredicateTag P>
FlatIndexList& GroundEffectConditional::get_negative_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_negative_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_negative_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_negative_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template FlatIndexList& GroundEffectConditional::get_negative_precondition<Static>();
template FlatIndexList& GroundEffectConditional::get_negative_precondition<Fluent>();
template FlatIndexList& GroundEffectConditional::get_negative_precondition<Derived>();

template<PredicateTag P>
const FlatIndexList& GroundEffectConditional::get_negative_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_negative_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_negative_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_negative_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatIndexList& GroundEffectConditional::get_negative_precondition<Static>() const;
template const FlatIndexList& GroundEffectConditional::get_negative_precondition<Fluent>() const;
template const FlatIndexList& GroundEffectConditional::get_negative_precondition<Derived>() const;

GroundEffectFluentLiteralList& GroundEffectConditional::get_fluent_effect_literals() { return m_effects; }

const GroundEffectFluentLiteralList& GroundEffectConditional::get_fluent_effect_literals() const { return m_effects; }

ContinuousCost& GroundEffectConditional::get_cost() { return m_cost; }

const ContinuousCost& GroundEffectConditional::get_cost() const { return m_cost; }

template<DynamicPredicateTag P>
bool GroundEffectConditional::is_applicable(State state) const
{
    const auto& state_atoms = state->get_atoms<P>();

    return is_superseteq(state_atoms, get_positive_precondition<P>())  //
           && are_disjoint(state_atoms, get_negative_precondition<P>());
}

template bool GroundEffectConditional::is_applicable<Fluent>(State state) const;
template bool GroundEffectConditional::is_applicable<Derived>(State state) const;

bool GroundEffectConditional::is_dynamically_applicable(State state) const
{  //
    return is_applicable<Fluent>(state) && is_applicable<Derived>(state);
}

bool GroundEffectConditional::is_statically_applicable(Problem problem) const
{
    const auto& static_initial_atoms = problem->get_static_initial_positive_atoms_bitset();

    return is_superseteq(static_initial_atoms, get_positive_precondition<Static>())  //
           && are_disjoint(static_initial_atoms, get_negative_precondition<Static>());
}

bool GroundEffectConditional::is_applicable(Problem problem, State state) const
{
    return is_dynamically_applicable(state) && is_statically_applicable(problem);
}

/* GroundActionImpl */

Index& GroundActionImpl::get_index() { return m_index; }

Index GroundActionImpl::get_index() const { return m_index; }

Index& GroundActionImpl::get_action_index() { return m_action_index; }

Index GroundActionImpl::get_action_index() const { return m_action_index; }

FlatIndexList& GroundActionImpl::get_objects() { return m_objects; }

const FlatIndexList& GroundActionImpl::get_object_indices() const { return m_objects; }

GroundConditionStrips& GroundActionImpl::get_strips_precondition() { return m_strips_precondition; }

const GroundConditionStrips& GroundActionImpl::get_strips_precondition() const { return m_strips_precondition; }

GroundEffectStrips& GroundActionImpl::get_strips_effect() { return m_strips_effect; }

const GroundEffectStrips& GroundActionImpl::get_strips_effect() const { return m_strips_effect; }

GroundEffectConditionalList& GroundActionImpl::get_conditional_effects() { return m_conditional_effects; }

const GroundEffectConditionalList& GroundActionImpl::get_conditional_effects() const { return m_conditional_effects; }

bool GroundActionImpl::is_dynamically_applicable(State state) const { return get_strips_precondition().is_dynamically_applicable(state); }

bool GroundActionImpl::is_statically_applicable(const FlatBitset& static_positive_atoms) const
{
    return get_strips_precondition().is_statically_applicable(static_positive_atoms);
}

bool GroundActionImpl::is_applicable(Problem problem, State state) const { return get_strips_precondition().is_applicable(problem, state); }

template<PredicateTag P>
bool GroundActionImpl::is_applicable(const FlatBitset& atoms) const
{
    return get_strips_precondition().template is_applicable<P>(atoms);
}

template bool GroundActionImpl::is_applicable<Static>(const FlatBitset& atoms) const;
template bool GroundActionImpl::is_applicable<Fluent>(const FlatBitset& atoms) const;
template bool GroundActionImpl::is_applicable<Derived>(const FlatBitset& atoms) const;

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundEffectFluentLiteral, const PDDLRepositories&>& data)
{
    const auto& [simple_effect, pddl_repositories] = data;

    const auto& ground_atom = pddl_repositories.get_ground_atom<Fluent>(simple_effect.atom_index);

    if (simple_effect.is_negated)
    {
        os << "(not ";
    }

    os << *ground_atom;

    if (simple_effect.is_negated)
    {
        os << ")";
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const std::tuple<GroundEffectFluentLiteralList, const PDDLRepositories&>& data)
{
    const auto& [simple_effects, pddl_repositories] = data;

    os << "(and";

    for (const auto& simple_effect : simple_effects)
    {
        os << " " << std::make_tuple(simple_effect, std::cref(pddl_repositories));
    }

    os << ")";

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundConditionStrips, const PDDLRepositories&>& data)
{
    const auto& [strips_precondition_proxy, pddl_repositories] = data;

    const auto& positive_static_precondition_bitset = strips_precondition_proxy.get_positive_precondition<Static>();
    const auto& negative_static_precondition_bitset = strips_precondition_proxy.get_negative_precondition<Static>();
    const auto& positive_fluent_precondition_bitset = strips_precondition_proxy.get_positive_precondition<Fluent>();
    const auto& negative_fluent_precondition_bitset = strips_precondition_proxy.get_negative_precondition<Fluent>();
    const auto& positive_derived_precondition_bitset = strips_precondition_proxy.get_positive_precondition<Derived>();
    const auto& negative_derived_precondition_bitset = strips_precondition_proxy.get_negative_precondition<Derived>();

    auto positive_static_precondition = GroundAtomList<Static> {};
    auto negative_static_precondition = GroundAtomList<Static> {};
    auto positive_fluent_precondition = GroundAtomList<Fluent> {};
    auto negative_fluent_precondition = GroundAtomList<Fluent> {};
    auto positive_derived_precondition = GroundAtomList<Derived> {};
    auto negative_derived_precondition = GroundAtomList<Derived> {};

    pddl_repositories.get_ground_atoms_from_indices<Static>(positive_static_precondition_bitset, positive_static_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Static>(negative_static_precondition_bitset, negative_static_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(positive_fluent_precondition_bitset, positive_fluent_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(negative_fluent_precondition_bitset, negative_fluent_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Derived>(positive_derived_precondition_bitset, positive_derived_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Derived>(negative_derived_precondition_bitset, negative_derived_precondition);

    os << "positive static precondition=" << positive_static_precondition << ", "
       << "negative static precondition=" << negative_static_precondition << ", "
       << "positive fluent precondition=" << positive_fluent_precondition << ", "
       << "negative fluent precondition=" << negative_fluent_precondition << ", "
       << "positive derived precondition=" << positive_derived_precondition << ", "
       << "negative derived precondition=" << negative_derived_precondition;

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundEffectStrips, const PDDLRepositories&>& data)
{
    const auto& [strips_effect_proxy, pddl_repositories] = data;

    const auto& positive_effect_bitset = strips_effect_proxy.get_positive_effects();
    const auto& negative_effect_bitset = strips_effect_proxy.get_negative_effects();

    auto positive_simple_effects = GroundAtomList<Fluent> {};
    auto negative_simple_effects = GroundAtomList<Fluent> {};

    pddl_repositories.get_ground_atoms_from_indices<Fluent>(positive_effect_bitset, positive_simple_effects);
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(negative_effect_bitset, negative_simple_effects);

    os << "delete effects=" << negative_simple_effects << ", "
       << "add effects=" << positive_simple_effects;

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundEffectConditional, const PDDLRepositories&>& data)
{
    const auto& [cond_effect_proxy, pddl_repositories] = data;

    const auto& positive_static_precondition_bitset = cond_effect_proxy.get_positive_precondition<Static>();
    const auto& negative_static_precondition_bitset = cond_effect_proxy.get_negative_precondition<Static>();
    const auto& positive_fluent_precondition_bitset = cond_effect_proxy.get_positive_precondition<Fluent>();
    const auto& negative_fluent_precondition_bitset = cond_effect_proxy.get_negative_precondition<Fluent>();
    const auto& positive_derived_precondition_bitset = cond_effect_proxy.get_positive_precondition<Derived>();
    const auto& negative_derived_precondition_bitset = cond_effect_proxy.get_negative_precondition<Derived>();
    const auto& simple_effect = cond_effect_proxy.get_fluent_effect_literals();

    auto positive_static_precondition = GroundAtomList<Static> {};
    auto negative_static_precondition = GroundAtomList<Static> {};
    auto positive_fluent_precondition = GroundAtomList<Fluent> {};
    auto negative_fluent_precondition = GroundAtomList<Fluent> {};
    auto positive_derived_precondition = GroundAtomList<Derived> {};
    auto negative_derived_precondition = GroundAtomList<Derived> {};

    pddl_repositories.get_ground_atoms_from_indices<Static>(positive_static_precondition_bitset, positive_static_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Static>(negative_static_precondition_bitset, negative_static_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(positive_fluent_precondition_bitset, positive_fluent_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(negative_fluent_precondition_bitset, negative_fluent_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Derived>(positive_derived_precondition_bitset, positive_derived_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Derived>(negative_derived_precondition_bitset, negative_derived_precondition);

    os << "positive static precondition=" << positive_static_precondition << ", "
       << "negative static precondition=" << negative_static_precondition << ", "
       << "positive fluent precondition=" << positive_fluent_precondition << ", "
       << "negative fluent precondition=" << negative_fluent_precondition << ", "
       << "positive derived precondition=" << positive_derived_precondition << ", "
       << "negative derived precondition=" << negative_derived_precondition << ", "
       << "effect=" << std::make_tuple(simple_effect, std::cref(pddl_repositories));

    return os;
}

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
       << ", "
       << "conditional_effects=[";
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
