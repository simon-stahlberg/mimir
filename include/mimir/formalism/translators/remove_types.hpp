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

#ifndef MIMIR_FORMALISM_TRANSLATORS_REMOVE_TYPES_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_REMOVE_TYPES_HPP_

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

    // Collect predicates that encode types
    std::unordered_map<loki::Type, loki::Predicate> m_type_to_predicates;

    /**
     * Translate
     */
    loki::Object translate_impl(const loki::ObjectImpl& object);
    loki::Parameter translate_impl(const loki::ParameterImpl& parameter);
    loki::Condition translate_impl(const loki::ConditionExistsImpl& condition);
    loki::Condition translate_impl(const loki::ConditionForallImpl& condition);
    loki::Effect translate_impl(const loki::EffectConditionalForallImpl& effect);
    loki::Action translate_impl(const loki::ActionImpl& action);
    loki::Domain translate_impl(const loki::DomainImpl& domain);
    loki::Problem translate_impl(const loki::ProblemImpl& problem);

    loki::Problem run_impl(const loki::ProblemImpl& problem);

public:
    explicit RemoveTypesTranslator(loki::PDDLFactories& pddl_factories);
};

}
#endif
