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

#include "../../generators/grounded_successor_generator.hpp"
#include "h2_heuristic.hpp"

#include <algorithm>
#include <cmath>
#include <deque>
#include <iterator>
#include <limits>

namespace planners
{
    H2Heuristic::H2Heuristic(const formalism::ProblemDescription& problem, const planners::SuccessorGenerator& successor_generator) :
        problem_(problem),
        actions_(),
        goal_(),
        ht1_(),
        ht2_()
    {
        const auto grounded_successor_generator = std::dynamic_pointer_cast<planners::GroundedSuccessorGenerator>(successor_generator);

        if (!grounded_successor_generator)
        {
            throw std::invalid_argument("successor generator must be grounded");
        }

        const auto num_ranks = static_cast<int32_t>(problem->num_ranks());

        // Pre-allocate memory for ht1 and ht2

        ht1_.resize(num_ranks);
        ht2_.resize(num_ranks);

        for (std::size_t i = 0; i < num_ranks; ++i)
        {
            ht2_[i].resize(num_ranks);
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

    double H2Heuristic::evaluate(const std::vector<int32_t>& s) const
    {
        double v = 0;

        for (std::size_t i = 0; i < s.size(); i++)
        {
            v = std::max(v, ht1_[s[i]]);

            if (is_dead_end(v))
            {
                return DEAD_END;
            }

            for (std::size_t j = i + 1; j < s.size(); j++)
            {
                v = std::max(v, ht2_[s[i]][s[j]]);

                if (is_dead_end(v))
                {
                    return DEAD_END;
                }
            }
        }

        return v;
    }

    double H2Heuristic::evaluate(const std::vector<int32_t>& s, int32_t x) const
    {
        double v = 0;

        v = std::max(v, ht1_[x]);

        if (is_dead_end(v))
        {
            return DEAD_END;
        }

        for (std::size_t i = 0; i < s.size(); i++)
        {
            if (x == s[i])
            {
                continue;
            }

            v = std::max(v, ht2_[x][s[i]]);

            if (is_dead_end(v))
            {
                return DEAD_END;
            }
        }

        return v;
    }

    void H2Heuristic::update(const std::size_t val, const int32_t h, bool& changed) const
    {
        if (ht1_[val] > h)
        {
            ht1_[val] = h;
            changed = true;
        }
    }

    void H2Heuristic::update(const std::size_t val1, const std::size_t val2, const int32_t h, bool& changed) const
    {
        if (ht2_[val1][val2] > h)
        {
            ht2_[val1][val2] = h;
            ht2_[val2][val1] = h;
            changed = true;
        }
    }

    void H2Heuristic::fill_tables(const formalism::State& state) const
    {
        const auto num_atoms = ht1_.size();

        for (std::size_t i = 0; i < num_atoms; ++i)
        {
            ht1_[i] = DEAD_END;

            for (std::size_t j = 0; j < num_atoms; ++j)
            {
                ht2_[i][j] = DEAD_END;
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
            ht1_[state_ids[i]] = 0;

            for (std::size_t j = 0; j < state_ids.size(); ++j)
            {
                ht2_[state_ids[i]][state_ids[j]] = 0;
            }
        }

        bool changed;

        do
        {
            changed = false;

            for (std::size_t action_index = 0; action_index < actions_.size(); ++action_index)
            {
                const auto& [precondition, add_effect, delete_effect_complement, cost] = actions_[action_index];
                const auto c1 = evaluate(precondition);

                if (is_dead_end(c1))
                {
                    continue;
                }

                for (std::size_t i = 0; i < add_effect.size(); i++)
                {
                    const auto p = add_effect[i];
                    update(p, c1 + cost, changed);

                    for (std::size_t j = i + 1; j < add_effect.size(); j++)
                    {
                        const auto q = add_effect[j];

                        if (p != q)
                        {
                            update(p, q, c1 + cost, changed);
                        }
                    }

                    for (const auto r : delete_effect_complement)
                    {
                        const auto c2 = std::max(c1, evaluate(precondition, r));

                        if (!is_dead_end(c2))
                        {
                            update(p, r, c2 + cost, changed);
                        }
                    }
                }
            }
        } while (changed);
    }

    double H2Heuristic::evaluate(const formalism::State& state) const
    {
        if (state->get_problem() != problem_)
        {
            throw std::invalid_argument("heuristic is constructed for a different problem");
        }

        fill_tables(state);
        return evaluate(goal_);
    }

    std::shared_ptr<H2Heuristic> create_h2_heuristic(const formalism::ProblemDescription& problem, const planners::SuccessorGenerator& successor_generator)
    {
        return std::make_shared<H2Heuristic>(problem, successor_generator);
    }
}  // namespace planners
