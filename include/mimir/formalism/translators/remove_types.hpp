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

#ifndef MIMIR_FORMALISM_TRANSLATORS_TYPES_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_TYPES_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{
/**
 * Compile away types.
 */
class RemoveTypesTranslator : public BaseTranslator<RemoveTypesTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<RemoveTypesTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    /**
     * Translate
     */
    Object translate_impl(const ObjectImpl& object);
    Parameter translate_impl(const ParameterImpl& parameter);
    Condition translate_impl(const ConditionExistsImpl& condition);
    Condition translate_impl(const ConditionForallImpl& condition);
    Effect translate_impl(const EffectConditionalForallImpl& effect);
    Action translate_impl(const ActionImpl& action);
    Domain translate_impl(const DomainImpl& domain);
    Problem translate_impl(const ProblemImpl& problem);

    Problem run_impl(const ProblemImpl& problem);

public:
    explicit RemoveTypesTranslator(PDDLFactories& pddl_factories);
};

}
#endif
