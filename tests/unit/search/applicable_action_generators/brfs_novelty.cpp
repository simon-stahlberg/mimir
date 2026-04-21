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

#include "mimir/formalism/problem.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/algorithms/brfs/event_handlers/default.hpp"
#include "mimir/search/applicable_action_generators/grounded/novelty.hpp"
#include "mimir/search/axiom_evaluators/grounded/grounded.hpp"
#include "mimir/search/grounders/lifted.hpp"
#include "mimir/search/heuristics/blind.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/state_repository.hpp"

#include <gtest/gtest.h>

using namespace mimir::formalism;
using namespace mimir::search;

class BrFSEventHandler : public brfs::EventHandlerBase<BrFSEventHandler>
{
    friend class brfs::EventHandlerBase<BrFSEventHandler>;

public:
    int expanded_states = 0;
    int generated_states = 0;
    GroundedNoveltyApplicableActionGenerator generator;

    explicit BrFSEventHandler(GroundedNoveltyApplicableActionGenerator generator = nullptr) : brfs::EventHandlerBase<BrFSEventHandler>(), generator(generator)
    {
    }

protected:
    void on_expand_state_impl(const State& state) { expanded_states++; }

    void on_expand_goal_state_impl(const State& state) {}

    void on_generate_state_impl(const State& state, mimir::formalism::GroundAction action, mimir::ContinuousCost action_cost, const State& successor_state)
    {
        generated_states++;
        if (generator)
        {
            generator->update_novelty_history(action);
        }
    }

    void on_generate_state_in_search_tree_impl(const State& state,
                                               mimir::formalism::GroundAction action,
                                               mimir::ContinuousCost action_cost,
                                               const State& successor_state)
    {
    }

    void on_generate_state_not_in_search_tree_impl(const State& state,
                                                   mimir::formalism::GroundAction action,
                                                   mimir::ContinuousCost action_cost,
                                                   const State& successor_state)
    {
    }

    void on_finish_g_layer_impl(mimir::DiscreteCost g_value, uint64_t num_expanded, uint64_t num_generated) {}

    void on_start_search_impl(const State& start_state) {}

    void on_end_search_impl(uint64_t num_reached_fluent_atoms,
                            uint64_t num_reached_derived_atoms,
                            uint64_t num_states,
                            uint64_t num_nodes,
                            uint64_t num_actions,
                            uint64_t num_axioms)
    {
    }

    void on_solved_impl(const Plan& plan) {}

    void on_unsolvable_impl() {}

    void on_exhausted_impl() {}
};

TEST(MimirTests, BrFSNoveltyTest)
{
    const auto domain_path = fs::path(std::string(DATA_DIR) + "blocks_3/domain.pddl");
    const auto problem_path = fs::path(std::string(DATA_DIR) + "blocks_3/test_problem.pddl");
    const auto problem = ProblemImpl::create(domain_path, problem_path);

    // Normal run
    auto normal_handler = std::make_shared<BrFSEventHandler>();
    auto normal_options = brfs::Options();
    normal_options.event_handler = normal_handler;
    auto normal_context = SearchContextImpl::create(problem, SearchContextImpl::Options(SearchContextImpl::GroundedOptions()));

    // Normal solve
    try
    {
        const auto normal_result = brfs::find_solution(normal_context, normal_options);
        EXPECT_EQ(normal_result.status, mimir::search::SearchStatus::SOLVED);
    }
    catch (...)
    {
    }

    // Novelty run
    const auto novelty_generator = GroundedNoveltyApplicableActionGeneratorImpl::create(problem,
                                                                                        1,
                                                                                        match_tree::Options(),
                                                                                        nullptr,
                                                                                        GroundedNoveltyApplicableActionGeneratorImpl::Mode::Minimal);
    auto novelty_handler = std::make_shared<BrFSEventHandler>(novelty_generator);
    auto novelty_options = brfs::Options();
    novelty_options.event_handler = novelty_handler;

    auto novelty_grounder = LiftedGrounder(problem);
    auto novelty_axiom_evaluator = novelty_grounder.create_grounded_axiom_evaluator();
    auto novelty_state_repository = StateRepositoryImpl::create(std::static_pointer_cast<IAxiomEvaluator>(novelty_axiom_evaluator));
    auto novelty_context = SearchContextImpl::create(problem, novelty_generator, novelty_state_repository);

    try
    {
        const auto novelty_result = brfs::find_solution(novelty_context, novelty_options);
    }
    catch (const std::exception& e)
    {
    }

    EXPECT_LT(novelty_handler->expanded_states, normal_handler->expanded_states);
    EXPECT_LT(novelty_handler->generated_states, normal_handler->generated_states);
}
