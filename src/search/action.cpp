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
#include "mimir/common/printers.hpp"
#include "mimir/formalism/factories.hpp"

#include <ostream>
#include <tuple>

namespace mimir
{

/* FlatSimpleEffect*/

bool FlatSimpleEffect::operator==(const FlatSimpleEffect& other) const
{
    if (this != &other)
    {
        return is_negated == other.is_negated && atom_id == other.atom_id;
    }
    return true;
}

/* FlatActionHash */

size_t FlatActionHash::operator()(FlatAction view) const
{
    const auto action = view.get<2>();
    const auto objects = view.get<3>();
    return loki::hash_combine(action, objects.hash());
}

bool FlatActionEqual::operator()(FlatAction view_left, FlatAction view_right) const
{
    const auto action_left = view_left.get<2>();
    const auto objects_left = view_left.get<3>();
    const auto action_right = view_right.get<2>();
    const auto objects_right = view_right.get<3>();
    return (action_left == action_right) && (objects_left == objects_right);
}

/* StripsActionPreconditionBuilderProxy */
StripsActionPreconditionBuilderProxy::StripsActionPreconditionBuilderProxy(FlatStripsActionPreconditionBuilder& builder) : m_builder(builder) {}

template<PredicateCategory P>
FlatBitsetBuilder<P>& StripsActionPreconditionBuilderProxy::get_positive_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_builder.get<0>();
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_builder.get<2>();
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_builder.get<4>();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatBitsetBuilder<Static>& StripsActionPreconditionBuilderProxy::get_positive_precondition<Static>();
template FlatBitsetBuilder<Fluent>& StripsActionPreconditionBuilderProxy::get_positive_precondition<Fluent>();
template FlatBitsetBuilder<Derived>& StripsActionPreconditionBuilderProxy::get_positive_precondition<Derived>();

template<PredicateCategory P>
FlatBitsetBuilder<P>& StripsActionPreconditionBuilderProxy::get_negative_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_builder.get<1>();
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_builder.get<3>();
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_builder.get<5>();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatBitsetBuilder<Static>& StripsActionPreconditionBuilderProxy::get_negative_precondition<Static>();
template FlatBitsetBuilder<Fluent>& StripsActionPreconditionBuilderProxy::get_negative_precondition<Fluent>();
template FlatBitsetBuilder<Derived>& StripsActionPreconditionBuilderProxy::get_negative_precondition<Derived>();

/* StripsActionPrecondition */
StripsActionPrecondition::StripsActionPrecondition(FlatStripsActionPrecondition view) : m_view(view) {}

template<PredicateCategory P>
FlatBitset<P> StripsActionPrecondition::get_positive_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_view.get<0>();
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_view.get<2>();
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_view.get<4>();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatBitset<Static> StripsActionPrecondition::get_positive_precondition<Static>() const;
template FlatBitset<Fluent> StripsActionPrecondition::get_positive_precondition<Fluent>() const;
template FlatBitset<Derived> StripsActionPrecondition::get_positive_precondition<Derived>() const;

template<PredicateCategory P>
FlatBitset<P> StripsActionPrecondition::get_negative_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_view.get<1>();
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_view.get<3>();
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_view.get<5>();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatBitset<Static> StripsActionPrecondition::get_negative_precondition<Static>() const;
template FlatBitset<Fluent> StripsActionPrecondition::get_negative_precondition<Fluent>() const;
template FlatBitset<Derived> StripsActionPrecondition::get_negative_precondition<Derived>() const;

template<DynamicPredicateCategory P>
bool StripsActionPrecondition::is_applicable(State state) const
{
    const auto state_atoms = state.get_atoms<P>();

    return state_atoms.is_superseteq(get_positive_precondition<P>())  //
           && state_atoms.are_disjoint(get_negative_precondition<P>());
}

template bool StripsActionPrecondition::is_applicable<Fluent>(State state) const;
template bool StripsActionPrecondition::is_applicable<Derived>(State state) const;

bool StripsActionPrecondition::is_dynamically_applicable(State state) const
{  //
    return is_applicable<Fluent>(state) && is_applicable<Derived>(state);
}

bool StripsActionPrecondition::is_statically_applicable(FlatBitset<Static> static_positive_atoms) const { return is_applicable<Static>(static_positive_atoms); }

bool StripsActionPrecondition::is_applicable(Problem problem, State state) const
{
    return is_dynamically_applicable(state) && is_statically_applicable(problem->get_static_initial_positive_atoms_bitset());
}

template<PredicateCategory P>
bool StripsActionPrecondition::is_applicable(const FlatBitsetBuilder<P>& atoms) const
{
    return atoms.is_superseteq(get_positive_precondition<P>())  //
           && atoms.are_disjoint(get_negative_precondition<P>());
}

template bool StripsActionPrecondition::is_applicable(const FlatBitsetBuilder<Static>& atoms) const;
template bool StripsActionPrecondition::is_applicable(const FlatBitsetBuilder<Fluent>& atoms) const;
template bool StripsActionPrecondition::is_applicable(const FlatBitsetBuilder<Derived>& atoms) const;

bool StripsActionPrecondition::is_applicable(const FlatBitsetBuilder<Fluent>& fluent_state_atoms,
                                             const FlatBitsetBuilder<Derived>& derived_state_atoms,
                                             const FlatBitsetBuilder<Static>& static_initial_atoms) const
{
    return is_applicable<Fluent>(fluent_state_atoms) && is_applicable<Static>(static_initial_atoms) && is_applicable<Derived>(derived_state_atoms);
}

/* StripsActionEffectBuilderProxy */

StripsActionEffectBuilderProxy::StripsActionEffectBuilderProxy(FlatStripsActionEffectBuilder& builder) : m_builder(builder) {}

FlatBitsetBuilder<Fluent>& StripsActionEffectBuilderProxy::get_positive_effects() { return m_builder.get<0>(); }

FlatBitsetBuilder<Fluent>& StripsActionEffectBuilderProxy::get_negative_effects() { return m_builder.get<1>(); }

/* StripsActionEffect */

StripsActionEffect::StripsActionEffect(FlatStripsActionEffect view) : m_view(view) {}

FlatBitset<Fluent> StripsActionEffect::get_positive_effects() const { return m_view.get<0>(); }
FlatBitset<Fluent> StripsActionEffect::get_negative_effects() const { return m_view.get<1>(); }

/* ConditionalEffectBuilderProxy */

ConditionalEffectBuilderProxy::ConditionalEffectBuilderProxy(FlatConditionalEffectBuilder& builder) : m_builder(builder) {}

template<PredicateCategory P>
FlatIndexListBuilder& ConditionalEffectBuilderProxy::get_positive_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_builder.get<0>();
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_builder.get<2>();
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_builder.get<4>();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatIndexListBuilder& ConditionalEffectBuilderProxy::get_positive_precondition<Static>();
template FlatIndexListBuilder& ConditionalEffectBuilderProxy::get_positive_precondition<Fluent>();
template FlatIndexListBuilder& ConditionalEffectBuilderProxy::get_positive_precondition<Derived>();

template<PredicateCategory P>
FlatIndexListBuilder& ConditionalEffectBuilderProxy::get_negative_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_builder.get<1>();
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_builder.get<3>();
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_builder.get<5>();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatIndexListBuilder& ConditionalEffectBuilderProxy::get_negative_precondition<Static>();
template FlatIndexListBuilder& ConditionalEffectBuilderProxy::get_negative_precondition<Fluent>();
template FlatIndexListBuilder& ConditionalEffectBuilderProxy::get_negative_precondition<Derived>();

FlatSimpleEffect& ConditionalEffectBuilderProxy::get_simple_effect() { return m_builder.get<6>(); }

/* ConditionalEffect */

ConditionalEffect::ConditionalEffect(FlatConditionalEffect view) : m_view(view) {}

template<PredicateCategory P>
FlatIndexList ConditionalEffect::get_positive_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_view.get<0>();
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_view.get<2>();
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_view.get<4>();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatIndexList ConditionalEffect::get_positive_precondition<Static>() const;
template FlatIndexList ConditionalEffect::get_positive_precondition<Fluent>() const;
template FlatIndexList ConditionalEffect::get_positive_precondition<Derived>() const;

template<PredicateCategory P>
FlatIndexList ConditionalEffect::get_negative_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_view.get<1>();
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_view.get<3>();
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_view.get<5>();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatIndexList ConditionalEffect::get_negative_precondition<Static>() const;
template FlatIndexList ConditionalEffect::get_negative_precondition<Fluent>() const;
template FlatIndexList ConditionalEffect::get_negative_precondition<Derived>() const;

const FlatSimpleEffect& ConditionalEffect::get_simple_effect() const { return m_view.get<6>(); }

template<DynamicPredicateCategory P>
bool ConditionalEffect::is_applicable(State state) const
{
    const auto state_atoms = state.get_atoms<P>();

    return is_superseteq(state_atoms, get_positive_precondition<P>())  //
           && are_disjoint(state_atoms, get_negative_precondition<P>());
}

template bool ConditionalEffect::is_applicable<Fluent>(State state) const;
template bool ConditionalEffect::is_applicable<Derived>(State state) const;

bool ConditionalEffect::is_dynamically_applicable(State state) const
{  //
    return is_applicable<Fluent>(state) && is_applicable<Derived>(state);
}

bool ConditionalEffect::is_statically_applicable(Problem problem) const
{
    const auto static_initial_atoms = problem->get_static_initial_positive_atoms_bitset();

    return is_superseteq(static_initial_atoms, get_positive_precondition<Static>())  //
           && are_disjoint(static_initial_atoms, get_negative_precondition<Static>());
}

bool ConditionalEffect::is_applicable(Problem problem, State state) const { return is_dynamically_applicable(state) && is_statically_applicable(problem); }

/* GroundActionBuilder */

FlatActionBuilder& GroundActionBuilder::get_flatmemory_builder() { return m_builder; }

const FlatActionBuilder& GroundActionBuilder::get_flatmemory_builder() const { return m_builder; }

uint32_t& GroundActionBuilder::get_id() { return m_builder.get<0>(); }

double& GroundActionBuilder::get_cost() { return m_builder.get<1>(); }

Action& GroundActionBuilder::get_action() { return m_builder.get<2>(); }

FlatObjectListBuilder& GroundActionBuilder::get_objects() { return m_builder.get<3>(); }

FlatStripsActionPreconditionBuilder& GroundActionBuilder::get_strips_precondition() { return m_builder.get<4>(); }

FlatStripsActionEffectBuilder& GroundActionBuilder::get_strips_effect() { return m_builder.get<5>(); }

FlatConditionalEffectsBuilder& GroundActionBuilder::get_conditional_effects() { return m_builder.get<6>(); }

/* GroundAction */

GroundAction::GroundAction(FlatAction view) : m_view(view) {}

size_t GroundAction::hash() const { return loki::hash_combine(m_view.buffer()); }

uint32_t GroundAction::get_id() const { return m_view.get<0>(); }

double GroundAction::get_cost() const { return m_view.get<1>(); }

Action GroundAction::get_action() const { return m_view.get<2>(); }

FlatObjectList GroundAction::get_objects() const { return m_view.get<3>(); }

bool GroundAction::operator==(GroundAction other) const { return m_view.buffer() == other.m_view.buffer(); }

FlatStripsActionPrecondition GroundAction::get_strips_precondition() const { return m_view.get<4>(); }

FlatStripsActionEffect GroundAction::get_strips_effect() const { return m_view.get<5>(); }

FlatConditionalEffects GroundAction::get_conditional_effects() const { return m_view.get<6>(); }

bool GroundAction::is_dynamically_applicable(State state) const
{  //
    return StripsActionPrecondition(get_strips_precondition()).is_dynamically_applicable(state);
}

bool GroundAction::is_statically_applicable(FlatBitset<Static> static_positive_atoms) const
{  //
    return StripsActionPrecondition(get_strips_precondition()).is_statically_applicable(static_positive_atoms);
}

bool GroundAction::is_applicable(Problem problem, State state) const
{  //
    return is_dynamically_applicable(state) && is_statically_applicable(problem->get_static_initial_positive_atoms_bitset());
}

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& os, const std::tuple<FlatSimpleEffect, const PDDLFactories&>& data)
{
    const auto [simple_effect, pddl_factories] = data;

    const auto& ground_atom = pddl_factories.get_ground_atom<Fluent>(simple_effect.atom_id);

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

std::ostream& operator<<(std::ostream& os, const std::tuple<StripsActionPrecondition, const PDDLFactories&>& data)
{
    const auto [strips_precondition_proxy, pddl_factories] = data;

    const auto positive_static_precondition_bitset = strips_precondition_proxy.get_positive_precondition<Static>();
    const auto negative_static_precondition_bitset = strips_precondition_proxy.get_negative_precondition<Static>();
    const auto positive_fluent_precondition_bitset = strips_precondition_proxy.get_positive_precondition<Fluent>();
    const auto negative_fluent_precondition_bitset = strips_precondition_proxy.get_negative_precondition<Fluent>();
    const auto positive_derived_precondition_bitset = strips_precondition_proxy.get_positive_precondition<Derived>();
    const auto negative_derived_precondition_bitset = strips_precondition_proxy.get_negative_precondition<Derived>();

    auto positive_static_precondition = GroundAtomList<Static> {};
    auto negative_static_precondition = GroundAtomList<Static> {};
    auto positive_fluent_precondition = GroundAtomList<Fluent> {};
    auto negative_fluent_precondition = GroundAtomList<Fluent> {};
    auto positive_derived_precondition = GroundAtomList<Derived> {};
    auto negative_derived_precondition = GroundAtomList<Derived> {};

    pddl_factories.get_ground_atoms_from_ids<Static>(positive_static_precondition_bitset, positive_static_precondition);
    pddl_factories.get_ground_atoms_from_ids<Static>(negative_static_precondition_bitset, negative_static_precondition);
    pddl_factories.get_ground_atoms_from_ids<Fluent>(positive_fluent_precondition_bitset, positive_fluent_precondition);
    pddl_factories.get_ground_atoms_from_ids<Fluent>(negative_fluent_precondition_bitset, negative_fluent_precondition);
    pddl_factories.get_ground_atoms_from_ids<Derived>(positive_derived_precondition_bitset, positive_derived_precondition);
    pddl_factories.get_ground_atoms_from_ids<Derived>(negative_derived_precondition_bitset, negative_derived_precondition);

    os << "positive static precondition=" << positive_static_precondition << ", "
       << "negative static precondition=" << negative_static_precondition << ", "
       << "positive fluent precondition=" << positive_fluent_precondition << ", "
       << "negative fluent precondition=" << negative_fluent_precondition << ", "
       << "positive derived precondition=" << positive_derived_precondition << ", "
       << "negative derived precondition=" << negative_derived_precondition;

    return os;
}

std::ostream& operator<<(std::ostream& os, const std::tuple<StripsActionEffect, const PDDLFactories&>& data)
{
    const auto [strips_effect_proxy, pddl_factories] = data;

    const auto positive_effect_bitset = strips_effect_proxy.get_positive_effects();
    const auto negative_effect_bitset = strips_effect_proxy.get_negative_effects();

    auto positive_simple_effects = GroundAtomList<Fluent> {};
    auto negative_simple_effects = GroundAtomList<Fluent> {};

    pddl_factories.get_ground_atoms_from_ids<Fluent>(positive_effect_bitset, positive_simple_effects);
    pddl_factories.get_ground_atoms_from_ids<Fluent>(negative_effect_bitset, negative_simple_effects);

    os << "delete effects=" << negative_simple_effects << ", "
       << "add effects=" << positive_simple_effects;

    return os;
}

std::ostream& operator<<(std::ostream& os, const std::tuple<ConditionalEffect, const PDDLFactories&>& data)
{
    const auto [cond_effect_proxy, pddl_factories] = data;

    const auto positive_static_precondition_bitset = cond_effect_proxy.get_positive_precondition<Static>();
    const auto negative_static_precondition_bitset = cond_effect_proxy.get_negative_precondition<Static>();
    const auto positive_fluent_precondition_bitset = cond_effect_proxy.get_positive_precondition<Fluent>();
    const auto negative_fluent_precondition_bitset = cond_effect_proxy.get_negative_precondition<Fluent>();
    const auto positive_derived_precondition_bitset = cond_effect_proxy.get_positive_precondition<Derived>();
    const auto negative_derived_precondition_bitset = cond_effect_proxy.get_negative_precondition<Derived>();
    const auto& simple_effect = cond_effect_proxy.get_simple_effect();

    auto positive_static_precondition = GroundAtomList<Static> {};
    auto negative_static_precondition = GroundAtomList<Static> {};
    auto positive_fluent_precondition = GroundAtomList<Fluent> {};
    auto negative_fluent_precondition = GroundAtomList<Fluent> {};
    auto positive_derived_precondition = GroundAtomList<Derived> {};
    auto negative_derived_precondition = GroundAtomList<Derived> {};

    pddl_factories.get_ground_atoms_from_ids<Static>(positive_static_precondition_bitset, positive_static_precondition);
    pddl_factories.get_ground_atoms_from_ids<Static>(negative_static_precondition_bitset, negative_static_precondition);
    pddl_factories.get_ground_atoms_from_ids<Fluent>(positive_fluent_precondition_bitset, positive_fluent_precondition);
    pddl_factories.get_ground_atoms_from_ids<Fluent>(negative_fluent_precondition_bitset, negative_fluent_precondition);
    pddl_factories.get_ground_atoms_from_ids<Derived>(positive_derived_precondition_bitset, positive_derived_precondition);
    pddl_factories.get_ground_atoms_from_ids<Derived>(negative_derived_precondition_bitset, negative_derived_precondition);

    os << "positive static precondition=" << positive_static_precondition << ", "
       << "negative static precondition=" << negative_static_precondition << ", "
       << "positive fluent precondition=" << positive_fluent_precondition << ", "
       << "negative fluent precondition=" << negative_fluent_precondition << ", "
       << "positive derived precondition=" << positive_derived_precondition << ", "
       << "negative derived precondition=" << negative_derived_precondition << ", "
       << "effect=" << std::make_tuple(simple_effect, std::cref(pddl_factories));

    return os;
}

std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAction, const PDDLFactories&>& data)
{
    const auto [action, pddl_factories] = data;

    auto binding = ObjectList {};
    for (const auto object : action.get_objects())
    {
        binding.push_back(object);
    }

    auto strips_precondition = StripsActionPrecondition(action.get_strips_precondition());
    auto strips_effect = StripsActionEffect(action.get_strips_effect());
    auto cond_effects = action.get_conditional_effects();

    os << "Action("                                                                //
       << "id=" << action.get_id() << ", "                                         //
       << "name=" << action.get_action()->get_name() << ", "                       //
       << "binding=" << binding << ", "                                            //
       << std::make_tuple(strips_precondition, std::cref(pddl_factories)) << ", "  //
       << std::make_tuple(strips_effect, std::cref(pddl_factories))                //
       << ", "
       << "conditional_effects=[";
    for (const auto& cond_effect : cond_effects)
    {
        os << "[" << std::make_tuple(ConditionalEffect(cond_effect), std::cref(pddl_factories)) << "], ";
    }
    os << "])";

    return os;
}

std::ostream& operator<<(std::ostream& os, GroundAction action)
{
    os << "(" << action.get_action()->get_name();
    // Only take objects w.r.t. to the original action parameters
    for (size_t i = 0; i < action.get_action()->get_original_arity(); ++i)
    {
        os << " " << *action.get_objects()[i];
    }
    os << ")";
    return os;
}
}
