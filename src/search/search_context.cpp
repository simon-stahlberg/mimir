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

#include "mimir/common/concepts.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/state_repository.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

SearchContextImpl::SearchContextImpl(formalism::Problem problem, ApplicableActionGenerator applicable_action_generator, StateRepository state_repository) :
    m_problem(std::move(problem)),
    m_applicable_action_generator(std::move(applicable_action_generator)),
    m_state_repository(std::move(state_repository))
{
}

SearchContext SearchContextImpl::create(const fs::path& domain_filepath, const fs::path& problem_filepath, const Options& options)
{
    return create(ProblemImpl::create(domain_filepath, problem_filepath), options);
}

SearchContext SearchContextImpl::create(Problem problem, const Options& options)
{
    return std::visit(
        [&](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, GroundedOptions>)
            {
                auto delete_relaxed_explorator = DeleteRelaxedProblemExplorator(problem);

                return create(problem,
                              delete_relaxed_explorator.create_grounded_applicable_action_generator(),
                              std::make_shared<StateRepositoryImpl>(delete_relaxed_explorator.create_grounded_axiom_evaluator()));
            }
            else if constexpr (std::is_same_v<T, LiftedOptions>)
            {  // Lifted
                switch (arg.kind)
                {
                    case LiftedOptions::Kind::EXHAUSTIVE:
                    {
                        return create(problem,
                                      std::make_shared<ExhaustiveLiftedApplicableActionGeneratorImpl>(problem),
                                      std::make_shared<StateRepositoryImpl>(std::make_shared<ExhaustiveLiftedAxiomEvaluatorImpl>(problem)));
                    }
                    case LiftedOptions::Kind::KPKC:
                    {
                        return create(problem,
                                      std::make_shared<KPKCLiftedApplicableActionGeneratorImpl>(problem),
                                      std::make_shared<StateRepositoryImpl>(std::make_shared<KPKCLiftedAxiomEvaluatorImpl>(problem)));
                    }
                    default:
                        throw std::logic_error("Unknown GeneratorOptions.");
                }
            }
            else
            {
                static_assert(dependent_false<T>::value, "Unhandled generator kind");
            }
        },
        options.mode);
}

SearchContext SearchContextImpl::create(Problem problem, ApplicableActionGenerator applicable_action_generator, StateRepository state_repository)
{
    if (applicable_action_generator->get_problem() != problem)
    {
        throw std::runtime_error("SearchContext::SearchContext: Expected the given applicable action generator to be defined over the given problem.");
    }
    if (state_repository->get_problem() != problem)
    {
        throw std::runtime_error("SearchContext::SearchContext: Expected the given state repository to be defined over the given problem.");
    }

    return std::shared_ptr<SearchContextImpl>(new SearchContextImpl(std::move(problem), std::move(applicable_action_generator), std::move(state_repository)));
}

const Problem& SearchContextImpl::get_problem() const { return m_problem; }

const ApplicableActionGenerator SearchContextImpl::get_applicable_action_generator() const { return m_applicable_action_generator; }

const StateRepository SearchContextImpl::get_state_repository() const { return m_state_repository; }

}
