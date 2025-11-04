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

#ifndef MIMIR_SEARCH_GROUNDERS_INTERFACE_HPP_
#define MIMIR_SEARCH_GROUNDERS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/options.hpp"

#include <memory>

namespace mimir::search
{

class IGrounder
{
protected:
    formalism::Problem m_problem;

public:
    explicit IGrounder(formalism::Problem problem);
    IGrounder(const IGrounder& other) = delete;
    IGrounder& operator=(const IGrounder& other) = delete;
    IGrounder(IGrounder&& other) = delete;
    IGrounder& operator=(IGrounder&& other) = delete;
    virtual ~IGrounder() = default;

    /// @brief Create all delete-relaxed-reachable unrelaxed ground actions.
    /// @return a vector containing all delete-relaxed-reachable unrelaxed ground actions.
    virtual formalism::GroundActionList create_ground_actions() const = 0;

    /// @brief Create all delete-relaxed-reachable unrelaxed ground axioms.
    /// @return a vector containing all delete-relaxed-reachable unrelaxed ground axioms.
    virtual formalism::GroundAxiomList create_ground_axioms() const = 0;

    /// @brief Create a grounded axiom evaluator.
    /// @param options the match tree options
    /// @param event_handler the grounded axiom evaluator event handler.
    /// @return a grounded axiom evaluator.
    GroundedAxiomEvaluator create_grounded_axiom_evaluator(const match_tree::Options& options = match_tree::Options(),
                                                           axiom_evaluator::grounded::EventHandler event_handler = nullptr) const;

    /// @brief Create a grounded applicable action generator.
    /// @param options the match tree options
    /// @param event_handler the grounded applicable action generator event handler.
    /// @return a grounded applicable action generator.
    GroundedApplicableActionGenerator
    create_grounded_applicable_action_generator(const match_tree::Options& options = match_tree::Options(),
                                                applicable_action_generator::grounded::EventHandler event_handler = nullptr) const;

    /// @brief Get the input problem.
    /// @return the input problem.
    const formalism::Problem& get_problem() const;
};

}  // namespace mimir

#endif
