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
 * 1. A -> B                           =>  not A or B
 * 2. A <-> B                          => (not A or B) and (A or not B)
 * 3. not (A or B)                     => not A and not B
 * 4. not (A and B)                    => not A or not B
 * 5. not (not A)                      => A
 * 6. not exists x A                   => forall x not A
 * 7. not forall x A                   => exists x not A
 * 8. A and (B and C)                  => A and B and C
 * 9. A or (B or C)                    => A or B or C
 * 10. exists(vars1, exists(vars2, A)) => exists(vars1+vars2, A)
 * 11. forall(vars1, forall(vars2, A)) => forall(vars1+vars2, A)
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
     * Translate
     */
    Condition translate_impl(const ConditionImplyImpl& condition);
    Condition translate_impl(const ConditionNotImpl& condition);
    Condition translate_impl(const ConditionImpl& condition);
    Condition translate_impl(const ConditionAndImpl& condition);
    Condition translate_impl(const ConditionOrImpl& condition);
    Condition translate_impl(const ConditionExistsImpl& condition);
    Condition translate_impl(const ConditionForallImpl& condition);

    Problem run_impl(const ProblemImpl& problem);

public:
    explicit NNFTranslator(PDDLFactories& pddl_factories);
};

}

#endif
