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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_AXIOM_EVALUATOR_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_AXIOM_EVALUATOR_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/grounding_table.hpp"
#include "mimir/formalism/predicate_tag.hpp"
#include "mimir/search/applicable_action_generators/lifted/assignment_set.hpp"
#include "mimir/search/applicable_action_generators/lifted/axiom_stratification.hpp"
#include "mimir/search/applicable_action_generators/lifted/consistency_graph.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers.hpp"
#include "mimir/search/axiom.hpp"
#include "mimir/search/condition_grounders.hpp"
#include "mimir/search/declarations.hpp"

#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace mimir
{

class AxiomEvaluator
{
private:
    Problem m_problem;
    std::shared_ptr<PDDLRepositories> m_pddl_repositories;
    std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> m_event_handler;

    std::vector<AxiomPartition> m_partitioning;

    GroundAxiomImplSet m_flat_axioms;
    GroundAxiomList m_axioms_by_index;
    GroundAxiomImpl m_axiom_builder;
    std::unordered_map<Axiom, GroundingTable<GroundAxiom>> m_axiom_groundings;

    std::unordered_map<Axiom, ConditionGrounder> m_condition_grounders;

public:
    /// @brief Simplest construction, expects the event handler from the lifted applicable_action_generator.
    AxiomEvaluator(Problem problem,
                   std::shared_ptr<PDDLRepositories> pddl_repositories,
                   std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> event_handler);

    // Uncopyable
    AxiomEvaluator(const AxiomEvaluator& other) = delete;
    AxiomEvaluator& operator=(const AxiomEvaluator& other) = delete;
    // Unmovable
    AxiomEvaluator(AxiomEvaluator&& other) = delete;
    AxiomEvaluator& operator=(AxiomEvaluator&& other) = delete;

    /// @brief Generate and apply all applicable axioms.
    void generate_and_apply_axioms(StateImpl& unextended_state);

    /// @brief Return the axiom partitioning.
    const std::vector<AxiomPartition>& get_axiom_partitioning() const;

    /// @brief Ground an axiom and return a view onto it.
    GroundAxiom ground_axiom(Axiom axiom, ObjectList&& binding);

    /// @brief Return all axioms.
    const GroundAxiomList& get_ground_axioms() const;

    GroundAxiom get_ground_axiom(Index axiom_index) const;

    size_t get_num_ground_axioms() const;
};

}  // namespace mimir

#endif
