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

#ifndef MIMIR_SEARCH_ACTION_HPP_
#define MIMIR_SEARCH_ACTION_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/formalism/formalism.hpp"
#include "mimir/search/flat_types.hpp"
#include "mimir/search/state.hpp"

#include <ostream>
#include <tuple>

namespace mimir
{
/**
 * Flatmemory types
 */

struct FlatSimpleEffect
{
    bool is_negated;
    size_t atom_id;

    bool operator==(const FlatSimpleEffect& other) const
    {
        if (this != &other)
        {
            return is_negated == other.is_negated && atom_id == other.atom_id;
        }
        return true;
    }
};

using FlatStripsActionPreconditionLayout = flatmemory::Tuple<FlatBitsetLayout<Static>,  //
                                                             FlatBitsetLayout<Static>,
                                                             FlatBitsetLayout<Fluent>,
                                                             FlatBitsetLayout<Fluent>,
                                                             FlatBitsetLayout<Derived>,
                                                             FlatBitsetLayout<Derived>>;
using FlatStripsActionPreconditionBuilder = flatmemory::Builder<FlatStripsActionPreconditionLayout>;
using FlatStripsActionPrecondition = flatmemory::ConstView<FlatStripsActionPreconditionLayout>;

using FlatStripsActionEffectLayout = flatmemory::Tuple<FlatBitsetLayout<Fluent>,   // add effects
                                                       FlatBitsetLayout<Fluent>>;  // delete effects
using FlatStripsActionEffectBuilder = flatmemory::Builder<FlatStripsActionEffectLayout>;
using FlatStripsActionEffect = flatmemory::ConstView<FlatStripsActionEffectLayout>;

using FlatConditionalEffectLayout = flatmemory::Tuple<FlatIndexListLayout,  // Positive static atom indices
                                                      FlatIndexListLayout,  // Negative static atom indices
                                                      FlatIndexListLayout,  // Positive fluent atom indices
                                                      FlatIndexListLayout,  // Negative fluent atom indices
                                                      FlatIndexListLayout,  // Positive derived atom indices
                                                      FlatIndexListLayout,  // Negative derived atom indices
                                                      FlatSimpleEffect>;    // simple add or delete
                                                                            // effect
using FlatConditionalEffectBuilder = flatmemory::Builder<FlatConditionalEffectLayout>;
using FlatConditionalEffect = flatmemory::ConstView<FlatConditionalEffectLayout>;

using FlatConditionalEffectsLayout = flatmemory::Vector<FlatConditionalEffectLayout>;  // simple add or delete effect
using FlatConditionalEffectsBuilder = flatmemory::Builder<FlatConditionalEffectsLayout>;
using FlatConditionalEffects = flatmemory::ConstView<FlatConditionalEffectsLayout>;

using FlatSimpleEffectVectorLayout = flatmemory::Vector<FlatSimpleEffect>;
using FlatSimpleEffectVectorBuilder = flatmemory::Builder<FlatSimpleEffectVectorLayout>;
using FlatSimpleEffectVector = flatmemory::ConstView<FlatSimpleEffectVectorLayout>;

using FlatActionLayout = flatmemory::Tuple<uint32_t,  //
                                           int32_t,
                                           Action,
                                           FlatObjectListLayout,
                                           FlatStripsActionPreconditionLayout,
                                           FlatStripsActionEffectLayout,
                                           FlatConditionalEffectsLayout>;
using FlatActionBuilder = flatmemory::Builder<FlatActionLayout>;
using FlatAction = flatmemory::ConstView<FlatActionLayout>;
using FlatActionVector = flatmemory::VariableSizedTypeVector<FlatActionLayout>;

struct FlatActionHash
{
    size_t operator()(FlatAction view) const
    {
        const auto action = view.get<2>();
        const auto objects = view.get<3>();
        return loki::hash_combine(action, objects.hash());
    }
};

struct FlatActionEqual
{
    bool operator()(FlatAction view_left, FlatAction view_right) const
    {
        const auto action_left = view_left.get<2>();
        const auto objects_left = view_left.get<3>();
        const auto action_right = view_right.get<2>();
        const auto objects_right = view_right.get<3>();
        return (action_left == action_right) && (objects_left == objects_right);
    }
};

using FlatActionSet = flatmemory::UnorderedSet<FlatActionLayout, FlatActionHash, FlatActionEqual>;

/**
 * Implementation class
 */

class StripsActionPreconditionBuilderProxy
{
private:
    FlatStripsActionPreconditionBuilder& m_builder;

public:
    explicit StripsActionPreconditionBuilderProxy(FlatStripsActionPreconditionBuilder& builder) : m_builder(builder) {}

    /* Precondition */

    template<PredicateCategory P>
    [[nodiscard]] FlatBitsetBuilder<P>& get_positive_precondition()
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

    template<PredicateCategory P>
    [[nodiscard]] FlatBitsetBuilder<P>& get_negative_precondition()
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
};

class StripsActionPrecondition
{
private:
    FlatStripsActionPrecondition m_view;

public:
    explicit StripsActionPrecondition(FlatStripsActionPrecondition view) : m_view(view) {}

    template<PredicateCategory P>
    [[nodiscard]] FlatBitset<P> get_positive_precondition() const
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

    template<PredicateCategory P>
    [[nodiscard]] FlatBitset<P> get_negative_precondition() const
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

    template<DynamicPredicateCategory P>
    [[nodiscard]] bool is_applicable(State state) const
    {
        const auto state_atoms = state.get_atoms<P>();

        return state_atoms.is_superseteq(get_positive_precondition<P>())  //
               && state_atoms.are_disjoint(get_negative_precondition<P>());
    }

    [[nodiscard]] bool is_dynamically_applicable(State state) const
    {  //
        return is_applicable<Fluent>(state) && is_applicable<Derived>(state);
    }

    template<flatmemory::IsBitset Bitset>
    [[nodiscard]] bool is_statically_applicable(const Bitset& static_positive_atoms) const
    {
        return is_applicable<Static>(static_positive_atoms);
    }

    [[nodiscard]] bool is_applicable(Problem problem, State state) const
    {
        return is_dynamically_applicable(state) && is_statically_applicable(problem->get_static_initial_positive_atoms_bitset());
    }

    template<PredicateCategory P, flatmemory::IsBitset Bitset>
        requires flatmemory::HasCompatibleTagType<Bitset, P>
    [[nodiscard]] bool is_applicable(const Bitset& atoms) const
    {
        return atoms.is_superseteq(get_positive_precondition<P>())  //
               && atoms.are_disjoint(get_negative_precondition<P>());
    }

    template<flatmemory::IsBitset Bitset1, flatmemory::IsBitset Bitset2, flatmemory::IsBitset Bitset3>
    [[nodiscard]] bool is_applicable(const Bitset1& fluent_state_atoms, const Bitset2& derived_state_atoms, const Bitset3& static_positive_bitset) const
    {
        return is_applicable<Fluent>(fluent_state_atoms) && is_applicable<Static>(static_positive_bitset) && is_applicable<Derived>(derived_state_atoms);
    }
};

class StripsActionEffectBuilderProxy
{
private:
    FlatStripsActionEffectBuilder& m_builder;

public:
    explicit StripsActionEffectBuilderProxy(FlatStripsActionEffectBuilder& builder) : m_builder(builder) {}

    [[nodiscard]] FlatBitsetBuilder<Fluent>& get_positive_effects() { return m_builder.get<0>(); }
    [[nodiscard]] FlatBitsetBuilder<Fluent>& get_negative_effects() { return m_builder.get<1>(); }
};

class StripsActionEffect
{
private:
    FlatStripsActionEffect m_view;

public:
    explicit StripsActionEffect(FlatStripsActionEffect view) : m_view(view) {}

    [[nodiscard]] FlatBitset<Fluent> get_positive_effects() const { return m_view.get<0>(); }
    [[nodiscard]] FlatBitset<Fluent> get_negative_effects() const { return m_view.get<1>(); }
};

class ConditionalEffectBuilderProxy
{
private:
    FlatConditionalEffectBuilder& m_builder;

public:
    explicit ConditionalEffectBuilderProxy(FlatConditionalEffectBuilder& builder) : m_builder(builder) {}

    /* Precondition */

    template<PredicateCategory P>
    [[nodiscard]] FlatIndexListBuilder& get_positive_precondition()
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

    template<PredicateCategory P>
    [[nodiscard]] FlatIndexListBuilder& get_negative_precondition()
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

    /* Simple effects */

    [[nodiscard]] FlatSimpleEffect& get_simple_effect() { return m_builder.get<6>(); }
};

class ConditionalEffect
{
private:
    FlatConditionalEffect m_view;

public:
    explicit ConditionalEffect(FlatConditionalEffect view) : m_view(view) {}

    /* Precondition */

    template<PredicateCategory P>
    [[nodiscard]] FlatIndexList get_positive_precondition() const
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

    template<PredicateCategory P>
    [[nodiscard]] FlatIndexList get_negative_precondition() const
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

    /* Simple effects */
    [[nodiscard]] const FlatSimpleEffect& get_simple_effect() const { return m_view.get<6>(); }

    template<DynamicPredicateCategory P>
    [[nodiscard]] bool is_applicable(State state) const
    {
        const auto state_atoms = state.get_atoms<P>();

        return is_superseteq(state_atoms, get_positive_precondition<P>())  //
               && are_disjoint(state_atoms, get_negative_precondition<P>());
    }

    [[nodiscard]] bool is_dynamically_applicable(State state) const
    {  //
        return is_applicable<Fluent>(state) && is_applicable<Derived>(state);
    }

    [[nodiscard]] bool is_statically_applicable(Problem problem) const
    {
        const auto static_initial_atoms = problem->get_static_initial_positive_atoms_bitset();

        return is_superseteq(static_initial_atoms, get_positive_precondition<Static>())  //
               && are_disjoint(static_initial_atoms, get_negative_precondition<Static>());
    }

    [[nodiscard]] bool is_applicable(Problem problem, State state) const { return is_dynamically_applicable(state) && is_statically_applicable(problem); }
};

class GroundActionBuilder
{
private:
    FlatActionBuilder m_builder;

public:
    [[nodiscard]] FlatActionBuilder& get_flatmemory_builder() { return m_builder; }
    [[nodiscard]] const FlatActionBuilder& get_flatmemory_builder() const { return m_builder; }

    [[nodiscard]] uint32_t& get_id() { return m_builder.get<0>(); }
    [[nodiscard]] int32_t& get_cost() { return m_builder.get<1>(); }
    [[nodiscard]] Action& get_action() { return m_builder.get<2>(); }
    [[nodiscard]] FlatObjectListBuilder& get_objects() { return m_builder.get<3>(); }

    /* STRIPS part */
    [[nodiscard]] FlatStripsActionPreconditionBuilder& get_strips_precondition() { return m_builder.get<4>(); }
    [[nodiscard]] FlatStripsActionEffectBuilder& get_strips_effect() { return m_builder.get<5>(); }
    /* Conditional effects */
    [[nodiscard]] FlatConditionalEffectsBuilder& get_conditional_effects() { return m_builder.get<6>(); }
};

/**
 * Implementation class
 *
 * Reads the memory layout generated by the search node builder.
 */
class GroundAction
{
private:
    FlatAction m_view;

public:
    /// @brief Create a view on a DefaultAction.
    explicit GroundAction(FlatAction view) : m_view(view) {}

    /// @brief Return a hash value for the grounded action.
    ///
    /// Same argument from operator== applies.
    [[nodiscard]] size_t hash() const { return loki::hash_combine(m_view.buffer()); }

    [[nodiscard]] uint32_t get_id() const { return m_view.get<0>(); }
    [[nodiscard]] int32_t get_cost() const { return m_view.get<1>(); }
    [[nodiscard]] Action get_action() const { return m_view.get<2>(); }
    [[nodiscard]] FlatObjectList get_objects() const { return m_view.get<3>(); }

    /// Return true iff two grounded actions are equal.
    ///
    /// For grounded actions in same AAG, we know they are already unique.
    /// Hence, comparison of the buffer pointer suffices.
    /// For grounded actions in different AAG, buffer pointers are always different.
    /// Hence, comparison always returns false.
    [[nodiscard]] bool operator==(GroundAction other) const { return m_view.buffer() == other.m_view.buffer(); }

    /* STRIPS part */
    [[nodiscard]] FlatStripsActionPrecondition get_strips_precondition() const { return m_view.get<4>(); }
    [[nodiscard]] FlatStripsActionEffect get_strips_effect() const { return m_view.get<5>(); }
    /* Conditional effects */
    [[nodiscard]] FlatConditionalEffects get_conditional_effects() const { return m_view.get<6>(); }

    [[nodiscard]] bool is_dynamically_applicable(State state) const
    {  //
        return StripsActionPrecondition(get_strips_precondition()).is_dynamically_applicable(state);
    }

    template<flatmemory::IsBitset Bitset>
    [[nodiscard]] bool is_statically_applicable(const Bitset& static_positive_bitset) const
    {  //
        return StripsActionPrecondition(get_strips_precondition()).is_statically_applicable(static_positive_bitset);
    }

    [[nodiscard]] bool is_applicable(Problem problem, State state) const
    {  //
        return is_dynamically_applicable(state) && is_statically_applicable(problem->get_static_initial_positive_atoms_bitset());
    }
};

/**
 * Mimir types
 */

using GroundActionList = std::vector<GroundAction>;
using GroundActionSet = std::unordered_set<GroundAction, loki::Hash<GroundAction>, loki::EqualTo<GroundAction>>;

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& os, const std::tuple<FlatSimpleEffect, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, const std::tuple<StripsActionPrecondition, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, const std::tuple<StripsActionEffect, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, const std::tuple<ConditionalEffect, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAction, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, GroundAction action);

}

#endif
