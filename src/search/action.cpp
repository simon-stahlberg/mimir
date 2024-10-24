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
#include "mimir/formalism/factories.hpp"

#include <ostream>
#include <tuple>

size_t std::hash<mimir::GroundAction>::operator()(mimir::GroundAction element) const { return element.get_index(); }

size_t cista::storage::DerefStdHasher<mimir::FlatAction>::operator()(const mimir::FlatAction* ptr) const
{
    const auto action = cista::get<1>(*ptr);
    const auto& objects = cista::get<3>(*ptr);
    return mimir::hash_combine(action, objects);
}

bool cista::storage::DerefStdEqualTo<mimir::FlatAction>::operator()(const mimir::FlatAction* lhs, const mimir::FlatAction* rhs) const
{
    const auto action_left = cista::get<1>(*lhs);
    const auto& objects_left = cista::get<3>(*lhs);
    const auto action_right = cista::get<1>(*rhs);
    const auto& objects_right = cista::get<3>(*rhs);
    return (action_left == action_right) && (objects_left == objects_right);
}

namespace mimir
{

/* FlatSimpleEffect*/

bool FlatSimpleEffect::operator==(const FlatSimpleEffect& other) const
{
    if (this != &other)
    {
        return is_negated == other.is_negated && atom_index == other.atom_index;
    }
    return true;
}

/* StripsActionPreconditionBuilder */
StripsActionPreconditionBuilder::StripsActionPreconditionBuilder(FlatStripsActionPrecondition& builder) : m_builder(builder) {}

template<PredicateCategory P>
FlatBitset& StripsActionPreconditionBuilder::get_positive_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return cista::get<0>(m_builder.get());
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return cista::get<2>(m_builder.get());
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return cista::get<4>(m_builder.get());
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatBitset& StripsActionPreconditionBuilder::get_positive_precondition<Static>();
template FlatBitset& StripsActionPreconditionBuilder::get_positive_precondition<Fluent>();
template FlatBitset& StripsActionPreconditionBuilder::get_positive_precondition<Derived>();

template<PredicateCategory P>
FlatBitset& StripsActionPreconditionBuilder::get_negative_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return cista::get<1>(m_builder.get());
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return cista::get<3>(m_builder.get());
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return cista::get<5>(m_builder.get());
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatBitset& StripsActionPreconditionBuilder::get_negative_precondition<Static>();
template FlatBitset& StripsActionPreconditionBuilder::get_negative_precondition<Fluent>();
template FlatBitset& StripsActionPreconditionBuilder::get_negative_precondition<Derived>();

/* StripsActionPrecondition */
StripsActionPrecondition::StripsActionPrecondition(const FlatStripsActionPrecondition& view) : m_view(view) {}

template<PredicateCategory P>
const FlatBitset& StripsActionPrecondition::get_positive_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return cista::get<0>(m_view.get());
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return cista::get<2>(m_view.get());
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return cista::get<4>(m_view.get());
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template const FlatBitset& StripsActionPrecondition::get_positive_precondition<Static>() const;
template const FlatBitset& StripsActionPrecondition::get_positive_precondition<Fluent>() const;
template const FlatBitset& StripsActionPrecondition::get_positive_precondition<Derived>() const;

template<PredicateCategory P>
const FlatBitset& StripsActionPrecondition::get_negative_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return cista::get<1>(m_view.get());
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return cista::get<3>(m_view.get());
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return cista::get<5>(m_view.get());
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template const FlatBitset& StripsActionPrecondition::get_negative_precondition<Static>() const;
template const FlatBitset& StripsActionPrecondition::get_negative_precondition<Fluent>() const;
template const FlatBitset& StripsActionPrecondition::get_negative_precondition<Derived>() const;

template<PredicateCategory P>
bool StripsActionPrecondition::is_applicable(const FlatBitset& atoms) const
{
    return atoms.is_superseteq(get_positive_precondition<P>())  //
           && atoms.are_disjoint(get_negative_precondition<P>());
}

template bool StripsActionPrecondition::is_applicable<Static>(const FlatBitset& atoms) const;
template bool StripsActionPrecondition::is_applicable<Fluent>(const FlatBitset& atoms) const;
template bool StripsActionPrecondition::is_applicable<Derived>(const FlatBitset& atoms) const;

template<DynamicPredicateCategory P>
bool StripsActionPrecondition::is_applicable(State state) const
{
    return is_applicable<P>(state.get_atoms<P>());
}

template bool StripsActionPrecondition::is_applicable<Fluent>(State state) const;
template bool StripsActionPrecondition::is_applicable<Derived>(State state) const;

bool StripsActionPrecondition::is_dynamically_applicable(State state) const
{  //
    return is_applicable<Fluent>(state) && is_applicable<Derived>(state);
}

bool StripsActionPrecondition::is_statically_applicable(const FlatBitset& static_positive_atoms) const { return is_applicable<Static>(static_positive_atoms); }

bool StripsActionPrecondition::is_applicable(Problem problem, State state) const
{
    return is_dynamically_applicable(state) && is_statically_applicable(problem->get_static_initial_positive_atoms());
}

bool StripsActionPrecondition::is_applicable(const FlatBitset& fluent_state_atoms,
                                             const FlatBitset& derived_state_atoms,
                                             const FlatBitset& static_initial_atoms) const
{
    return is_applicable<Fluent>(fluent_state_atoms) && is_applicable<Static>(static_initial_atoms) && is_applicable<Derived>(derived_state_atoms);
}

/* StripsActionEffectBuilder */

StripsActionEffectBuilder::StripsActionEffectBuilder(FlatStripsActionEffect& builder) : m_builder(builder) {}

FlatBitset& StripsActionEffectBuilder::get_positive_effects() { return cista::get<0>(m_builder.get()); }

FlatBitset& StripsActionEffectBuilder::get_negative_effects() { return cista::get<1>(m_builder.get()); }

/* StripsActionEffect */

StripsActionEffect::StripsActionEffect(const FlatStripsActionEffect& view) : m_view(view) {}

const FlatBitset& StripsActionEffect::get_positive_effects() const { return cista::get<0>(m_view.get()); }
const FlatBitset& StripsActionEffect::get_negative_effects() const { return cista::get<1>(m_view.get()); }

/* ConditionalEffectBuilder */

ConditionalEffectBuilder::ConditionalEffectBuilder(FlatConditionalEffect& builder) : m_builder(builder) {}

template<PredicateCategory P>
FlatIndexList& ConditionalEffectBuilder::get_positive_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return cista::get<0>(m_builder.get());
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return cista::get<2>(m_builder.get());
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return cista::get<4>(m_builder.get());
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatIndexList& ConditionalEffectBuilder::get_positive_precondition<Static>();
template FlatIndexList& ConditionalEffectBuilder::get_positive_precondition<Fluent>();
template FlatIndexList& ConditionalEffectBuilder::get_positive_precondition<Derived>();

template<PredicateCategory P>
FlatIndexList& ConditionalEffectBuilder::get_negative_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return cista::get<1>(m_builder.get());
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return cista::get<3>(m_builder.get());
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return cista::get<5>(m_builder.get());
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatIndexList& ConditionalEffectBuilder::get_negative_precondition<Static>();
template FlatIndexList& ConditionalEffectBuilder::get_negative_precondition<Fluent>();
template FlatIndexList& ConditionalEffectBuilder::get_negative_precondition<Derived>();

FlatSimpleEffect& ConditionalEffectBuilder::get_simple_effect() { return cista::get<6>(m_builder.get()); }

/* ConditionalEffect */

ConditionalEffect::ConditionalEffect(const FlatConditionalEffect& view) : m_view(view) {}

template<PredicateCategory P>
const FlatIndexList& ConditionalEffect::get_positive_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return cista::get<0>(m_view.get());
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return cista::get<2>(m_view.get());
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return cista::get<4>(m_view.get());
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template const FlatIndexList& ConditionalEffect::get_positive_precondition<Static>() const;
template const FlatIndexList& ConditionalEffect::get_positive_precondition<Fluent>() const;
template const FlatIndexList& ConditionalEffect::get_positive_precondition<Derived>() const;

template<PredicateCategory P>
const FlatIndexList& ConditionalEffect::get_negative_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return cista::get<1>(m_view.get());
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return cista::get<3>(m_view.get());
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return cista::get<5>(m_view.get());
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template const FlatIndexList& ConditionalEffect::get_negative_precondition<Static>() const;
template const FlatIndexList& ConditionalEffect::get_negative_precondition<Fluent>() const;
template const FlatIndexList& ConditionalEffect::get_negative_precondition<Derived>() const;

const FlatSimpleEffect& ConditionalEffect::get_simple_effect() const { return cista::get<6>(m_view.get()); }

template<DynamicPredicateCategory P>
bool ConditionalEffect::is_applicable(State state) const
{
    const auto& state_atoms = state.get_atoms<P>();

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
    const auto& static_initial_atoms = problem->get_static_initial_positive_atoms();

    return is_superseteq(static_initial_atoms, get_positive_precondition<Static>())  //
           && are_disjoint(static_initial_atoms, get_negative_precondition<Static>());
}

bool ConditionalEffect::is_applicable(Problem problem, State state) const { return is_dynamically_applicable(state) && is_statically_applicable(problem); }

/* GroundActionBuilder */

FlatAction& GroundActionBuilder::get_data() { return m_builder; }

const FlatAction& GroundActionBuilder::get_data() const { return m_builder; }

Index& GroundActionBuilder::get_index() { return cista::get<0>(m_builder); }

Index& GroundActionBuilder::get_action_index() { return cista::get<1>(m_builder); }

ContinuousCost& GroundActionBuilder::get_cost() { return cista::get<2>(m_builder); }

FlatIndexList& GroundActionBuilder::get_objects() { return cista::get<3>(m_builder); }

FlatStripsActionPrecondition& GroundActionBuilder::get_strips_precondition() { return cista::get<4>(m_builder); }

FlatStripsActionEffect& GroundActionBuilder::get_strips_effect() { return cista::get<5>(m_builder); }

FlatConditionalEffects& GroundActionBuilder::get_conditional_effects() { return cista::get<6>(m_builder); }

/* GroundAction */

GroundAction::GroundAction(const FlatAction& view) : m_view(view) {}

GroundAction GroundAction::get_null_ground_action() { return GroundAction(s_null_ground_action.get_data()); }

static GroundActionBuilder create_null_ground_action()
{
    auto ground_action_builder = GroundActionBuilder();
    ground_action_builder.get_index() = std::numeric_limits<Index>::max();
    return ground_action_builder;
}

const GroundActionBuilder GroundAction::s_null_ground_action = create_null_ground_action();

Index GroundAction::get_index() const { return cista::get<0>(m_view.get()); }

Index GroundAction::get_action_index() const { return cista::get<1>(m_view.get()); }

ContinuousCost GroundAction::get_cost() const { return cista::get<2>(m_view.get()); }

const FlatIndexList& GroundAction::get_object_indices() const { return cista::get<3>(m_view.get()); }

const FlatStripsActionPrecondition& GroundAction::get_strips_precondition() const { return cista::get<4>(m_view.get()); }

const FlatStripsActionEffect& GroundAction::get_strips_effect() const { return cista::get<5>(m_view.get()); }

const FlatConditionalEffects& GroundAction::get_conditional_effects() const { return cista::get<6>(m_view.get()); }

bool GroundAction::is_dynamically_applicable(State state) const
{  //
    return StripsActionPrecondition(get_strips_precondition()).is_dynamically_applicable(state);
}

bool GroundAction::is_statically_applicable(const FlatBitset& static_positive_atoms) const
{  //
    return StripsActionPrecondition(get_strips_precondition()).is_statically_applicable(static_positive_atoms);
}

bool GroundAction::is_applicable(Problem problem, State state) const
{  //
    return is_dynamically_applicable(state) && is_statically_applicable(problem->get_static_initial_positive_atoms());
}
bool operator==(GroundAction lhs, GroundAction rhs) { return (lhs.get_index() == rhs.get_index()); }

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<FlatSimpleEffect, const PDDLFactories&>& data)
{
    const auto [simple_effect, pddl_factories] = data;

    const auto& ground_atom = pddl_factories.get_ground_atom<Fluent>(simple_effect.atom_index);

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

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<StripsActionPrecondition, const PDDLFactories&>& data)
{
    const auto [strips_precondition_proxy, pddl_factories] = data;

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

    pddl_factories.get_ground_atoms_from_indices<Static>(positive_static_precondition_bitset, positive_static_precondition);
    pddl_factories.get_ground_atoms_from_indices<Static>(negative_static_precondition_bitset, negative_static_precondition);
    pddl_factories.get_ground_atoms_from_indices<Fluent>(positive_fluent_precondition_bitset, positive_fluent_precondition);
    pddl_factories.get_ground_atoms_from_indices<Fluent>(negative_fluent_precondition_bitset, negative_fluent_precondition);
    pddl_factories.get_ground_atoms_from_indices<Derived>(positive_derived_precondition_bitset, positive_derived_precondition);
    pddl_factories.get_ground_atoms_from_indices<Derived>(negative_derived_precondition_bitset, negative_derived_precondition);

    os << "positive static precondition=" << positive_static_precondition << ", "
       << "negative static precondition=" << negative_static_precondition << ", "
       << "positive fluent precondition=" << positive_fluent_precondition << ", "
       << "negative fluent precondition=" << negative_fluent_precondition << ", "
       << "positive derived precondition=" << positive_derived_precondition << ", "
       << "negative derived precondition=" << negative_derived_precondition;

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<StripsActionEffect, const PDDLFactories&>& data)
{
    const auto [strips_effect_proxy, pddl_factories] = data;

    const auto& positive_effect_bitset = strips_effect_proxy.get_positive_effects();
    const auto& negative_effect_bitset = strips_effect_proxy.get_negative_effects();

    auto positive_simple_effects = GroundAtomList<Fluent> {};
    auto negative_simple_effects = GroundAtomList<Fluent> {};

    pddl_factories.get_ground_atoms_from_indices<Fluent>(positive_effect_bitset, positive_simple_effects);
    pddl_factories.get_ground_atoms_from_indices<Fluent>(negative_effect_bitset, negative_simple_effects);

    os << "delete effects=" << negative_simple_effects << ", "
       << "add effects=" << positive_simple_effects;

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<ConditionalEffect, const PDDLFactories&>& data)
{
    const auto [cond_effect_proxy, pddl_factories] = data;

    const auto& positive_static_precondition_bitset = cond_effect_proxy.get_positive_precondition<Static>();
    const auto& negative_static_precondition_bitset = cond_effect_proxy.get_negative_precondition<Static>();
    const auto& positive_fluent_precondition_bitset = cond_effect_proxy.get_positive_precondition<Fluent>();
    const auto& negative_fluent_precondition_bitset = cond_effect_proxy.get_negative_precondition<Fluent>();
    const auto& positive_derived_precondition_bitset = cond_effect_proxy.get_positive_precondition<Derived>();
    const auto& negative_derived_precondition_bitset = cond_effect_proxy.get_negative_precondition<Derived>();
    const auto& simple_effect = cond_effect_proxy.get_simple_effect();

    auto positive_static_precondition = GroundAtomList<Static> {};
    auto negative_static_precondition = GroundAtomList<Static> {};
    auto positive_fluent_precondition = GroundAtomList<Fluent> {};
    auto negative_fluent_precondition = GroundAtomList<Fluent> {};
    auto positive_derived_precondition = GroundAtomList<Derived> {};
    auto negative_derived_precondition = GroundAtomList<Derived> {};

    pddl_factories.get_ground_atoms_from_indices<Static>(positive_static_precondition_bitset, positive_static_precondition);
    pddl_factories.get_ground_atoms_from_indices<Static>(negative_static_precondition_bitset, negative_static_precondition);
    pddl_factories.get_ground_atoms_from_indices<Fluent>(positive_fluent_precondition_bitset, positive_fluent_precondition);
    pddl_factories.get_ground_atoms_from_indices<Fluent>(negative_fluent_precondition_bitset, negative_fluent_precondition);
    pddl_factories.get_ground_atoms_from_indices<Derived>(positive_derived_precondition_bitset, positive_derived_precondition);
    pddl_factories.get_ground_atoms_from_indices<Derived>(negative_derived_precondition_bitset, negative_derived_precondition);

    os << "positive static precondition=" << positive_static_precondition << ", "
       << "negative static precondition=" << negative_static_precondition << ", "
       << "positive fluent precondition=" << positive_fluent_precondition << ", "
       << "negative fluent precondition=" << negative_fluent_precondition << ", "
       << "positive derived precondition=" << positive_derived_precondition << ", "
       << "negative derived precondition=" << negative_derived_precondition << ", "
       << "effect=" << std::make_tuple(simple_effect, std::cref(pddl_factories));

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAction, const PDDLFactories&>& data)
{
    const auto [action, pddl_factories] = data;

    auto binding = ObjectList {};
    for (const auto object_index : action.get_object_indices())
    {
        binding.push_back(pddl_factories.get_object(object_index));
    }

    const auto strips_precondition = StripsActionPrecondition(action.get_strips_precondition());
    const auto strips_effect = StripsActionEffect(action.get_strips_effect());
    const auto& cond_effects = action.get_conditional_effects();

    os << "Action("                                                                            //
       << "index=" << action.get_index() << ", "                                               //
       << "name=" << pddl_factories.get_action(action.get_action_index())->get_name() << ", "  //
       << "binding=" << binding << ", "                                                        //
       << std::make_tuple(strips_precondition, std::cref(pddl_factories)) << ", "              //
       << std::make_tuple(strips_effect, std::cref(pddl_factories))                            //
       << ", "
       << "conditional_effects=[";
    for (const auto& cond_effect : cond_effects)
    {
        os << "[" << std::make_tuple(ConditionalEffect(cond_effect), std::cref(pddl_factories)) << "], ";
    }
    os << "])";

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<const PDDLFactories&, GroundAction>& data)
{
    const auto [pddl_factories, ground_action] = data;

    const auto action = pddl_factories.get_action(ground_action.get_action_index());
    os << "(" << action->get_name();
    // Only take objects w.r.t. to the original action parameters
    for (size_t i = 0; i < action->get_original_arity(); ++i)
    {
        os << " " << *pddl_factories.get_object(ground_action.get_object_indices()[i]);
    }
    os << ")";
    return os;
}
}
