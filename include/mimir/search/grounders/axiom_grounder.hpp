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

#ifndef MIMIR_SEARCH_GROUNDERS_AXIOM_GROUNDER_HPP_
#define MIMIR_SEARCH_GROUNDERS_AXIOM_GROUNDER_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/axiom.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/grounders/grounding_table.hpp"
#include "mimir/search/grounders/literal_grounder.hpp"
#include "mimir/search/satisficing_binding_generator.hpp"

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
    std::shared_ptr<LiteralGrounder> m_literal_grounder;

    GroundAxiomImplSet m_axioms;
    GroundAxiomList m_axioms_by_index;
    GroundAxiomImpl m_axiom_builder;
    std::unordered_map<Axiom, GroundingTable<GroundAxiom>> m_axiom_groundings;

public:
    AxiomGrounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositori, std::shared_ptr<LiteralGrounder> literal_grounderes);

    // Uncopyable
    AxiomGrounder(const AxiomGrounder& other) = delete;
    AxiomGrounder& operator=(const AxiomGrounder& other) = delete;
    // Moveable
    AxiomGrounder(AxiomGrounder&& other) = default;
    AxiomGrounder& operator=(AxiomGrounder&& other) = default;

    /// @brief Ground an axiom and return a view onto it.
    GroundAxiom ground_axiom(Axiom axiom, ObjectList binding);

    /// @brief Return all axioms.
    const GroundAxiomList& get_ground_axioms() const;

    GroundAxiom get_ground_axiom(Index axiom_index) const;

    size_t get_num_ground_axioms() const;

    Problem get_problem() const;
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const;
    const std::shared_ptr<LiteralGrounder>& get_literal_grounder() const;
};

}  // namespace mimir

#endif