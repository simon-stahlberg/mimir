#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_BRFS_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/common/translations.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/config.hpp"
#include "mimir/search/states/bitset/interface.hpp"
#include "mimir/search/statistics.hpp"

#include <deque>
#include <flatmemory/flatmemory.hpp>
#include <functional>
#include <ostream>
#include <tuple>
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
    std::deque<ConstStateView> m_queue;
    CostSearchNodeVector m_search_nodes;
    Statistics m_statistics;

    /* Implement IAlgorithm interface. */
    template<typename>
    friend class IAlgorithm;

    SearchStatus find_solution_impl(ConstActionViewList& out_plan)
    {
        auto initial_search_node = CostSearchNodeViewProxy(this->m_search_nodes[this->m_initial_state.get_id()]);
        initial_search_node.get_g_value() = 0;
        initial_search_node.get_status() = SearchNodeStatus::OPEN;

        auto goal_literals = LiteralList {};
        to_literals(m_problem->get_goal_condition(), goal_literals);

        auto goal_ground_literals = GroundLiteralList {};
        m_pddl_factories.to_ground_literals(goal_literals, goal_ground_literals);

        auto applicable_actions = ConstActionViewList {};

        // std::cout << "Initial: " << std::make_tuple(this->m_initial_state, std::cref(m_pddl_factories)) << std::endl;

        m_queue.emplace_back(this->m_initial_state);
        while (!m_queue.empty())
        {
            const auto state = m_queue.front();
            m_queue.pop_front();

            if (state.literals_hold(goal_ground_literals))
            {
                std::cout << "Expanded: " << m_statistics.get_num_expanded() << "; Generated: " << m_statistics.get_num_generated() << std::endl;

                set_plan(CostSearchNodeConstViewProxy(this->m_search_nodes[state.get_id()]), out_plan);

                return SearchStatus::SOLVED;
            }

            m_statistics.increment_num_expanded();

            auto search_node = CostSearchNodeViewProxy(this->m_search_nodes[state.get_id()]);
            search_node.get_status() = SearchNodeStatus::CLOSED;

            // std::cout << "---" << std::endl;

            this->m_successor_generator.generate_applicable_actions(state, applicable_actions);
            for (const auto& action : applicable_actions)
            {
                m_statistics.increment_num_generated();

                const auto state_count = m_state_repository.state_count();
                const auto& successor_state = this->m_state_repository.get_or_create_successor_state(state, action);

                if (state_count != m_state_repository.state_count())
                {
                    auto successor_search_node = CostSearchNodeViewProxy(this->m_search_nodes[successor_state.get_id()]);
                    successor_search_node.get_status() = SearchNodeStatus::OPEN;
                    successor_search_node.get_g_value() = search_node.get_g_value() + 1;  // we use unit costs for now
                    successor_search_node.get_parent_state_id() = state.get_id();
                    successor_search_node.get_creating_action_id() = action.get_id();

                    m_queue.emplace_back(successor_state);

                    // std::cout << "Action: " << std::make_tuple(action, std::cref(m_pddl_factories)) << std::endl;
                    // std::cout << "Successor: " << std::make_tuple(successor_state, std::cref(m_pddl_factories)) << std::endl;
                }
            }
        }

        std::cout << "Expanded: " << m_statistics.get_num_expanded() << "; Generated: " << m_statistics.get_num_generated() << std::endl;

        return SearchStatus::FAILED;
    }

    /// @brief Compute the plan consisting of ground actions by collecting the creating actions
    ///        and reversing them.
    /// @param view The search node from which backtracking begins for which the goal was
    ///             satisfied.
    /// @param[out] out_plan The sequence of ground actions that leads from the initial state to
    ///                      the to the state underlying the search node.
    void set_plan(const CostSearchNodeConstViewProxy& view, ConstActionViewList& out_plan) const
    {
        out_plan.clear();
        auto cur_view = view;

        while (cur_view.get_parent_state_id() != -1)
        {
            out_plan.push_back(m_successor_generator.get_action(cur_view.get_creating_action_id()));

            cur_view = CostSearchNodeConstViewProxy(this->m_search_nodes[cur_view.get_parent_state_id()]);
        }

        std::reverse(out_plan.begin(), out_plan.end());
    }

    /// @brief Creates a CostSearchNodeBuilderProxy whose attributes are default initialized.
    static auto create_default_search_node_builder()
    {
        auto builder = CostSearchNodeBuilderProxy(CostSearchNodeBuilder());
        builder.set_status(SearchNodeStatus::CLOSED);
        builder.set_g_value(-1);
        builder.set_parent_state_id(-1);
        builder.set_creating_action_id(-1);
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
