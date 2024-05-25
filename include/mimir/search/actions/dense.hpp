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

#ifndef MIMIR_SEARCH_ACTIONS_DENSE_HPP_
#define MIMIR_SEARCH_ACTIONS_DENSE_HPP_

#include "mimir/formalism/formalism.hpp"
#include "mimir/search/actions/interface.hpp"
#include "mimir/search/builder.hpp"
#include "mimir/search/flat_types.hpp"
#include "mimir/search/states/dense.hpp"
#include "mimir/search/view_const.hpp"

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

using FlatDenseStripsActionPreconditionLayout = flatmemory::Tuple<FlatBitsetLayout,   // static positive conditions
                                                                  FlatBitsetLayout,   // static negative conditions
                                                                  FlatBitsetLayout,   // fluent positive conditions
                                                                  FlatBitsetLayout>;  // fluent negative conditions
using FlatDenseStripsActionPreconditionBuilder = flatmemory::Builder<FlatDenseStripsActionPreconditionLayout>;
using FlatDenseStripsActionPrecondition = flatmemory::ConstView<FlatDenseStripsActionPreconditionLayout>;

using FlatDenseStripsActionEffectLayout = flatmemory::Tuple<FlatBitsetLayout,   // add effects
                                                            FlatBitsetLayout>;  // delete effects
using FlatDenseStripsActionEffectBuilder = flatmemory::Builder<FlatDenseStripsActionEffectLayout>;
using FlatDenseStripsActionEffect = flatmemory::ConstView<FlatDenseStripsActionEffectLayout>;

using FlatDenseConditionalEffectLayout = flatmemory::Tuple<FlatBitsetLayout,   // static positive conditions
                                                           FlatBitsetLayout,   // static negative conditions
                                                           FlatBitsetLayout,   // fluent positive conditions
                                                           FlatBitsetLayout,   // fluent negative conditions
                                                           FlatSimpleEffect>;  // simple add or delete effect
using FlatDenseConditionalEffectBuilder = flatmemory::Builder<FlatDenseConditionalEffectLayout>;
using FlatDenseConditionalEffect = flatmemory::ConstView<FlatDenseConditionalEffectLayout>;

using FlatDenseConditionalEffectsLayout = flatmemory::Vector<FlatDenseConditionalEffectLayout>;  // simple add or delete effect
using FlatDenseConditionalEffectsBuilder = flatmemory::Builder<FlatDenseConditionalEffectsLayout>;
using FlatDenseConditionalEffects = flatmemory::ConstView<FlatDenseConditionalEffectsLayout>;

using FlatSimpleEffectVectorLayout = flatmemory::Vector<FlatSimpleEffect>;
using FlatSimpleEffectVectorBuilder = flatmemory::Builder<FlatSimpleEffectVectorLayout>;
using FlatSimpleEffectVector = flatmemory::ConstView<FlatSimpleEffectVectorLayout>;

using FlatDenseActionLayout = flatmemory::Tuple<uint32_t,  //
                                                int32_t,
                                                Action,
                                                FlatObjectListLayout,
                                                FlatDenseStripsActionPreconditionLayout,
                                                FlatDenseStripsActionEffectLayout,
                                                FlatDenseConditionalEffectsLayout>;
using FlatDenseActionBuilder = flatmemory::Builder<FlatDenseActionLayout>;
using FlatDenseAction = flatmemory::ConstView<FlatDenseActionLayout>;
using FlatDenseActionVector = flatmemory::VariableSizedTypeVector<FlatDenseActionLayout>;

struct FlatDenseActionHash
{
    size_t operator()(const FlatDenseAction& view) const
    {
        const auto action = view.get<2>();
        const auto objects = view.get<3>();
        return loki::hash_combine(action, objects.hash());
    }
};

struct FlatDenseActionEqual
{
    bool operator()(const FlatDenseAction& view_left, const FlatDenseAction& view_right) const
    {
        const auto action_left = view_left.get<2>();
        const auto objects_left = view_left.get<3>();
        const auto action_right = view_right.get<2>();
        const auto objects_right = view_right.get<3>();
        return (action_left == action_right) && (objects_left == objects_right);
    }
};

using FlatDenseActionSet = flatmemory::UnorderedSet<FlatDenseActionLayout, FlatDenseActionHash, FlatDenseActionEqual>;

/**
 * Implementation class
 */

class DenseStripsActionPreconditionBuilderProxy
{
private:
    FlatDenseStripsActionPreconditionBuilder& m_builder;

public:
    explicit DenseStripsActionPreconditionBuilderProxy(FlatDenseStripsActionPreconditionBuilder& builder) : m_builder(builder) {}

    /* Precondition */
    [[nodiscard]] FlatBitsetBuilder& get_positive_fluent_precondition() { return m_builder.get<0>(); }
    [[nodiscard]] FlatBitsetBuilder& get_negative_fluent_precondition() { return m_builder.get<1>(); }
    [[nodiscard]] FlatBitsetBuilder& get_positive_static_precondition() { return m_builder.get<2>(); }
    [[nodiscard]] FlatBitsetBuilder& get_negative_static_precondition() { return m_builder.get<3>(); }
};

class DenseStripsActionPrecondition
{
private:
    FlatDenseStripsActionPrecondition m_view;

public:
    explicit DenseStripsActionPrecondition(FlatDenseStripsActionPrecondition view) : m_view(view) {}

    /* Precondition */
    [[nodiscard]] FlatBitset get_positive_fluent_precondition() const { return m_view.get<0>(); }
    [[nodiscard]] FlatBitset get_negative_fluent_precondition() const { return m_view.get<1>(); }
    [[nodiscard]] FlatBitset get_positive_static_precondition() const { return m_view.get<2>(); }
    [[nodiscard]] FlatBitset get_negative_static_precondition() const { return m_view.get<3>(); }

    [[nodiscard]] bool is_applicable(DenseState state) const
    {
        const auto state_bitset = state.get_fluent_atoms();
        const auto initial_static_atoms = state.get_problem()->get_static_initial_positive_atoms_bitset();

        return state_bitset.is_superseteq(get_positive_fluent_precondition())             //
               && state_bitset.are_disjoint(get_negative_fluent_precondition())           //
               && initial_static_atoms.is_superseteq(get_positive_static_precondition())  //
               && initial_static_atoms.are_disjoint(get_negative_static_precondition());
    }

    template<flatmemory::IsBitset Bitset1, flatmemory::IsBitset Bitset2>
    [[nodiscard]] bool is_applicable(const Bitset1 state_bitset, const Bitset2 static_positive_bitset) const
    {
        return state_bitset.is_superseteq(get_positive_fluent_precondition())               //
               && state_bitset.are_disjoint(get_negative_fluent_precondition())             //
               && static_positive_bitset.is_superseteq(get_positive_static_precondition())  //
               && static_positive_bitset.are_disjoint(get_negative_static_precondition());
    }

    template<flatmemory::IsBitset Bitset>
    [[nodiscard]] bool is_statically_applicable(const Bitset static_positive_bitset) const
    {
        return static_positive_bitset.is_superseteq(get_positive_static_precondition())
               && static_positive_bitset.are_disjoint(get_negative_static_precondition());
    }
};

class DenseStripsActionEffectBuilderProxy
{
private:
    FlatDenseStripsActionEffectBuilder& m_builder;

public:
    explicit DenseStripsActionEffectBuilderProxy(FlatDenseStripsActionEffectBuilder& builder) : m_builder(builder) {}

    [[nodiscard]] FlatBitsetBuilder& get_positive_effects() { return m_builder.get<0>(); }
    [[nodiscard]] FlatBitsetBuilder& get_negative_effects() { return m_builder.get<1>(); }
};

class DenseStripsActionEffect
{
private:
    FlatDenseStripsActionEffect m_view;

public:
    explicit DenseStripsActionEffect(FlatDenseStripsActionEffect view) : m_view(view) {}

    [[nodiscard]] FlatBitset get_positive_effects() const { return m_view.get<0>(); }
    [[nodiscard]] FlatBitset get_negative_effects() const { return m_view.get<1>(); }
};

class DenseConditionalEffectBuilderProxy
{
private:
    FlatDenseConditionalEffectBuilder& m_builder;

public:
    explicit DenseConditionalEffectBuilderProxy(FlatDenseConditionalEffectBuilder& builder) : m_builder(builder) {}

    /* Precondition */
    [[nodiscard]] FlatBitsetBuilder& get_positive_fluent_precondition() { return m_builder.get<0>(); }
    [[nodiscard]] FlatBitsetBuilder& get_negative_fluent_precondition() { return m_builder.get<1>(); }
    [[nodiscard]] FlatBitsetBuilder& get_positive_static_precondition() { return m_builder.get<2>(); }
    [[nodiscard]] FlatBitsetBuilder& get_negative_static_precondition() { return m_builder.get<3>(); }
    /* Simple effects */
    [[nodiscard]] FlatSimpleEffect& get_simple_effect() { return m_builder.get<4>(); }
};

class DenseConditionalEffect
{
private:
    FlatDenseConditionalEffect m_view;

public:
    explicit DenseConditionalEffect(FlatDenseConditionalEffect view) : m_view(view) {}

    /* Precondition */
    [[nodiscard]] FlatBitset get_positive_fluent_precondition() const { return m_view.get<0>(); }
    [[nodiscard]] FlatBitset get_negative_fluent_precondition() const { return m_view.get<1>(); }
    [[nodiscard]] FlatBitset get_positive_static_precondition() const { return m_view.get<2>(); }
    [[nodiscard]] FlatBitset get_negative_static_precondition() const { return m_view.get<3>(); }
    /* Simple effects */
    [[nodiscard]] const FlatSimpleEffect& get_simple_effect() const { return m_view.get<4>(); }

    [[nodiscard]] bool is_applicable(DenseState state) const
    {
        const auto state_bitset = state.get_fluent_atoms();
        const auto initial_static_atoms = state.get_problem()->get_static_initial_positive_atoms_bitset();

        return state_bitset.is_superseteq(get_positive_fluent_precondition())             //
               && state_bitset.are_disjoint(get_negative_fluent_precondition())           //
               && initial_static_atoms.is_superseteq(get_positive_static_precondition())  //
               && initial_static_atoms.are_disjoint(get_negative_static_precondition());
    }
};

template<>
class Builder<ActionDispatcher<DenseStateTag>> :
    public IBuilder<Builder<ActionDispatcher<DenseStateTag>>>,
    public IActionBuilder<Builder<ActionDispatcher<DenseStateTag>>>
{
private:
    FlatDenseActionBuilder m_builder;

    /* Implement IBuilder interface */
    friend class IBuilder<Builder<ActionDispatcher<DenseStateTag>>>;

    [[nodiscard]] FlatDenseActionBuilder& get_flatmemory_builder_impl() { return m_builder; }
    [[nodiscard]] const FlatDenseActionBuilder& get_flatmemory_builder_impl() const { return m_builder; }

    /* Implement IActionBuilder interface */
    friend class IActionBuilder<Builder<ActionDispatcher<DenseStateTag>>>;

    [[nodiscard]] uint32_t& get_id_impl() { return m_builder.get<0>(); }
    [[nodiscard]] int32_t& get_cost_impl() { return m_builder.get<1>(); }
    [[nodiscard]] Action& get_action_impl() { return m_builder.get<2>(); }
    [[nodiscard]] FlatObjectListBuilder& get_objects_impl() { return m_builder.get<3>(); }

public:
    /* STRIPS part */
    [[nodiscard]] FlatDenseStripsActionPreconditionBuilder& get_strips_precondition() { return m_builder.get<4>(); }
    [[nodiscard]] FlatDenseStripsActionEffectBuilder& get_strips_effect() { return m_builder.get<5>(); }
    /* Conditional effects */
    [[nodiscard]] FlatDenseConditionalEffectsBuilder& get_conditional_effects() { return m_builder.get<6>(); }
};

/**
 * Implementation class
 *
 * Reads the memory layout generated by the search node builder.
 */
template<>
class ConstView<ActionDispatcher<DenseStateTag>> :
    public IConstView<ConstView<ActionDispatcher<DenseStateTag>>>,
    public IActionView<ConstView<ActionDispatcher<DenseStateTag>>>
{
private:
    using DenseState = ConstView<StateDispatcher<DenseStateTag>>;

    FlatDenseAction m_view;

    /* Implement IView interface: */
    friend class IConstView<ConstView<ActionDispatcher<DenseStateTag>>>;

    /// @brief Compute equality based on the lifted action and the objects assigned to the parameters.
    [[nodiscard]] bool are_equal_impl(const ConstView& other) const { return get_action() == other.get_action() && get_objects() == other.get_objects(); }

    /// @brief Compute hash based on the lifted action and the objects assigned to the parameters.
    [[nodiscard]] size_t hash_impl() const { return loki::hash_combine(get_action(), get_objects().hash()); }

    /* Implement IActionView interface */
    friend class IActionView<ConstView<ActionDispatcher<DenseStateTag>>>;

    [[nodiscard]] uint32_t get_id_impl() const { return m_view.get<0>(); }
    [[nodiscard]] int32_t get_cost_impl() const { return m_view.get<1>(); }
    [[nodiscard]] Action get_action_impl() const { return m_view.get<2>(); }
    [[nodiscard]] FlatObjectList get_objects_impl() const { return m_view.get<3>(); }

public:
    /// @brief Create a view on a DefaultAction.
    explicit ConstView(FlatDenseAction view) : m_view(view) {}

    /* STRIPS part */
    [[nodiscard]] FlatDenseStripsActionPrecondition get_strips_precondition() const { return m_view.get<4>(); }
    [[nodiscard]] FlatDenseStripsActionEffect get_strips_effect() const { return m_view.get<5>(); }
    /* Conditional effects */
    [[nodiscard]] FlatDenseConditionalEffects get_conditional_effects() const { return m_view.get<6>(); }

    [[nodiscard]] bool is_applicable(DenseState state) const
    {  //
        return DenseStripsActionPrecondition(get_strips_precondition()).is_applicable(state);
    }

    template<flatmemory::IsBitset Bitset>
    [[nodiscard]] bool is_statically_applicable(const Bitset static_positive_bitset) const
    {  //
        return DenseStripsActionPrecondition(get_strips_precondition()).is_statically_applicable(static_positive_bitset);
    }
};

/**
 * Mimir types
 */

using DenseGroundActionBuilder = Builder<ActionDispatcher<DenseStateTag>>;
using DenseGroundAction = ConstView<ActionDispatcher<DenseStateTag>>;
using DenseGroundActionList = std::vector<DenseGroundAction>;
using DenseGroundActionSet = std::unordered_set<DenseGroundAction, loki::Hash<DenseGroundAction>, loki::EqualTo<DenseGroundAction>>;

/**
 * Translations
 */

extern DenseGroundActionList to_ground_actions(const FlatDenseActionSet& flat_actions);

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& os, const std::tuple<FlatSimpleEffect, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, const std::tuple<DenseStripsActionPrecondition, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, const std::tuple<DenseStripsActionEffect, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, const std::tuple<DenseConditionalEffect, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, const std::tuple<DenseGroundAction, const PDDLFactories&>& data);

extern std::ostream& operator<<(std::ostream& os, const DenseGroundAction& action);

}

#endif
