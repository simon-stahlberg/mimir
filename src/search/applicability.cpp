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

#include "mimir/search/applicability.hpp"

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/conjunctive_condition.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/ground_conjunctive_condition.hpp"
#include "mimir/formalism/ground_effects.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/dense_state.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{

/**
 * ConjunctiveCondition
 */

template<PredicateTag P>
static bool nullary_literals_hold(const GroundLiteralList<P>& literals, const FlatBitset& atom_indices)
{
    for (const auto& literal : literals)
    {
        assert(literal->get_atom()->get_arity() == 0);

        if (literal->is_negated() == atom_indices.get(literal->get_atom()->get_index()))
        {
            return false;
        }
    }

    return true;
}

bool nullary_conditions_hold(ConjunctiveCondition conjunctive_condition, const DenseState& dense_state)
{
    return nullary_literals_hold(conjunctive_condition->get_nullary_ground_literals<Fluent>(), dense_state.get_atoms<Fluent>())
           && nullary_literals_hold(conjunctive_condition->get_nullary_ground_literals<Derived>(), dense_state.get_atoms<Derived>());
}

/**
 * GroundConjunctiveCondition
 */

template<PredicateTag P>
bool is_applicable(const GroundConjunctiveCondition& conjunctive_condition, const FlatBitset& atoms)
{
    return is_supseteq(atoms, conjunctive_condition.template get_positive_precondition<P>())  //
           && are_disjoint(atoms, conjunctive_condition.template get_negative_precondition<P>());
}

template bool is_applicable<Static>(const GroundConjunctiveCondition& conjunctive_condition, const FlatBitset& atoms);
template bool is_applicable<Fluent>(const GroundConjunctiveCondition& conjunctive_condition, const FlatBitset& atoms);
template bool is_applicable<Derived>(const GroundConjunctiveCondition& conjunctive_condition, const FlatBitset& atoms);

bool is_applicable(const GroundConjunctiveCondition& conjunctive_condition, const FlatDoubleList& fluent_numeric_variables)
{
    for (const auto& constraint : conjunctive_condition.get_numeric_constraints())
    {
        if (!evaluate(constraint.get(), fluent_numeric_variables))
            return false;
    }
    return true;
}

bool is_dynamically_applicable(const GroundConjunctiveCondition& conjunctive_condition, const DenseState& dense_state)
{
    return is_applicable<Fluent>(conjunctive_condition, dense_state.get_atoms<Fluent>())
           && is_applicable<Derived>(conjunctive_condition, dense_state.get_atoms<Derived>())
           && is_applicable(conjunctive_condition, dense_state.get_numeric_variables());
}

bool is_statically_applicable(const GroundConjunctiveCondition& conjunctive_condition, const FlatBitset& static_positive_atoms)
{
    return is_applicable<Static>(conjunctive_condition, static_positive_atoms);
}

bool is_applicable(const GroundConjunctiveCondition& conjunctive_condition, Problem problem, const DenseState& dense_state)
{
    return is_dynamically_applicable(conjunctive_condition, dense_state)
           && is_statically_applicable(conjunctive_condition, problem->get_static_initial_positive_atoms_bitset());
}

/**
 * GroundConjunctiveEffect
 */

template<FunctionTag F>
bool is_applicable(const GroundConjunctiveEffect& conjunctive_effect, const FlatBitset& atoms)
{
}

bool is_applicable(const GroundConjunctiveEffect& conjunctive_effect, const FlatDoubleList& fluent_numeric_variables)
{
    // return is_applicable(conjunctive_effect.get_numeric_effects<Fluent>(), fluent_numeric_variables)
    //        && is_applicable(conjunctive_effect.get_numeric_effects<Auxiliary>(), fluent_numeric_variables);
    // TODO
}

bool is_dynamically_applicable(const GroundConjunctiveEffect& conjunctive_effect, const DenseState& dense_state)
{
    // TODO
}

bool is_statically_applicable(const GroundConjunctiveEffect& conjunctive_effect, Problem problem)
{
    // TODO
}

bool is_applicable(const GroundConjunctiveEffect& conjunctive_effect, Problem problem, const DenseState& dense_state)
{
    // TODO
}

/**
 * GroundConditionalEffect
 */

template<PredicateTag P>
bool is_applicable(const GroundConditionalEffect& conditional_effect, const FlatBitset& atoms)
{
    return is_applicable<P>(conditional_effect.get_conjunctive_condition(), atoms);
}

template bool is_applicable<Static>(const GroundConditionalEffect& conditional_effect, const FlatBitset& atoms);
template bool is_applicable<Fluent>(const GroundConditionalEffect& conditional_effect, const FlatBitset& atoms);
template bool is_applicable<Derived>(const GroundConditionalEffect& conditional_effect, const FlatBitset& atoms);

bool is_applicable(const GroundConditionalEffect& conditional_effect, const FlatDoubleList& fluent_numeric_variables)
{
    return is_applicable(conditional_effect.get_conjunctive_condition(), fluent_numeric_variables)
           && is_applicable(conditional_effect.get_conjunctive_effect(), fluent_numeric_variables);
}

bool is_dynamically_applicable(const GroundConditionalEffect& conditional_effect, const DenseState& dense_state)
{
    return is_dynamically_applicable(conditional_effect.get_conjunctive_condition(), dense_state);
}

bool is_statically_applicable(const GroundConditionalEffect& conditional_effect, Problem problem)
{
    return is_statically_applicable(conditional_effect.get_conjunctive_condition(), problem->get_static_initial_positive_atoms_bitset());
}

bool is_applicable(const GroundConditionalEffect& conditional_effect, Problem problem, const DenseState& dense_state)
{
    return is_applicable(conditional_effect.get_conjunctive_condition(), problem, dense_state);
}

/**
 * GroundAction
 */

bool is_dynamically_applicable(GroundAction action, const DenseState& dense_state)
{
    return is_dynamically_applicable(action->get_conjunctive_condition(), dense_state);
}

bool is_statically_applicable(GroundAction action, const FlatBitset& static_positive_atoms)
{
    return is_statically_applicable(action->get_conjunctive_condition(), static_positive_atoms);
}

bool is_applicable(GroundAction action, Problem problem, const DenseState& dense_state)
{
    return is_applicable(action->get_conjunctive_condition(), problem, dense_state);
}

/**
 * GroundAxiom
 */

bool is_dynamically_applicable(GroundAxiom axiom, const DenseState& dense_state)
{
    return is_dynamically_applicable(axiom->get_conjunctive_condition(), dense_state);
}

bool is_statically_applicable(GroundAxiom axiom, const FlatBitset& static_positive_atoms)
{
    return is_statically_applicable(axiom->get_conjunctive_condition(), static_positive_atoms);
}

bool is_applicable(GroundAxiom axiom, Problem problem, const DenseState& dense_state)
{
    return is_applicable(axiom->get_conjunctive_condition(), problem, dense_state);
}
}
