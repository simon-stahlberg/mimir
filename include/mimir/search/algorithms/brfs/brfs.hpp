#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_BRFS_HPP_

#include "../interface.hpp"
#include "mimir/common/translations.hpp"
#include "mimir/formalism/common/types.hpp"
#include "mimir/formalism/problem/declarations.hpp"
#include "mimir/search/config.hpp"
#include "mimir/search/states/bitset/interface.hpp"

#include <deque>
#include <flatmemory/flatmemory.hpp>
#include <vector>

namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
 */
template<IsPlanningModeTag P, IsStateTag S = BitsetStateTag>
struct BrFSTag : public AlgorithmTag
{
};

/**
 * Specialized implementation class.
 */

template<IsPlanningModeTag P, IsStateTag S>
class Algorithm<AlgorithmDispatcher<BrFSTag<P, S>>> : public IAlgorithm<Algorithm<AlgorithmDispatcher<BrFSTag<P, S>>>>
{
private:
    using ConstStateView = ConstView<StateDispatcher<S, P>>;
    using ConstActionView = ConstView<ActionDispatcher<P, S>>;
    using ConstActionViewList = std::vector<ConstActionView>;

    Problem m_problem;
    PDDLFactories& m_pddl_factories;
    SSG<SSGDispatcher<P, S>> m_state_repository;
    ConstStateView m_initial_state;
    AAG<AAGDispatcher<P, S>> m_successor_generator;

    // Implement configuration independent functionality.
    std::deque<ConstStateView> m_queue;

    CostSearchNodeVector m_search_nodes;

    /* Implement IAlgorithm interface. */
    template<typename>
    friend class IAlgorithm;

    SearchStatus find_solution_impl(ConstActionViewList& out_plan)
    {
        // TODO: Implement tracking of visited states and the predecessor state.

        // auto initial_search_node = CostSearchNodeViewWrapper(this->m_search_nodes[this->m_initial_state.get_id()]);
        // initial_search_node.get_g_value() = 0;

        LiteralList goal_literals;
        to_literals(m_problem->get_goal_condition(), goal_literals);

        GroundLiteralList goal_ground_literals;
        m_pddl_factories.to_ground_literals(goal_literals, goal_ground_literals);

        auto applicable_actions = ConstActionViewList();

        m_queue.emplace_back(this->m_initial_state);
        while (!m_queue.empty())
        {
            const auto state = m_queue.front();
            m_queue.pop_front();

            if (state.literals_hold(goal_ground_literals))
            {
                return SearchStatus::SOLVED;
            }

            // const auto search_node = this->m_search_nodes[state.get_id()];

            this->m_successor_generator.generate_applicable_actions(state, applicable_actions);
            for (const auto& action : applicable_actions)
            {
                const auto& successor_state = this->m_state_repository.get_or_create_successor_state(state, action);
                m_queue.emplace_back(successor_state);
            }
        }

        return SearchStatus::FAILED;
    }

    static auto create_default_search_node_builder()
    {
        auto builder = CostSearchNodeBuilderWrapper(CostSearchNodeBuilder());
        builder.set_status(SearchNodeStatus::CLOSED);
        builder.set_g_value(-1);
        builder.set_parent_state_id(-1);
        builder.finish();
        return builder.get_flatmemory_builder();
    }

public:
    Algorithm(const Problem& problem, PDDLFactories& pddl_factories) :
        m_problem(problem),
        m_pddl_factories(pddl_factories),
        m_state_repository(SSG<SSGDispatcher<P, S>>(problem)),
        m_initial_state(m_state_repository.get_or_create_initial_state(problem)),
        m_successor_generator(AAG<AAGDispatcher<P, S>>(problem, pddl_factories)),
        m_search_nodes(flatmemory::FixedSizedTypeVector(create_default_search_node_builder()))
    {
    }
};

/**
 * Type traits.
 */
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<Algorithm<AlgorithmDispatcher<BrFSTag<P, S>>>>
{
    using PlanningModeTag = P;
    using StateTag = S;

    using ConstActionView = ConstView<ActionDispatcher<P, S>>;
    using ConstActionViewList = std::vector<ConstActionView>;
};

}

#endif
