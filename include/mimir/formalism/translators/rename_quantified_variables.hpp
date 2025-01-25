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

#ifndef MIMIR_FORMALISM_TRANSLATORS_RENAME_QUANTIFIED_VARIABLES_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_RENAME_QUANTIFIED_VARIABLES_HPP_

#include "mimir/formalism/translators/base_recurse.hpp"

#include <deque>

namespace mimir
{

/**
 * Rename all variables to be able to move quantifiers around.
 */
class RenameQuantifiedVariablesTranslator : public BaseRecurseTranslator<RenameQuantifiedVariablesTranslator>
{
private:
    /* Implement BaseRecurseTranslator interface. */
    friend class BaseRecurseTranslator<RenameQuantifiedVariablesTranslator>;

    // Provide default implementations
    // Note: for context dependent translations, we cannot use the BaseCachedRecurseTranslator.
    using BaseRecurseTranslator::prepare_impl;
    using BaseRecurseTranslator::translate_impl;

    /* Translate step */

    std::unordered_map<loki::Variable, size_t> m_num_quantifications;

    bool m_renaming_enabled;

    loki::Variable translate_impl(loki::Variable variable);

    /**
     * Keep variable names of parameters in these constructs, i.e., turn off renaming
     */
    loki::Predicate translate_impl(loki::Predicate predicate);
    loki::FunctionSkeleton translate_impl(loki::FunctionSkeleton function_skeleton);

    /**
     * Rename variables names of parameters in these constructs, i.e., turn on renaming
     */
    loki::Condition translate_impl(loki::ConditionExists condition);
    loki::Condition translate_impl(loki::ConditionForall condition);
    loki::Effect translate_impl(loki::EffectCompositeForall effect);
    loki::Action translate_impl(loki::Action action);
    loki::Axiom translate_impl(loki::Axiom axiom);

    loki::Problem run_impl(loki::Problem problem);

public:
    explicit RenameQuantifiedVariablesTranslator(loki::PDDLRepositories& pddl_repositories);
};
}

#endif