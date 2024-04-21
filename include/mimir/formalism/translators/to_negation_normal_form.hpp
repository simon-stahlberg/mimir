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

#ifndef MIMIR_FORMALISM_TRANSLATORS_TO_NEGATION_NORMAL_FORM_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_TO_NEGATION_NORMAL_FORM_HPP_

#include "mimir/formalism/translators/base_cached_recurse.hpp"

namespace mimir
{
/**
 * Translate formulas to negation normal form (NNF) in top-down manner using the following rules.
 * Flattens conjunctions, disjunctions, existential quantifiers, and universal quantifiers
 * before pushing the negation and afterwards to ensure that emerged structures are as simple as possible.
 *
 * 1. A -> B                           =>  not A or B
 * 2. A <-> B                          => (not A or B) and (A or not B)
 * 3. not (A or B)                     => not A and not B
 * 4. not (A and B)                    => not A or not B
 * 5. not (not A)                      => A
 * 6. not exists x A                   => forall x not A
 * 7. not forall x A                   => exists x not A
 * 8. not (A -> B)                     => not (not A or B)                => A and not B
 * 9. A and (B and C)                  => A and B and C
 * 10. A or (B or C)                    => A or B or C
 * 11. exists(vars1, exists(vars2, A)) => exists(vars1+vars2, A)
 * 12. forall(vars1, forall(vars2, A)) => forall(vars1+vars2, A)
 */
class ToNNFTranslator : public BaseCachedRecurseTranslator<ToNNFTranslator>
{
private:
    /* Implement BaseCachedRecurseTranslator interface. */
    friend class BaseCachedRecurseTranslator<ToNNFTranslator>;

    // Provide default implementations
    using BaseCachedRecurseTranslator::prepare_impl;
    using BaseCachedRecurseTranslator::translate_impl;

    /**
     * Translate
     */

    /**
     * Eliminate implication
     *
     * 1. A -> B   =>  not A or B
     * 2. A <-> B  =>  (not A or B) and (A or not B)
     */
    loki::Condition translate_impl(const loki::ConditionImplyImpl& condition);
    /**
     * Push negation inwards.
     *
     * 3. not (A or B)    =>  not A and not B
     * 4. not (A and B)   =>  not A or not B
     * 5. not (not A)     =>  A
     * 6. not exists x A  =>  forall x not A
     * 7. not forall x A  =>  exists x not A
     * 8. not (A -> B)    =>  not (not A or B)  =>  A and not B
     */
    loki::Condition translate_impl(const loki::ConditionNotImpl& condition);
    /**
     * Flatten conjunctions.
     *
     * 9. A and (B and C)  =>  A and B and C
     */
    loki::Condition translate_impl(const loki::ConditionAndImpl& condition);
    /**
     * Flatten disjunctions.
     *
     * 10. A or (B or C)  =>  A or B or C
     */
    loki::Condition translate_impl(const loki::ConditionOrImpl& condition);
    /**
     * Flatten existential quantifiers.
     *
     * 11. exists(vars1, exists(vars2, A))  =>  exists(vars1+vars2, A)
     */
    loki::Condition translate_impl(const loki::ConditionExistsImpl& condition);
    /**
     * Flatten universal quantifiers.
     *
     * 12. forall(vars1, forall(vars2, A))  =>  forall(vars1+vars2, A)
     */
    loki::Condition translate_impl(const loki::ConditionForallImpl& condition);

    loki::Problem run_impl(const loki::ProblemImpl& problem);

public:
    explicit ToNNFTranslator(loki::PDDLFactories& pddl_factories);
};

}

#endif
