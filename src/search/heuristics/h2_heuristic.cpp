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
#include "../../../include/mimir/search/heuristics/h2_heuristic.hpp"

#include <algorithm>
#include <cmath>
#include <deque>
#include <iterator>
#include <limits>

namespace mimir::planners
{
    H2Heuristic::H2Heuristic(const mimir::formalism::ProblemDescription& problem, const mimir::planners::SuccessorGenerator& successor_generator) :
        problem_(problem),
        actions_(),
        goal_(),
        h1_table_(),
        h2_table_()
    {
        const auto grounded_successor_generator = std::dynamic_pointer_cast<mimir::planners::GroundedSuccessorGenerator>(successor_generator);

        if (!grounded_successor_generator)
        {
            throw std::invalid_argument("successor generator must be grounded");
        }

        const auto num_ranks = static_cast<int32_t>(problem->num_ranks());

        // Pre-allocate memory for ht1 and ht2

        h1_table_.resize(num_ranks);
        h2_table_.resize(num_ranks);

        for (int32_t i = 0; i < num_ranks; ++i)
        {
            h2_table_[i].resize(num_ranks);
        }

        // Convert the goal to the internal format

        for (const auto& literal : problem->goal)
        {
            if (literal->negated)
            {
                throw std::invalid_argument("negative literals in the goal are not supported");
            }

            goal_.push_back(problem->get_rank(literal->atom));
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

    double H2Heuristic::eval(const std::vector<int32_t>& ranks) const
    {
        double v = 0;

        for (std::size_t i = 0; i < ranks.size(); i++)
        {
            v = std::max(v, h1_table_[ranks[i]]);

            if (is_dead_end(v))
            {
                return DEAD_END;
            }

            for (std::size_t j = i + 1; j < ranks.size(); j++)
            {
                v = std::max(v, h2_table_[ranks[i]][ranks[j]]);

                if (is_dead_end(v))
                {
                    return DEAD_END;
                }
            }
        }

        return v;
    }

    double H2Heuristic::eval(const std::vector<int32_t>& ranks, int32_t rank) const
    {
        double v = 0;

        v = std::max(v, h1_table_[rank]);

        if (is_dead_end(v))
        {
            return DEAD_END;
        }

        for (std::size_t i = 0; i < ranks.size(); i++)
        {
            if (rank == ranks[i])
            {
                continue;
            }

            v = std::max(v, h2_table_[rank][ranks[i]]);

            if (is_dead_end(v))
            {
                return DEAD_END;
            }
        }

        return v;
    }

    void H2Heuristic::update(int32_t rank, double value, bool& changed) const
    {
        if (h1_table_[rank] > value)
        {
            h1_table_[rank] = value;
            changed = true;
        }
    }

    void H2Heuristic::update(int32_t rank1, int32_t rank2, double value, bool& changed) const
    {
        if (h2_table_[rank1][rank2] > value)
        {
            h2_table_[rank1][rank2] = value;
            h2_table_[rank2][rank1] = value;
            changed = true;
        }
    }

    void H2Heuristic::fill_tables(const mimir::formalism::State& state) const
    {
        const auto num_atoms = h1_table_.size();

        for (std::size_t i = 0; i < num_atoms; ++i)
        {
            h1_table_[i] = DEAD_END;

            for (std::size_t j = 0; j < num_atoms; ++j)
            {
                h2_table_[i][j] = DEAD_END;
            }
        }

        const auto& state_atoms = state->get_atoms();
        std::vector<int32_t> state_ids;

        for (const auto& atom : state_atoms)
        {
            state_ids.emplace_back(problem_->get_rank(atom));
        }

        for (std::size_t i = 0; i < state_ids.size(); ++i)
        {
            h1_table_[state_ids[i]] = 0;

            for (std::size_t j = 0; j < state_ids.size(); ++j)
            {
                h2_table_[state_ids[i]][state_ids[j]] = 0;
            }
        }

        bool changed;

        do
        {
            changed = false;

            for (std::size_t action_index = 0; action_index < actions_.size(); ++action_index)
            {
                const auto& [precondition, add_effect, delete_effect_complement, cost] = actions_[action_index];
                const auto cost1 = eval(precondition);

                if (is_dead_end(cost1))
                {
                    continue;
                }

                for (std::size_t i = 0; i < add_effect.size(); i++)
                {
                    const auto rank1 = add_effect[i];
                    update(rank1, cost1 + cost, changed);

                    for (std::size_t j = i + 1; j < add_effect.size(); j++)
                    {
                        const auto rank2 = add_effect[j];

                        if (rank1 != rank2)
                        {
                            update(rank1, rank2, cost1 + cost, changed);
                        }
                    }

                    for (const auto rank2 : delete_effect_complement)
                    {
                        const auto cost2 = std::max(cost1, eval(precondition, rank2));

                        if (!is_dead_end(cost2))
                        {
                            update(rank1, rank2, cost2 + cost, changed);
                        }
                    }
                }
            }
        } while (changed);
    }

    double H2Heuristic::evaluate(const mimir::formalism::State& state) const
    {
        if (state->get_problem() != problem_)
        {
            throw std::invalid_argument("heuristic is constructed for a different problem");
        }

        fill_tables(state);
        return eval(goal_);
    }

    std::shared_ptr<H2Heuristic> create_h2_heuristic(const mimir::formalism::ProblemDescription& problem,
                                                     const mimir::planners::SuccessorGenerator& successor_generator)
    {
        return std::make_shared<H2Heuristic>(problem, successor_generator);
    }
}  // namespace planners
