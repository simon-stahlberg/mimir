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

#include "mimir/formalism/problem.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/state_repository.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

SearchContext::SearchContext(const fs::path& domain_filepath, const fs::path& problem_filepath, const Options& options) :
    m_problem(ProblemImpl::create(domain_filepath, problem_filepath))
{
    auto search_context = SearchContext(m_problem, options);
    m_applicable_action_generator = std::move(search_context.m_applicable_action_generator);
    m_state_repository = std::move(search_context.m_state_repository);
}

SearchContext::SearchContext(Problem problem, const Options& options) : m_problem(problem)
{
    switch (options.mode)
    {
        case SearchMode::GROUNDED:
        {
            auto delete_relaxed_explorator = DeleteRelaxedProblemExplorator(m_problem);
            m_state_repository = std::make_shared<StateRepositoryImpl>(delete_relaxed_explorator.create_grounded_axiom_evaluator());
            m_applicable_action_generator =
                std::dynamic_pointer_cast<IApplicableActionGenerator>(delete_relaxed_explorator.create_grounded_applicable_action_generator());
            break;
        }
        case SearchMode::LIFTED:
        {
            m_state_repository = std::make_shared<StateRepositoryImpl>(std::make_shared<LiftedAxiomEvaluator>(m_problem));
            m_applicable_action_generator = std::dynamic_pointer_cast<IApplicableActionGenerator>(std::make_shared<LiftedApplicableActionGenerator>(m_problem));
            break;
        }
        default:
        {
            throw std::runtime_error("SearchContext::SearchContext: Unexpected search mode.");
        }
    }
}

SearchContext::SearchContext(Problem problem, ApplicableActionGenerator applicable_action_generator, StateRepository state_repository) :
    m_problem(std::move(problem)),
    m_applicable_action_generator(std::move(applicable_action_generator)),
    m_state_repository(std::move(state_repository))
{
    if (m_applicable_action_generator->get_problem() != m_problem)
    {
        throw std::runtime_error("SearchContext::SearchContext: Expected the given applicable action generator to be defined over the given problem.");
    }
    if (m_state_repository->get_problem() != m_problem)
    {
        throw std::runtime_error("SearchContext::SearchContext: Expected the given state repository to be defined over the given problem.");
    }
}

const Problem& SearchContext::get_problem() const { return m_problem; }

const ApplicableActionGenerator SearchContext::get_applicable_action_generator() const { return m_applicable_action_generator; }

const StateRepository SearchContext::get_state_repository() const { return m_state_repository; }

}
