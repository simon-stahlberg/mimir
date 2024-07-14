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
#include "mimir/formalism/factories.hpp"
#include "mimir/languages/description_logics/constructor_interface.hpp"
#include "mimir/languages/description_logics/denotation_repositories.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl
{

struct EvaluationContext
{
    std::reference_wrapper<const PDDLFactories> factories;
    Problem problem;
    State state;
    DenotationBuilder<Concept> concept_denotation_builder;
    DenotationRepository<Concept> concept_denotation_repository;
    DenotationBuilder<Role> role_denotation_builder;
    DenotationRepository<Role> role_denotation_repository;

    template<IsConceptOrRole D>
    DenotationBuilder<D>& get_denotation_builder()
    {
        if constexpr (std::is_same_v<D, Concept>)
        {
            return concept_denotation_builder;
        }
        else if constexpr (std::is_same_v<D, Role>)
        {
            return role_denotation_builder;
        }
        else
        {
            static_assert(dependent_false<D>::value, "Missing implementation for IsConceptOrRole.");
        }
    }

    template<IsConceptOrRole D>
    DenotationRepository<D>& get_denotation_repository()
    {
        if constexpr (std::is_same_v<D, Concept>)
        {
            return concept_denotation_repository;
        }
        else if constexpr (std::is_same_v<D, Role>)
        {
            return role_denotation_repository;
        }
        else
        {
            static_assert(dependent_false<D>::value, "Missing implementation for IsConceptOrRole.");
        }
    }
};
}

#endif
