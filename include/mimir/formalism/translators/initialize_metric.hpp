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

#ifndef MIMIR_FORMALISM_TRANSLATORS_INITIALIZE_METRIC_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_INITIALIZE_METRIC_HPP_

#include "mimir/formalism/translators/base_cached_recurse.hpp"

namespace mimir
{

/**
 * If ":metric" is undefined, introduce metric "(min (total-cost))".
 * If additionally, action_costs is disabled, we must add (increase (total-cost) 1) to each conjunctive effect.
 */
class InitializeMetricTranslator : public BaseCachedRecurseTranslator<InitializeMetricTranslator>
{
private:
    /* Implement BaseCachedRecurseTranslator interface. */
    friend class BaseCachedRecurseTranslator<InitializeMetricTranslator>;

    // Provide default implementations
    using BaseCachedRecurseTranslator::prepare_impl;
    using BaseCachedRecurseTranslator::translate_impl;

    bool m_has_metric_defined;
    bool m_action_costs_enabled;

    void prepare_impl(loki::Domain domain);
    void prepare_impl(loki::Problem problem);

    loki::Effect translate_impl(loki::Effect effect);
    loki::Domain translate_impl(loki::Domain domain);
    loki::Problem translate_impl(loki::Problem problem);

public:
    explicit InitializeMetricTranslator(loki::PDDLRepositories& pddl_repositories);
};
}

#endif