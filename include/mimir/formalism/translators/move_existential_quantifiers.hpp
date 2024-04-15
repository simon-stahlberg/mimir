/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg and Simon Stahlberg
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

#ifndef MIMIR_FORMALISM_TRANSLATORS_MOVE_EXISTENTIAL_QUANTIFIERS_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_MOVE_EXISTENTIAL_QUANTIFIERS_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{

/**
 * 1. and(phi, exists(vars, psi))       =>  exists(vars, and(phi, psi))
 * 2. exists(vars, exists(vars', phi))  =>  exists(vars + vars', phi)
 */
class MoveExistentialQuantifiersTranslator : public BaseTranslator<MoveExistentialQuantifiersTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<MoveExistentialQuantifiersTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    /**
     * Push existential quantifiers to the root.
     *
     * 1. and(phi, exists(vars, psi))       =>  exists(vars, and(phi, psi))
     */
    loki::Condition translate_impl(const loki::ConditionAndImpl& condition);
    /**
     * Flatten existential quantifier
     *
     * 2. exists(vars, exists(vars', phi))  =>  exists(vars + vars', phi)
     */
    loki::Condition translate_impl(const loki::ConditionExistsImpl& condition);

    loki::Problem run_impl(const loki::ProblemImpl& problem);

public:
    explicit MoveExistentialQuantifiersTranslator(loki::PDDLFactories& pddl_factories);
};
}

#endif