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

/* GroundConditionNumeric */
loki::BinaryComparatorEnum& GroundConditionNumeric::get_binary_comparator() { return m_binary_comparator; }
loki::BinaryComparatorEnum GroundConditionNumeric::get_binary_comparator() const { return m_binary_comparator; }

FlatExternalPtr<const GroundFunctionExpressionImpl>& GroundConditionNumeric::get_left_function_expression() { return m_left_function_expression; }
FlatExternalPtr<const GroundFunctionExpressionImpl> GroundConditionNumeric::get_left_function_expression() const { return m_left_function_expression; }

FlatExternalPtr<const GroundFunctionExpressionImpl>& GroundConditionNumeric::get_right_function_expression() { return m_right_function_expression; }
FlatExternalPtr<const GroundFunctionExpressionImpl> GroundConditionNumeric::get_right_function_expression() const { return m_right_function_expression; }

/* GroundConditionStrips */

template<PredicateTag P>
FlatIndexList& GroundConditionStrips::get_positive_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        assert(std::is_sorted(m_positive_static_atoms.cbegin(), m_positive_static_atoms.cend()));
        return m_positive_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        assert(std::is_sorted(m_positive_fluent_atoms.cbegin(), m_positive_fluent_atoms.cend()));
        return m_positive_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        assert(std::is_sorted(m_positive_derived_atoms.cbegin(), m_positive_derived_atoms.cend()));
        return m_positive_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template FlatIndexList& GroundConditionStrips::get_positive_precondition<Static>();
template FlatIndexList& GroundConditionStrips::get_positive_precondition<Fluent>();
template FlatIndexList& GroundConditionStrips::get_positive_precondition<Derived>();

template<PredicateTag P>
const FlatIndexList& GroundConditionStrips::get_positive_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        assert(std::is_sorted(m_positive_static_atoms.begin(), m_positive_static_atoms.end()));
        return m_positive_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        assert(std::is_sorted(m_positive_fluent_atoms.begin(), m_positive_fluent_atoms.end()));
        return m_positive_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        assert(std::is_sorted(m_positive_derived_atoms.begin(), m_positive_derived_atoms.end()));
        return m_positive_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatIndexList& GroundConditionStrips::get_positive_precondition<Static>() const;
template const FlatIndexList& GroundConditionStrips::get_positive_precondition<Fluent>() const;
template const FlatIndexList& GroundConditionStrips::get_positive_precondition<Derived>() const;

template<PredicateTag P>
FlatIndexList& GroundConditionStrips::get_negative_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        assert(std::is_sorted(m_negative_static_atoms.cbegin(), m_negative_static_atoms.cend()));
        return m_negative_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        assert(std::is_sorted(m_negative_fluent_atoms.cbegin(), m_negative_fluent_atoms.cend()));
        return m_negative_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        assert(std::is_sorted(m_negative_derived_atoms.cbegin(), m_negative_derived_atoms.cend()));
        return m_negative_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template FlatIndexList& GroundConditionStrips::get_negative_precondition<Static>();
template FlatIndexList& GroundConditionStrips::get_negative_precondition<Fluent>();
template FlatIndexList& GroundConditionStrips::get_negative_precondition<Derived>();

template<PredicateTag P>
const FlatIndexList& GroundConditionStrips::get_negative_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        assert(std::is_sorted(m_negative_static_atoms.begin(), m_negative_static_atoms.end()));
        return m_negative_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        assert(std::is_sorted(m_negative_fluent_atoms.begin(), m_negative_fluent_atoms.end()));
        return m_negative_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        assert(std::is_sorted(m_negative_derived_atoms.begin(), m_negative_derived_atoms.end()));
        return m_negative_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatIndexList& GroundConditionStrips::get_negative_precondition<Static>() const;
template const FlatIndexList& GroundConditionStrips::get_negative_precondition<Fluent>() const;
template const FlatIndexList& GroundConditionStrips::get_negative_precondition<Derived>() const;

GroundConditionNumericList& GroundConditionStrips::get_numeric_constraints() { return m_numeric_constraints; }

const GroundConditionNumericList& GroundConditionStrips::get_numeric_constraints() const { return m_numeric_constraints; }

template<PredicateTag P>
bool GroundConditionStrips::is_applicable(const FlatBitset& atoms) const
{
    return is_supseteq(atoms, get_positive_precondition<P>())  //
           && are_disjoint(atoms, get_negative_precondition<P>());
}

template bool GroundConditionStrips::is_applicable<Static>(const FlatBitset& atoms) const;
template bool GroundConditionStrips::is_applicable<Fluent>(const FlatBitset& atoms) const;
template bool GroundConditionStrips::is_applicable<Derived>(const FlatBitset& atoms) const;

bool GroundConditionStrips::is_applicable(const FlatDoubleList& fluent_numeric_variables) const
{
    for (const auto& constraint : get_numeric_constraints())
    {
        if (!evaluate(constraint, fluent_numeric_variables))
            return false;
    }
    return true;
}

bool GroundConditionStrips::is_dynamically_applicable(const DenseState& dense_state) const
{  //
    return is_applicable<Fluent>(dense_state.get_atoms<Fluent>()) && is_applicable<Derived>(dense_state.get_atoms<Derived>())
           && is_applicable(dense_state.get_numeric_variables());
}

bool GroundConditionStrips::is_statically_applicable(const FlatBitset& static_positive_atoms) const { return is_applicable<Static>(static_positive_atoms); }

bool GroundConditionStrips::is_applicable(Problem problem, const DenseState& dense_state) const
{
    return is_dynamically_applicable(dense_state) && is_statically_applicable(problem->get_static_initial_positive_atoms_bitset());
}

/* GroundEffectNumeric */
template<DynamicFunctionTag F>
loki::AssignOperatorEnum& GroundEffectNumeric<F>::get_assign_operator()
{
    return m_assign_operator;
}
template<DynamicFunctionTag F>
loki::AssignOperatorEnum GroundEffectNumeric<F>::get_assign_operator() const
{
    return m_assign_operator;
}

template<DynamicFunctionTag F>
FlatExternalPtr<const GroundFunctionImpl<F>>& GroundEffectNumeric<F>::get_function()
{
    return m_function;
}
template<DynamicFunctionTag F>
FlatExternalPtr<const GroundFunctionImpl<F>> GroundEffectNumeric<F>::get_function() const
{
    return m_function;
}

template<DynamicFunctionTag F>
FlatExternalPtr<const GroundFunctionExpressionImpl>& GroundEffectNumeric<F>::get_function_expression()
{
    return m_function_expression;
}
template<DynamicFunctionTag F>
FlatExternalPtr<const GroundFunctionExpressionImpl> GroundEffectNumeric<F>::get_function_expression() const
{
    return m_function_expression;
}

template struct GroundEffectNumeric<Fluent>;
template struct GroundEffectNumeric<Auxiliary>;

/* GroundEffectStrips */

FlatIndexList& GroundEffectStrips::get_positive_effects() { return m_positive_effects; }

const FlatIndexList& GroundEffectStrips::get_positive_effects() const { return m_positive_effects; }

FlatIndexList& GroundEffectStrips::get_negative_effects() { return m_negative_effects; }

const FlatIndexList& GroundEffectStrips::get_negative_effects() const { return m_negative_effects; }

template<DynamicFunctionTag F>
GroundEffectNumericList<F>& GroundEffectStrips::get_numeric_effects()
{
    if constexpr (std::is_same_v<F, Fluent>)
    {
        return m_fluent_numeric_effects;
    }
    else if constexpr (std::is_same_v<F, Auxiliary>)
    {
        return m_auxiliary_numeric_effects;
    }
    else
    {
        static_assert(dependent_false<F>::value, "Missing implementation for DynamicFunctionTag.");
    }
}

template GroundEffectNumericList<Fluent>& GroundEffectStrips::get_numeric_effects<Fluent>();
template GroundEffectNumericList<Auxiliary>& GroundEffectStrips::get_numeric_effects<Auxiliary>();

template<DynamicFunctionTag F>
const GroundEffectNumericList<F>& GroundEffectStrips::get_numeric_effects() const
{
    if constexpr (std::is_same_v<F, Fluent>)
    {
        return m_fluent_numeric_effects;
    }
    else if constexpr (std::is_same_v<F, Auxiliary>)
    {
        return m_auxiliary_numeric_effects;
    }
    else
    {
        static_assert(dependent_false<F>::value, "Missing implementation for DynamicFunctionTag.");
    }
}

template const GroundEffectNumericList<Fluent>& GroundEffectStrips::get_numeric_effects<Fluent>() const;
template const GroundEffectNumericList<Auxiliary>& GroundEffectStrips::get_numeric_effects<Auxiliary>() const;

/* GroundEffectConditional */

/* Precondition */
GroundConditionStrips& GroundEffectConditional::get_strips_precondition() { return m_strips_condition; }

const GroundConditionStrips& GroundEffectConditional::get_strips_precondition() const { return m_strips_condition; }

/* Effect */
GroundEffectStrips& GroundEffectConditional::get_strips_effect() { return m_strips_effect; }

const GroundEffectStrips& GroundEffectConditional::get_strips_effect() const { return m_strips_effect; }

template<PredicateTag P>
bool GroundEffectConditional::is_applicable(const FlatBitset& atoms) const
{
    return m_strips_condition.is_applicable<P>(atoms);
}

template bool GroundEffectConditional::is_applicable<Static>(const FlatBitset& atoms) const;
template bool GroundEffectConditional::is_applicable<Fluent>(const FlatBitset& atoms) const;
template bool GroundEffectConditional::is_applicable<Derived>(const FlatBitset& atoms) const;

bool GroundEffectConditional::is_applicable(const FlatDoubleList& fluent_numeric_variables) const
{
    return m_strips_condition.is_applicable(fluent_numeric_variables);
}

bool GroundEffectConditional::is_dynamically_applicable(const DenseState& dense_state) const
{
    return m_strips_condition.is_dynamically_applicable(dense_state);
}

bool GroundEffectConditional::is_statically_applicable(Problem problem) const
{
    return m_strips_condition.is_statically_applicable(problem->get_static_initial_positive_atoms_bitset());
}

bool GroundEffectConditional::is_applicable(Problem problem, const DenseState& dense_state) const
{
    return m_strips_condition.is_applicable(problem, dense_state);
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
 * Utils
 */

template<DynamicFunctionTag F>
ContinuousCost evaluate(GroundEffectNumeric<F> effect, const FlatDoubleList& fluent_numeric_variables, const FlatDoubleList& auxiliary_numeric_variables)
{
    auto old_value = ContinuousCost(0);
    if constexpr (std::is_same_v<F, Fluent>)
    {
        if (effect.get_function()->get_index() >= fluent_numeric_variables.size())
        {
            throw std::logic_error("evaluate(effect, fluent_numeric_variables, auxiliary_numeric_variables): undefined fluent function value "
                                   + to_string(effect.get_function()) + ".");
        }
        old_value = fluent_numeric_variables.at(effect.get_function()->get_index());
    }
    else if constexpr (std::is_same_v<F, Auxiliary>)
    {
        if (effect.get_function()->get_index() >= auxiliary_numeric_variables.size())
        {
            throw std::logic_error("evaluate(effect, fluent_numeric_variables, auxiliary_numeric_variables): undefined auxiliary function value "
                                   + to_string(effect.get_function()) + ".");
        }
        old_value = auxiliary_numeric_variables.at(effect.get_function()->get_index());
    }

    const auto new_value = evaluate(effect.get_function_expression().get(), fluent_numeric_variables, auxiliary_numeric_variables);

    switch (effect.get_assign_operator())
    {
        case loki::AssignOperatorEnum::ASSIGN:
        {
            return new_value;
        }
        case loki::AssignOperatorEnum::DECREASE:
        {
            return old_value - new_value;
        }
        case loki::AssignOperatorEnum::INCREASE:
        {
            return old_value + new_value;
        }
        case loki::AssignOperatorEnum::SCALE_DOWN:
        {
            return old_value / new_value;
        }
        case loki::AssignOperatorEnum::SCALE_UP:
        {
            return old_value * new_value;
        }
        default:
        {
            throw std::logic_error("evaluate(effect, fluent_numeric_variables, auxiliary_numeric_variables): Unexpected loki::AssignOperatorEnum.");
        }
    }
}

bool evaluate(GroundConditionNumeric effect, const FlatDoubleList& fluent_numeric_variables)
{
    const auto left_value = evaluate(effect.get_left_function_expression().get(), fluent_numeric_variables);
    const auto right_value = evaluate(effect.get_right_function_expression().get(), fluent_numeric_variables);

    switch (effect.get_binary_comparator())
    {
        case loki::BinaryComparatorEnum::EQUAL:
        {
            return left_value == right_value;
        }
        case loki::BinaryComparatorEnum::GREATER:
        {
            return left_value > right_value;
        }
        case loki::BinaryComparatorEnum::GREATER_EQUAL:
        {
            return left_value >= right_value;
        }
        case loki::BinaryComparatorEnum::LESS:
        {
            return left_value < right_value;
        }
        case loki::BinaryComparatorEnum::LESS_EQUAL:
        {
            return left_value <= right_value;
        }
        default:
        {
            throw std::logic_error("evaluate(effect, fluent_numeric_variables, auxiliary_numeric_variables): Unexpected loki::BinaryComparatorEnum.");
        }
    }
}

template double evaluate(GroundEffectNumeric<Fluent> effect, const FlatDoubleList& fluent_numeric_variables, const FlatDoubleList& auxiliary_numeric_variables);
template double
evaluate(GroundEffectNumeric<Auxiliary> effect, const FlatDoubleList& fluent_numeric_variables, const FlatDoubleList& auxiliary_numeric_variables);

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& os, const GroundConditionNumeric& element)
{
    os << "(" << to_string(element.get_binary_comparator()) << " " << element.get_left_function_expression() << " " << element.get_right_function_expression()
       << ")";

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundConditionStrips, const PDDLRepositories&>& data)
{
    // TODO(numerical): add numeric constraints
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

    os << "positive static precondition=" << positive_static_precondition << ", " << "negative static precondition=" << negative_static_precondition << ", "
       << "positive fluent precondition=" << positive_fluent_precondition << ", " << "negative fluent precondition=" << negative_fluent_precondition << ", "
       << "positive derived precondition=" << positive_derived_precondition << ", " << "negative derived precondition=" << negative_derived_precondition << ", "
       << "numeric constraints=" << strips_precondition_proxy.get_numeric_constraints();

    return os;
}

template<DynamicFunctionTag F>
std::ostream& operator<<(std::ostream& os, const GroundEffectNumeric<F>& element)
{
    os << "(" << to_string(element.get_assign_operator()) << " " << element.get_function() << " " << element.get_function_expression() << ")";

    return os;
}

template std::ostream& operator<<(std::ostream& os, const GroundEffectNumeric<Fluent>& data);
template std::ostream& operator<<(std::ostream& os, const GroundEffectNumeric<Auxiliary>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundEffectStrips, const PDDLRepositories&>& data)
{
    // TODO(numerical): add numeric effects
    const auto& [strips_effect_proxy, pddl_repositories] = data;

    const auto& positive_effect_bitset = strips_effect_proxy.get_positive_effects();
    const auto& negative_effect_bitset = strips_effect_proxy.get_negative_effects();
    const auto& fluent_numeric_effects = strips_effect_proxy.get_numeric_effects<Fluent>();
    const auto& auxiliary_numeric_effects = strips_effect_proxy.get_numeric_effects<Auxiliary>();

    auto positive_simple_effects = GroundAtomList<Fluent> {};
    auto negative_simple_effects = GroundAtomList<Fluent> {};

    pddl_repositories.get_ground_atoms_from_indices<Fluent>(positive_effect_bitset, positive_simple_effects);
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(negative_effect_bitset, negative_simple_effects);

    os << "delete effects=" << negative_simple_effects << ", " << "add effects=" << positive_simple_effects
       << ", fluent numeric effects=" << fluent_numeric_effects << ", auxiliary numeric effects=" << auxiliary_numeric_effects;

    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundEffectConditional, const PDDLRepositories&>& data)
{
    const auto& [cond_effect_proxy, pddl_repositories] = data;

    os << std::make_tuple(cond_effect_proxy.get_strips_precondition(), std::cref(pddl_repositories)) << ", "
       << std::make_tuple(cond_effect_proxy.get_strips_effect(), std::cref(pddl_repositories));

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
