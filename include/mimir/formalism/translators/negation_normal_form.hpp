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

#ifndef MIMIR_FORMALISM_TRANSLATORS_NEGATION_NORMAL_FORM_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_NEGATION_NORMAL_FORM_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{
/**
 * Translate formulas to negation normal form (NNF) using the following rules.
 *
 * 1. A -> B         =>  not A or B
 * 2. A <-> B        => (not A or B) and (A or not B)
 * 3. not (A or B)   => not A and not B
 * 4. not (A and B)  => not A or not B
 * 5. not (not A)    => A
 * 6. not exists x A => forall x not A
 * 7. not forall x A => exists x not A
 */
class NNFTranslator : public BaseTranslator<NNFTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<NNFTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    /**
     * Utility functions
     */
    class ConditionNotVisitor
    {
    private:
        NNFTranslator& m_translator;

    public:
        explicit ConditionNotVisitor(NNFTranslator& translator);

        /// @brief Default: wrap the translation into a not condition.
        template<typename ConditionImpl>
        Condition operator()(const ConditionImpl& condition)
        {
            return m_translator.m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(m_translator.translate(condition));
        }
        Condition operator()(const ConditionLiteralImpl& condition);
        Condition operator()(const ConditionNotImpl& condition);
        Condition operator()(const ConditionAndImpl& condition);
        Condition operator()(const ConditionOrImpl& condition);
        Condition operator()(const ConditionExistsImpl& condition);
        Condition operator()(const ConditionForallImpl& condition);
    };

    /**
     * Translate
     */
    Condition translate_impl(const ConditionImplyImpl& condition);
    Condition translate_impl(const ConditionNotImpl& condition);
    Problem translate_impl(const ProblemImpl& problem);

    Problem run_impl(const ProblemImpl& problem);

public:
    explicit NNFTranslator(PDDLFactories& pddl_factories);
};

}

#endif
