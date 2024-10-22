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

#include "cista/containers/tuple.h"
#include "cista/storage/unordered_set.h"
#include "cista/storage/vector.h"
#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/state.hpp"

#include <ostream>
#include <tuple>

namespace mimir
{
class PDDLFactories;

/**
 * Flatmemory types
 */

struct FlatSimpleEffect
{
    bool is_negated;
    uint32_t atom_index;

    bool operator==(const FlatSimpleEffect& other) const;
};

using FlatStripsActionPrecondition = cista::tuple<FlatBitset,   // positive static atoms
                                                  FlatBitset,   // negative static atoms
                                                  FlatBitset,   // positive fluent atoms
                                                  FlatBitset,   // negative fluent atoms
                                                  FlatBitset,   // positive derived atoms
                                                  FlatBitset>;  // negative derived atoms

using FlatStripsActionEffect = cista::tuple<FlatBitset,   // add fluent effects
                                            FlatBitset>;  // delete fluent effects

using FlatConditionalEffect = cista::tuple<FlatIndexList,      // Positive static atom indices
                                           FlatIndexList,      // Negative static atom indices
                                           FlatIndexList,      // Positive fluent atom indices
                                           FlatIndexList,      // Negative fluent atom indices
                                           FlatIndexList,      // Positive derived atom indices
                                           FlatIndexList,      // Negative derived atom indices
                                           FlatSimpleEffect>;  // simple add or delete effect

using FlatConditionalEffects = cista::offset::vector<FlatConditionalEffect>;

using FlatSimpleEffectVector = cista::offset::vector<FlatSimpleEffect>;

using FlatAction = cista::tuple<Index,           // GroundActionIndex
                                Index,           // ActionIndex
                                ContinuousCost,  // GroundActionCost
                                FlatIndexList,   // ObjectIndices
                                FlatStripsActionPrecondition,
                                FlatStripsActionEffect,
                                FlatConditionalEffects>;

using FlatActionVector = cista::storage::Vector<FlatAction>;

}

template<>
struct cista::storage::DerefStdHasher<mimir::FlatAction>
{
    size_t operator()(const mimir::FlatAction* ptr) const;
};

template<>
struct cista::storage::DerefStdEqualTo<mimir::FlatAction>
{
    bool operator()(const mimir::FlatAction* lhs, const mimir::FlatAction* rhs) const;
};

namespace mimir
{

using FlatActionSet = cista::storage::UnorderedSet<FlatAction>;

/**
 * Implementation class
 */

class StripsActionPreconditionBuilder
{
private:
    std::reference_wrapper<FlatStripsActionPrecondition> m_builder;

public:
    explicit StripsActionPreconditionBuilder(FlatStripsActionPrecondition& builder);

    /* Precondition */

    template<PredicateCategory P>
    FlatBitset& get_positive_precondition();

    template<PredicateCategory P>
    FlatBitset& get_negative_precondition();
};

class StripsActionPrecondition
{
private:
    std::reference_wrapper<const FlatStripsActionPrecondition> m_view;

public:
    explicit StripsActionPrecondition(const FlatStripsActionPrecondition& view);

    template<PredicateCategory P>
    const FlatBitset& get_positive_precondition() const;

    template<PredicateCategory P>
    const FlatBitset& get_negative_precondition() const;

    template<DynamicPredicateCategory P>
    bool is_applicable(State state) const;

    bool is_dynamically_applicable(State state) const;

    bool is_statically_applicable(const FlatBitset& static_positive_atoms) const;

    bool is_applicable(Problem problem, State state) const;

    template<PredicateCategory P>
    bool is_applicable(const FlatBitset& atoms) const;

    bool is_applicable(const FlatBitset& fluent_state_atoms, const FlatBitset& derived_state_atoms, const FlatBitset& static_initial_atoms) const;
};

class StripsActionEffectBuilder
{
private:
    std::reference_wrapper<FlatStripsActionEffect> m_builder;

public:
    explicit StripsActionEffectBuilder(FlatStripsActionEffect& builder);

    FlatBitset& get_positive_effects();
    FlatBitset& get_negative_effects();
};

class StripsActionEffect
{
private:
    std::reference_wrapper<const FlatStripsActionEffect> m_view;

public:
    explicit StripsActionEffect(const FlatStripsActionEffect& view);

    const FlatBitset& get_positive_effects() const;
    const FlatBitset& get_negative_effects() const;
};

class ConditionalEffectBuilder
{
private:
    std::reference_wrapper<FlatConditionalEffect> m_builder;

public:
    explicit ConditionalEffectBuilder(FlatConditionalEffect& builder);

    /* Precondition */

    template<PredicateCategory P>
    FlatIndexList& get_positive_precondition();

    template<PredicateCategory P>
    FlatIndexList& get_negative_precondition();

    /* Simple effects */

    FlatSimpleEffect& get_simple_effect();
};

class ConditionalEffect
{
private:
    std::reference_wrapper<const FlatConditionalEffect> m_view;

public:
    explicit ConditionalEffect(const FlatConditionalEffect& view);

    /* Precondition */

    template<PredicateCategory P>
    const FlatIndexList& get_positive_precondition() const;

    template<PredicateCategory P>
    const FlatIndexList& get_negative_precondition() const;

    /* Simple effects */
    const FlatSimpleEffect& get_simple_effect() const;

    template<DynamicPredicateCategory P>
    bool is_applicable(State state) const;

    bool is_dynamically_applicable(State state) const;

    bool is_statically_applicable(Problem problem) const;

    bool is_applicable(Problem problem, State state) const;
};

using ConditionalEffectList = std::vector<ConditionalEffect>;

class GroundActionBuilder
{
private:
    FlatAction m_builder;

public:
    FlatAction& get_data();
    const FlatAction& get_data() const;

    Index& get_index();
    Index& get_action_index();
    ContinuousCost& get_cost();
    FlatIndexList& get_objects();

    /* STRIPS part */
    FlatStripsActionPrecondition& get_strips_precondition();
    FlatStripsActionEffect& get_strips_effect();
    /* Conditional effects */
    FlatConditionalEffects& get_conditional_effects();
};

/**
 * Implementation class
 *
 * Reads the memory layout generated by the search node builder.
 */
class GroundAction
{
private:
    std::reference_wrapper<const FlatAction> m_view;

    static const GroundActionBuilder s_null_ground_action;

public:
    /// @brief Create a view on a DefaultAction.
    explicit GroundAction(const FlatAction& view);

    static GroundAction get_null_ground_action();

    Index get_index() const;
    Index get_action_index() const;
    ContinuousCost get_cost() const;
    const FlatIndexList& get_object_indices() const;

    /* STRIPS part */
    const FlatStripsActionPrecondition& get_strips_precondition() const;
    const FlatStripsActionEffect& get_strips_effect() const;
    /* Conditional effects */
    const FlatConditionalEffects& get_conditional_effects() const;

    bool is_dynamically_applicable(State state) const;

    bool is_statically_applicable(const FlatBitset& static_positive_atoms) const;

    bool is_applicable(Problem problem, State state) const;
};

extern bool operator==(GroundAction lhs, GroundAction rhs);

}

template<>
struct std::hash<mimir::GroundAction>
{
    size_t operator()(mimir::GroundAction element) const;
};

namespace mimir
{

/**
 * Mimir types
 */

using GroundActionList = std::vector<GroundAction>;
using GroundActionSet = std::unordered_set<GroundAction>;

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<FlatSimpleEffect, const PDDLFactories&>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<StripsActionPrecondition, const PDDLFactories&>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<StripsActionEffect, const PDDLFactories&>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<ConditionalEffect, const PDDLFactories&>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundAction, const PDDLFactories&>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<const PDDLFactories&, GroundAction>& data);

}

#endif
