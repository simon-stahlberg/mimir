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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/grounding/action_grounder.hpp"

#include <unordered_map>
#include <vector>

namespace mimir
{

/// @brief `LiftedApplicableActionGenerator` implements lifted applicable action generation
/// using maximum clique enumeration by Stahlberg (ECAI2023).
/// Source: https://mrlab.ai/papers/stahlberg-ecai2023.pdf
class LiftedApplicableActionGenerator : public IApplicableActionGenerator
{
private:
    ActionGrounder m_grounder;

    std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> m_event_handler;

public:
    /// @brief Simplest construction
    LiftedApplicableActionGenerator(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories);

    /// @brief Complete construction
    LiftedApplicableActionGenerator(Problem problem,
                                    std::shared_ptr<PDDLRepositories> pddl_repositories,
                                    std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> event_handler);

    // Uncopyable
    LiftedApplicableActionGenerator(const LiftedApplicableActionGenerator& other) = delete;
    LiftedApplicableActionGenerator& operator=(const LiftedApplicableActionGenerator& other) = delete;
    // Unmovable
    LiftedApplicableActionGenerator(LiftedApplicableActionGenerator&& other) = delete;
    LiftedApplicableActionGenerator& operator=(LiftedApplicableActionGenerator&& other) = delete;

    std::generator<GroundAction> create_applicable_action_generator(State state) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    ActionGrounder& get_action_grounder() override;
    const ActionGrounder& get_action_grounder() const override;
};

}  // namespace mimir

#endif
