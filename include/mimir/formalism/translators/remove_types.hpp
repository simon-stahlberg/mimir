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

#include "mimir/formalism/translators/base_cached_recurse.hpp"

namespace mimir
{
/**
 * Compile away types.
 */
class RemoveTypesTranslator : public BaseCachedRecurseTranslator<RemoveTypesTranslator>
{
private:
    /* Implement BaseCachedRecurseTranslator interface. */
    friend class BaseCachedRecurseTranslator<RemoveTypesTranslator>;

    // Provide default implementations
    using BaseCachedRecurseTranslator::prepare_impl;
    using BaseCachedRecurseTranslator::translate_impl;

    // Collect predicates that encode types
    std::unordered_map<loki::Type, loki::Predicate> m_type_to_predicates;

    /**
     * Translate
     */
    loki::Object translate_impl(loki::Object object);
    loki::Parameter translate_impl(loki::Parameter parameter);
    loki::Condition translate_impl(loki::ConditionExists condition);
    loki::Condition translate_impl(loki::ConditionForall condition);
    loki::Effect translate_impl(loki::EffectCompositeForall effect);
    loki::Action translate_impl(loki::Action action);
    loki::Axiom translate_impl(loki::Axiom axiom);
    loki::Domain translate_impl(loki::Domain domain);
    loki::Problem translate_impl(loki::Problem problem);

    loki::Problem run_impl(loki::Problem problem);

public:
    explicit RemoveTypesTranslator(loki::PDDLRepositories& pddl_repositories);
};

}
#endif
