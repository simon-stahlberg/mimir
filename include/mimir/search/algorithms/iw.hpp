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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_HPP_

#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/algorithms/event_handlers.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/algorithms/iw/index_mappers.hpp"
#include "mimir/search/algorithms/iw/pruning_strategy.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/successor_state_generators.hpp"

namespace mimir
{

class IterativeWidthAlgorithm : public IAlgorithm
{
private:
    std::shared_ptr<IApplicableActionGenerator> m_aag;
    int m_max_arity;

    std::shared_ptr<ISuccessorStateGenerator> m_ssg;
    std::shared_ptr<IAlgorithmEventHandler> m_event_handler;

    std::shared_ptr<FluentAndDerivedMapper> m_atom_index_mapper;

    State m_initial_state;
    int m_cur_arity;
    BrFsAlgorithm m_brfs;

public:
    /// @brief Simplest construction
    IterativeWidthAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator, int max_arity) :
        IterativeWidthAlgorithm(applicable_action_generator,
                                max_arity,
                                std::make_shared<SuccessorStateGenerator>(applicable_action_generator),
                                std::make_shared<DebugAlgorithmEventHandler>())
    {
    }

    /// @brief Complete construction
    IterativeWidthAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                            int max_arity,
                            std::shared_ptr<ISuccessorStateGenerator> successor_state_generator,
                            std::shared_ptr<IAlgorithmEventHandler> event_handler) :
        m_aag(applicable_action_generator),
        m_max_arity(max_arity),
        m_ssg(successor_state_generator),
        m_event_handler(event_handler),
        m_initial_state(m_ssg->get_or_create_initial_state()),
        m_cur_arity(0),
        m_brfs(applicable_action_generator, successor_state_generator, event_handler)
    {
        if (max_arity < 0)
        {
            throw std::runtime_error("Arity must be greater of equal than 0.");
        }
    }

    SearchStatus find_solution(GroundActionList& out_plan) override { return find_solution(m_initial_state, out_plan); }

    SearchStatus find_solution(const State start_state, GroundActionList& out_plan) override
    {
        while (m_cur_arity <= m_max_arity)
        {
            std::cout << "[IterativeWidth] Run IW(" << m_cur_arity << ")" << std::endl;

            auto search_status =
                (m_cur_arity > 0) ?
                    m_brfs.find_solution(start_state, std::make_unique<ArityKNoveltyPruning>(m_cur_arity, INITIAL_TABLE_ATOMS, m_atom_index_mapper), out_plan) :
                    m_brfs.find_solution(start_state, std::make_unique<ArityZeroNoveltyPruning>(start_state), out_plan);

            if (search_status == SearchStatus::SOLVED)
            {
                return SearchStatus::SOLVED;
            }

            ++m_cur_arity;
        }
        return SearchStatus::FAILED;
    }
};

using IWAlgorithm = IterativeWidthAlgorithm;
}

#endif
