#include "mimir/search/algorithms/event_handlers/debug.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/search/plan.hpp"

namespace mimir
{
void DebugAlgorithmEventHandler::on_generate_state_impl(GroundAction action, State successor_state, const PDDLFactories& pddl_factories) const
{
    std::cout << "[Algorithm] Action: " << std::make_tuple(action, std::cref(pddl_factories)) << "\n"
              << "[Algorithm] Successor: " << std::make_tuple(successor_state, std::cref(pddl_factories)) << "\n"
              << std::endl;
}

void DebugAlgorithmEventHandler::on_finish_f_layer_impl(uint64_t f_value, uint64_t num_expanded_states) const
{
    std::cout << "[Algorithm] Finished state expansion until f-layer " << f_value << " with num states " << num_expanded_states << std::endl << std::endl;
}

void DebugAlgorithmEventHandler::on_expand_state_impl(State state, const PDDLFactories& pddl_factories) const
{
    std::cout << "[Algorithm] ----------------------------------------"
              << "\n"
              << "[Algorithm] State: " << std::make_tuple(state, std::cref(pddl_factories)) << std::endl
              << std::endl;
}

void DebugAlgorithmEventHandler::on_start_search_impl(State initial_state, const PDDLFactories& pddl_factories) const
{
    std::cout << "[Algorithm] Search started."
              << "\n"
              << "[Algorithm] Initial: " << std::make_tuple(initial_state, std::cref(pddl_factories)) << std::endl;
}

void DebugAlgorithmEventHandler::on_end_search_impl() const
{
    std::cout << "[Algorithm] Search ended."
              << "\n"
              << m_statistics << std::endl;
}

void DebugAlgorithmEventHandler::on_solved_impl(const GroundActionList& ground_action_plan) const
{
    auto plan = to_plan(ground_action_plan);
    std::cout << "[Algorithm] Plan found with cost: " << plan.get_cost() << std::endl;
    for (size_t i = 0; i < plan.get_actions().size(); ++i)
    {
        std::cout << "[Algorithm] " << i + 1 << ". " << plan.get_actions()[i] << std::endl;
    }
}

void DebugAlgorithmEventHandler::on_exhausted_impl() const { std::cout << "[Algorithm] Exhausted!" << std::endl; }
}
