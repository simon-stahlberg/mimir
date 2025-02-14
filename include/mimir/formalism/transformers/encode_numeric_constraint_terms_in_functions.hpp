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

#ifndef MIMIR_FORMALISM_TRANSFORMERS_ENCODE_NUMERIC_CONSTRAINT_TERMS_IN_FUNCTIONS_HPP_
#define MIMIR_FORMALISM_TRANSFORMERS_ENCODE_NUMERIC_CONSTRAINT_TERMS_IN_FUNCTIONS_HPP_

#include "mimir/formalism/transformers/base_recurse.hpp"

#include <unordered_map>

namespace mimir
{

/**
 * Encode term remapping needed in the lifted applicable action generator for checking vertex and edge consistency of numeric constraints.
 */
class EncodeNumericConstraintTermsInFunctions : public BaseRecurseTransformer<EncodeNumericConstraintTermsInFunctions>
{
private:
    /* Implement BaseRecurseTransformer interface. */
    friend class BaseRecurseTransformer<EncodeNumericConstraintTermsInFunctions>;

    // Provide default implementations
    using BaseRecurseTransformer<EncodeNumericConstraintTermsInFunctions>::prepare_impl;
    using BaseRecurseTransformer<EncodeNumericConstraintTermsInFunctions>::transform_impl;

    TermList m_numeric_constraint_terms;

    template<StaticOrFluentOrAuxiliary F>
    Function<F> transform_impl(Function<F> function);
    NumericConstraint transform_impl(NumericConstraint numeric_constraint);

    Problem run_impl(Problem problem);

public:
    EncodeNumericConstraintTermsInFunctions(PDDLRepositories& pddl_repositories);
};
}

#endif