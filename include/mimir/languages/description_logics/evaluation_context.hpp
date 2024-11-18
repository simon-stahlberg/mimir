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

#include "mimir/common/concepts.hpp"
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
    const PDDLFactories& m_pddl_factories;
    Problem m_problem;
    State m_state;
    DenotationImpl<Concept>& m_concept_denotation_builder;
    DenotationImpl<Role>& m_role_denotation_builder;
    DenotationRepository<Concept>& m_concept_denotation_repository;
    DenotationRepository<Role>& m_role_denotation_repository;

public:
    EvaluationContext(const PDDLFactories& pddl_factories,
                      Problem problem,
                      State state,
                      DenotationImpl<Concept>& ref_concept_denotation_builder,
                      DenotationImpl<Role>& ref_role_denotation_builder,
                      DenotationRepository<Concept>& ref_concept_denotation_repository,
                      DenotationRepository<Role>& ref_role_denotation_repository);

    /**
     * Getters
     */

    const PDDLFactories& get_pddl_factories() const;

    Problem get_problem() const;

    State get_state() const;

    template<ConstructorTag D>
    DenotationImpl<D>& get_denotation_builder();

    template<ConstructorTag D>
    DenotationRepository<D>& get_denotation_repository();
};
}

#endif
