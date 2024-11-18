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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_EVALUATION_CONTEXT_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_EVALUATION_CONTEXT_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/constructor_interface.hpp"
#include "mimir/languages/description_logics/denotation_repositories.hpp"
#include "mimir/search/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl
{

class EvaluationContext
{
private:
    /* Memory */
    const PDDLRepositories& m_pddl_repositories;
    Problem m_problem;
    State m_state;
    ConstructorTagToDenotationType& m_denotation_builder;
    ConstructorTagToDenotationRepository& m_denotation_repository;

public:
    EvaluationContext(const PDDLRepositories& pddl_repositories,
                      Problem problem,
                      State state,
                      ConstructorTagToDenotationType& ref_denotation_builder,
                      ConstructorTagToDenotationRepository& ref_denotation_repository);

    /**
     * Getters
     */

    const PDDLRepositories& get_pddl_repositories() const;

    Problem get_problem() const;

    State get_state() const;

    template<ConstructorTag D>
    DenotationImpl<D>& get_denotation_builder();

    template<ConstructorTag D>
    DenotationRepository<D>& get_denotation_repository();
};
}

#endif
