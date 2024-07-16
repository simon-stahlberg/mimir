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

class EvaluationContext
{
private:
    /* Memory */
    std::reference_wrapper<DenotationRepository<Concept>> m_concept_denotation_repository;
    std::reference_wrapper<DenotationRepository<Role>> m_role_denotation_repository;

    /* State Information */
    size_t m_state_id;
    std::reference_wrapper<const GroundAtomList<Static>> m_static_state_atoms;
    std::reference_wrapper<const GroundAtomList<Fluent>> m_fluent_state_atoms;
    std::reference_wrapper<const GroundAtomList<Derived>> m_derived_state_atoms;

    /* Goal information */
    std::reference_wrapper<const GroundAtomList<Static>> m_static_goal_atoms;
    std::reference_wrapper<const GroundAtomList<Fluent>> m_fluent_goal_atoms;
    std::reference_wrapper<const GroundAtomList<Derived>> m_derived_goal_atoms;

    /* Evaluation memory. */
    DenotationBuilder<Concept> m_concept_denotation_builder;
    DenotationBuilder<Role> m_role_denotation_builder;

public:
    EvaluationContext(DenotationRepository<Concept>& concept_denotation_repository,
                      DenotationRepository<Role>& role_denotation_repository,
                      size_t state_id,
                      const GroundAtomList<Static>& static_state_atoms,
                      const GroundAtomList<Fluent>& fluent_state_atoms,
                      const GroundAtomList<Derived>& derived_state_atoms,
                      const GroundAtomList<Static>& static_goal_atoms,
                      const GroundAtomList<Fluent>& fluent_goal_atoms,
                      const GroundAtomList<Derived>& derived_goal_atoms,
                      size_t num_objects);

    /**
     * Setters
     */

    /// @brief Set the state id used to identify the result for caching.
    void set_state_id(size_t state_id);

    /// @brief Set the state atoms.
    template<DynamicPredicateCategory P>
    void set_state_atoms(const GroundAtomList<P>& state_atoms);

    /**
     * Getters
     */

    template<IsConceptOrRole D>
    DenotationBuilder<D>& get_denotation_builder();

    template<IsConceptOrRole D>
    DenotationRepository<D>& get_denotation_repository();

    size_t get_state_id() const;

    template<PredicateCategory P>
    const GroundAtomList<P>& get_state_atoms() const;

    template<PredicateCategory P>
    const GroundAtomList<P>& get_goal_atoms() const;
};
}

#endif
