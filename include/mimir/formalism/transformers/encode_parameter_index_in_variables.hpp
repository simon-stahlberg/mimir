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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_TRANSFORMERS_ENCODE_PARAMETER_INDEX_IN_VARIABLES_HPP_
#define MIMIR_FORMALISM_TRANSFORMERS_ENCODE_PARAMETER_INDEX_IN_VARIABLES_HPP_

#include "mimir/formalism/transformers/base.hpp"

#include <unordered_map>

namespace mimir
{

/**
 * Introduce an axiom for complicated goals, i.e., goals that are not conjunctions of literals.
 */
class EncodeParameterIndexInVariables : public BaseTransformer<EncodeParameterIndexInVariables>
{
private:
    /* Implement BaseTransformer interface. */
    friend class BaseTransformer<EncodeParameterIndexInVariables>;

    // Provide default implementations
    using BaseTransformer<EncodeParameterIndexInVariables>::prepare_impl;
    using BaseTransformer<EncodeParameterIndexInVariables>::transform_impl;

    std::unordered_map<Variable, size_t> m_variable_to_parameter_index;
    bool m_enable_encoding;

    Variable transform_impl(const VariableImpl& variable);

    // Do not rename within predicates or function skeleton
    Predicate<Static> transform_impl(const PredicateImpl<Static>& predicate);
    Predicate<Fluent> transform_impl(const PredicateImpl<Fluent>& predicate);
    Predicate<Derived> transform_impl(const PredicateImpl<Derived>& predicate);
    FunctionSkeleton transform_impl(const FunctionSkeletonImpl& function_skeleton);

    EffectUniversal transform_impl(const EffectUniversalImpl& effect);
    Axiom transform_impl(const AxiomImpl& axiom);
    Action transform_impl(const ActionImpl& action);

    Problem run_impl(const ProblemImpl& problem);

public:
    EncodeParameterIndexInVariables(PDDLFactories& pddl_factories);
};
}

#endif