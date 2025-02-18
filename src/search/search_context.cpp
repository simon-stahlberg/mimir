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

#include "mimir/search/search_context.hpp"

#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/state_repository.hpp"

namespace mimir
{

SearchContext::SearchContext(ProblemContext problem_context, const Options& options) : m_problem_context(problem_context)
{
    switch (options.mode)
    {
        case SearchMode::GROUNDED:
        {
            auto delete_relaxed_explorator = DeleteRelaxedProblemExplorator(m_problem_context);
            m_state_repository = std::make_shared<StateRepository>(delete_relaxed_explorator.create_grounded_axiom_evaluator());
            m_applicable_action_generator =
                std::dynamic_pointer_cast<IApplicableActionGenerator>(delete_relaxed_explorator.create_grounded_applicable_action_generator());
            break;
        }
        case SearchMode::LIFTED:
        {
            m_state_repository = std::make_shared<StateRepository>(std::make_shared<LiftedAxiomEvaluator>(m_problem_context));
            m_applicable_action_generator =
                std::dynamic_pointer_cast<IApplicableActionGenerator>(std::make_shared<LiftedApplicableActionGenerator>(m_problem_context));
            break;
        }
        default:
        {
            throw std::runtime_error("SearchContext::SearchContext: Unexpected search mode.");
        }
    }
}

SearchContext::SearchContext(ProblemContext problem_context,
                             std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                             std::shared_ptr<StateRepository> state_repository) :
    m_problem_context(std::move(problem_context)),
    m_applicable_action_generator(std::move(applicable_action_generator)),
    m_state_repository(std::move(state_repository))
{
}

std::vector<SearchContext> SearchContext::create(ProblemContextList problem_contexts, const Options& options)
{
    auto result = SearchContextList {};
    for (auto& problem_context : problem_contexts)
    {
        result.emplace_back(std::move(problem_context), options);
    }
    return result;
}

const ProblemContext& SearchContext::get_problem_context() const { return m_problem_context; }

const std::shared_ptr<IApplicableActionGenerator> SearchContext::get_applicable_action_generator() const { return m_applicable_action_generator; }

const std::shared_ptr<StateRepository> SearchContext::get_state_repository() const { return m_state_repository; }

}
