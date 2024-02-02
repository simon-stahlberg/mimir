/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "../../../include/mimir/generators/grounded_successor_generator.hpp"
#include "../../../include/mimir/search/heuristics/h1_heuristic.hpp"

#include <algorithm>
#include <cmath>
#include <deque>
#include <iterator>
#include <limits>

namespace mimir::planners
{
    H1Heuristic::H1Heuristic(const mimir::formalism::ProblemDescription& problem, const mimir::planners::SuccessorGenerator& successor_generator) :
        problem_(problem),
        actions_(),
        goal_(),
        table_()
    {
        const auto grounded_successor_generator = std::dynamic_pointer_cast<mimir::planners::GroundedSuccessorGenerator>(successor_generator);

        if (!grounded_successor_generator)
        {
            throw std::invalid_argument("successor generator must be grounded");
        }

        const auto num_ranks = static_cast<int32_t>(problem->num_ranks());

        // Pre-allocate memory for the table

        table_.resize(num_ranks);

        // Convert the goal to the internal format

        for (const auto& literal : problem->goal)
        {
            if (literal->negated)
            {
                throw std::invalid_argument("negative literals in the goal are not supported");
            }

            goal_.emplace_back(problem->get_rank(literal->atom));
        }

        // Convert all actions to the internal format

        const auto& actions = grounded_successor_generator->get_actions();

        for (const auto& action : actions)
        {
            std::vector<int32_t> precondition;
            std::vector<int32_t> add_effect;
            std::vector<int32_t> delete_effect;
            std::vector<int32_t> delete_effect_complement;

            for (const auto& literal : action->get_precondition())
            {
                if (!literal->negated)
                {
                    precondition.emplace_back(problem->get_rank(literal->atom));
                }
            }

            for (const auto& literal : action->get_unconditional_effect())
            {
                if (literal->negated)
                {
                    delete_effect.emplace_back(problem->get_rank(literal->atom));
                }
                else
                {
                    add_effect.emplace_back(problem->get_rank(literal->atom));
                }
            }

            for (const auto& [antecedent, consequence] : action->get_conditional_effect())
            {
                for (const auto& literal : consequence)
                {
                    if (literal->negated)
                    {
                        delete_effect.emplace_back(problem->get_rank(literal->atom));
                    }
                    else
                    {
                        add_effect.emplace_back(problem->get_rank(literal->atom));
                    }
                }
            }

            for (int32_t rank = 0; rank < num_ranks; ++rank)
            {
                if (std::find(delete_effect.begin(), delete_effect.end(), rank) == delete_effect.end())
                {
                    delete_effect_complement.emplace_back(rank);
                }
            }

            actions_.emplace_back(precondition, add_effect, delete_effect_complement, action->cost);
        }
    }

    double H1Heuristic::eval(const std::vector<int32_t>& ranks) const
    {
        double value = 0;

        for (const auto rank : ranks)
        {
            value = std::max(value, table_[rank]);

            if (is_dead_end(value))
            {
                return DEAD_END;
            }
        }

        return value;
    }

    void H1Heuristic::update(int32_t rank, double value, bool& changed) const
    {
        if (table_[rank] > value)
        {
            table_[rank] = value;
            changed = true;
        }
    }

    void H1Heuristic::fill_tables(const mimir::formalism::State& state) const
    {
        for (auto& value : table_)
        {
            value = DEAD_END;
        }

        const auto& state_atoms = state->get_atoms();
        std::vector<int32_t> state_ranks;

        for (const auto& atom : state_atoms)
        {
            state_ranks.emplace_back(problem_->get_rank(atom));
        }

        for (const auto rank : state_ranks)
        {
            table_[rank] = 0;
        }

        bool changed;

        do
        {
            changed = false;

            for (const auto& [precondition, add_effect, delete_effect_complement, cost] : actions_)
            {
                const auto c1 = eval(precondition);

                if (is_dead_end(c1))
                {
                    continue;
                }

                for (const auto rank : add_effect)
                {
                    update(rank, c1 + cost, changed);
                }
            }
        } while (changed);
    }

    double H1Heuristic::evaluate(const mimir::formalism::State& state) const
    {
        if (state->get_problem() != problem_)
        {
            throw std::invalid_argument("heuristic is constructed for a different problem");
        }

        fill_tables(state);
        return eval(goal_);
    }

    std::shared_ptr<H1Heuristic> create_h1_heuristic(const mimir::formalism::ProblemDescription& problem,
                                                     const mimir::planners::SuccessorGenerator& successor_generator)
    {
        return std::make_shared<H1Heuristic>(problem, successor_generator);
    }
}  // namespace planners
