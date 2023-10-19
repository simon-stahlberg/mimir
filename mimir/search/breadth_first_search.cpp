#include "../algorithms/robin_map.hpp"
#include "breadth_first_search.hpp"

#include <algorithm>
#include <deque>

namespace planners
{
    BreadthFirstSearchImpl::BreadthFirstSearchImpl(const formalism::ProblemDescription& problem, const planners::SuccessorGenerator& successor_generator) :
        SearchBase(),
        statistics_(),
        problem_(problem),
        successor_generator_(successor_generator)
    {
    }

    std::map<std::string, std::variant<int32_t, double>> BreadthFirstSearchImpl::get_statistics() const { return statistics_; }

    SearchResult BreadthFirstSearchImpl::plan(formalism::ActionList& out_plan)
    {
        statistics_.clear();
        int32_t expanded = 0;
        int32_t generated = 0;
        int32_t max_depth = -1;
        double max_g_value = -1;
        int32_t last_depth = -1;  // Used to notify handlers

        struct Frame
        {
            formalism::State state;
            formalism::Action predecessor_action;
            int32_t predecessor_index;
            int32_t depth;
            double g_value;
        };

        tsl::robin_map<formalism::State, int32_t> state_indices;
        std::deque<Frame> frame_list;
        std::deque<int32_t> open_list;

        {  // Initialize data-structures
            // We want the index of the initial state to be 1 for convenience.
            frame_list.emplace_back(Frame { nullptr, nullptr, -1, 0, 0.0 });

            // Add the initial state to the data-structures
            const int32_t initial_index = static_cast<int32_t>(frame_list.size());
            const auto initial_state = formalism::create_state(problem_->initial, problem_);
            state_indices[initial_state] = initial_index;
            frame_list.emplace_back(Frame { initial_state, nullptr, -1, 0, 0.0 });
            open_list.emplace_back(initial_index);
        }

        while (open_list.size() > 0)
        {
            const auto index = open_list.front();
            const auto& frame = frame_list[index];
            open_list.pop_front();

            max_depth = std::max(max_depth, frame.depth);
            max_g_value = std::max(max_g_value, frame.g_value);

            if (last_depth < frame.depth)
            {
                last_depth = frame.depth;
                statistics_["expanded"] = expanded;
                statistics_["generated"] = generated;
                statistics_["max_depth"] = max_depth;
                statistics_["max_g_value"] = max_g_value;
                notify_handlers();
            }

            if (should_abort)
            {
                return SearchResult::ABORTED;
            }

            if (formalism::literals_hold(problem_->goal, frame.state))
            {
                // Update the statistics to the final values

                statistics_["expanded"] = expanded;
                statistics_["generated"] = generated;
                statistics_["max_depth"] = max_depth;
                statistics_["max_g_value"] = max_g_value;

                // Reconstruct the path to the goal state
                out_plan.clear();
                auto current_frame = frame;

                while (current_frame.predecessor_action)
                {
                    out_plan.emplace_back(current_frame.predecessor_action);
                    current_frame = frame_list.at(current_frame.predecessor_index);
                }

                std::reverse(out_plan.begin(), out_plan.end());
                return SearchResult::SOLVED;
            }

            ++expanded;

            const auto applicable_actions = successor_generator_->get_applicable_actions(frame.state);

            for (const auto& action : applicable_actions)
            {
                const auto successor_state = formalism::apply(action, frame.state);
                auto& successor_index = state_indices[successor_state];  // Reference is used to update state_indices

                if (successor_index == 0)
                {
                    // If successor_index is 0, then we haven't seen the state as it is reserved by the dummy frame that we added earlier.

                    ++generated;
                    successor_index = static_cast<int32_t>(frame_list.size());
                    frame_list.emplace_back(Frame { successor_state, action, index, frame.depth + 1, frame.g_value + action->cost });
                    open_list.emplace_back(successor_index);
                }
            }
        }

        // Update the statistics to the final values

        statistics_["expanded"] = expanded;
        statistics_["generated"] = generated;
        statistics_["max_depth"] = max_depth;
        statistics_["max_g_value"] = max_g_value;

        return SearchResult::UNSOLVABLE;
    }

    BreadthFirstSearch create_breadth_first_search(const formalism::ProblemDescription& problem, const planners::SuccessorGenerator& successor_generator)
    {
        return std::make_shared<BreadthFirstSearchImpl>(problem, successor_generator);
    }
}  // namespace planners
