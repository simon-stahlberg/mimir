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

#ifndef MIMIR_SEARCH_HEURISTICS_H2_HPP_
#define MIMIR_SEARCH_HEURISTICS_H2_HPP_

#include "mimir/search/grounders/interface.hpp"
#include "mimir/search/heuristics/interface.hpp"

#include <memory>
#include <vector>

namespace mimir::search
{

class H2HeuristicImpl : public IHeuristic
{
public:
    explicit H2HeuristicImpl(const IGrounder& grounder);

    ContinuousCost compute_heuristic(const State& state, formalism::GroundConjunctiveCondition goal = nullptr) override;

    static std::shared_ptr<H2HeuristicImpl> create(const IGrounder& grounder);

private:
    struct InternalGroundAction
    {
        std::vector<uint32_t> precondition;
        std::vector<uint32_t> add_effect;
        std::vector<uint32_t> delete_effect;
        std::vector<uint32_t> delete_effect_complement;
        double cost;
    };

    formalism::Problem m_problem;
    std::vector<InternalGroundAction> m_internal_actions;
    mutable std::vector<uint32_t> m_goal;
    mutable formalism::GroundConjunctiveCondition m_last_goal = nullptr;

    mutable std::vector<double> m_h1_table;
    mutable std::vector<double> m_h2_table;  // Flattened N x N

    uint32_t m_num_fluent_atoms;
    uint32_t m_num_derived_atoms;
    uint32_t m_num_atoms;

    double evaluate(const std::vector<uint32_t>& indices) const;
    double evaluate(const std::vector<uint32_t>& indices, uint32_t index) const;

    void update(uint32_t index, double value, bool& changed) const;
    void update(uint32_t first_index, uint32_t second_index, double value, bool& changed) const;

    void update_goal(formalism::GroundConjunctiveCondition goal) const;
    void initialize_tables(const State& state) const;
    void fill_tables(const State& state, formalism::GroundConjunctiveCondition goal) const;
};

using H2Heuristic = std::shared_ptr<H2HeuristicImpl>;

}

#endif  // MIMIR_SEARCH_HEURISTICS_H2_HPP_
