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

#ifndef MIMIR_FORMALISM_TRANSLATORS_SPLIT_DISJUNCTIVE_CONDITIONS_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_SPLIT_DISJUNCTIVE_CONDITIONS_HPP_

#include "mimir/formalism/translators/base_cached_recurse.hpp"

namespace mimir
{

/**
 * Split actions and axioms at the disjunction at the root of a condition
 * and conditional effects using the following rules:
 *
 * 1. (phi or psi) > e  => (phi > e) and (psi > e)
 * 2. <(phi or psi), e>  => <phi, e> and <psi, e>
 */
class SplitDisjunctiveConditionsTranslator : public BaseCachedRecurseTranslator<SplitDisjunctiveConditionsTranslator>
{
private:
    /* Implement BaseCachedRecurseTranslator interface. */
    friend class BaseCachedRecurseTranslator<SplitDisjunctiveConditionsTranslator>;

    // Provide default implementations
    using BaseCachedRecurseTranslator::prepare_impl;
    using BaseCachedRecurseTranslator::translate_impl;

    /**
     * Split conditional effects
     *
     * 1. (phi or psi) > e  => (phi > e) and (psi > e)
     */
    loki::Effect translate_impl(const loki::EffectConditionalWhenImpl& effect);
    /**
     * Split actions and actions at conditions
     *
     * 2. <(phi or psi), e>  => <phi, e> and <psi, e>
     */
    loki::Domain translate_impl(const loki::DomainImpl& domain);
    loki::Problem translate_impl(const loki::ProblemImpl& problem);

    loki::Problem run_impl(const loki::ProblemImpl& problem);

public:
    explicit SplitDisjunctiveConditionsTranslator(loki::PDDLFactories& pddl_factories);
};

}

#endif
