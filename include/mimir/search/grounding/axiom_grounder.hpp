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

#ifndef MIMIR_SEARCH_GROUNDING_AXIOM_GROUNDER_HPP_
#define MIMIR_SEARCH_GROUNDING_AXIOM_GROUNDER_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/grounding_table.hpp"
#include "mimir/search/axiom.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/grounding/condition_grounder.hpp"

#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace mimir
{

class AxiomGrounder
{
private:
    Problem m_problem;
    std::shared_ptr<PDDLRepositories> m_pddl_repositories;

    std::unordered_map<Axiom, ConditionGrounder> m_condition_grounders;

    GroundAxiomImplSet m_axioms;
    GroundAxiomList m_axioms_by_index;
    GroundAxiomImpl m_axiom_builder;
    std::unordered_map<Axiom, GroundingTable<GroundAxiom>> m_axiom_groundings;

public:
    AxiomGrounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories);

    // Uncopyable
    AxiomGrounder(const AxiomGrounder& other) = delete;
    AxiomGrounder& operator=(const AxiomGrounder& other) = delete;
    // Moveable
    AxiomGrounder(AxiomGrounder&& other) = default;
    AxiomGrounder& operator=(AxiomGrounder&& other) = default;

    /// @brief Ground an axiom and return a view onto it.
    GroundAxiom ground_axiom(Axiom axiom, ObjectList binding);

    std::unordered_map<Axiom, ConditionGrounder>& get_axiom_precondition_grounders();

    /// @brief Return all axioms.
    const GroundAxiomList& get_ground_axioms() const;

    GroundAxiom get_ground_axiom(Index axiom_index) const;

    size_t get_num_ground_axioms() const;

    Problem get_problem() const;
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const;
};

}  // namespace mimir

#endif
