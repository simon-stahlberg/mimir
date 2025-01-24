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

#include "mimir/formalism/transformers/base_recurse.hpp"

#include <unordered_map>

namespace mimir
{

/**
 * Introduce an axiom for complicated goals, i.e., goals that are not conjunctions of literals.
 */
class EncodeParameterIndexInVariables : public BaseRecurseTransformer<EncodeParameterIndexInVariables>
{
private:
    /* Implement BaseRecurseTransformer interface. */
    friend class BaseRecurseTransformer<EncodeParameterIndexInVariables>;

    // Provide default implementations
    using BaseRecurseTransformer<EncodeParameterIndexInVariables>::prepare_impl;
    using BaseRecurseTransformer<EncodeParameterIndexInVariables>::transform_impl;

    std::unordered_map<Variable, size_t> m_variable_to_parameter_index;
    bool m_enable_encoding;

    Variable transform_impl(Variable variable);

    // Do not rename within predicates or function skeleton
    template<PredicateTag P>
    Predicate<P> transform_impl(Predicate<P> predicate);
    template<FunctionTag F>
    FunctionSkeleton<F> transform_impl(FunctionSkeleton<F> function_skeleton);

    ConditionalEffect transform_impl(ConditionalEffect effect);
    Axiom transform_impl(Axiom axiom);
    Action transform_impl(Action action);

    Problem run_impl(Problem problem);

public:
    EncodeParameterIndexInVariables(PDDLRepositories& pddl_repositories);
};
}

#endif