#include "../../include/mimir/datastructures/robin_map.hpp"
#include "../../include/mimir/search/breadth_first_search.hpp"

#include <algorithm>
#include <deque>

namespace mimir::planners
{
    BreadthFirstSearchImpl::BreadthFirstSearchImpl(const mimir::formalism::ProblemDescription& problem,
                                                   const mimir::planners::SuccessorGenerator& successor_generator) :
        SearchBase(problem),
        problem_(problem),
        successor_generator_(successor_generator),
        max_g_value_(-1),
        max_depth_(-1),
        expanded_(0),
        generated_(0)
    {
    }

    void BreadthFirstSearchImpl::reset_statistics()
    {
        max_g_value_ = -1;
        max_depth_ = -1;
        expanded_ = 0;
        generated_ = 0;
    }

    std::map<std::string, std::variant<int32_t, double>> BreadthFirstSearchImpl::get_statistics() const
    {
        std::map<std::string, std::variant<int32_t, double>> statistics;
        statistics["expanded"] = expanded_;
        statistics["generated"] = generated_;
        statistics["max_depth"] = max_depth_;
        statistics["max_g_value"] = max_g_value_;
        return statistics;
    }

    SearchResult BreadthFirstSearchImpl::plan(mimir::formalism::ActionList& out_plan)
    {
        reset_statistics();
        int32_t last_depth = -1;  // Used to notify handlers

        struct Frame
        {
            mimir::formalism::State state;
            mimir::formalism::Action predecessor_action;
            int32_t predecessor_index;
            int32_t depth;
            double g_value;
        };

        mimir::tsl::robin_map<mimir::formalism::State, int32_t> state_indices;
        std::deque<Frame> frame_list;
        std::deque<int32_t> open_list;

        {  // Initialize data-structures
            // We want the index of the initial state to be 1 for convenience.
            frame_list.emplace_back(Frame { nullptr, nullptr, -1, 0, 0.0 });

            // Add the initial state to the data-structures
            const int32_t initial_index = static_cast<int32_t>(frame_list.size());
            const auto initial_state = this->initial_state;
            state_indices[initial_state] = initial_index;
            frame_list.emplace_back(Frame { initial_state, nullptr, -1, 0, 0.0 });
            open_list.emplace_back(initial_index);
        }

        while (open_list.size() > 0)
        {
            const auto index = open_list.front();
            const auto& frame = frame_list[index];
            open_list.pop_front();

            max_depth_ = std::max(max_depth_, frame.depth);
            max_g_value_ = std::max(max_g_value_, frame.g_value);

            if (last_depth < frame.depth)
            {
                last_depth = frame.depth;
                notify_handlers();
            }

            if (should_abort)
            {
                return SearchResult::ABORTED;
            }

            if (mimir::formalism::literals_hold(problem_->goal, frame.state))
            {
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

            ++expanded_;

            const auto applicable_actions = successor_generator_->get_applicable_actions(frame.state);

            for (const auto& action : applicable_actions)
            {
                const auto successor_state = mimir::formalism::apply(action, frame.state);
                auto& successor_index = state_indices[successor_state];  // Reference is used to update state_indices

                if (successor_index == 0)
                {
                    // If successor_index is 0, then we haven't seen the state as it is reserved by the dummy frame that we added earlier.

                    ++generated_;
                    successor_index = static_cast<int32_t>(frame_list.size());
                    frame_list.emplace_back(Frame { successor_state, action, index, frame.depth + 1, frame.g_value + action->cost });
                    open_list.emplace_back(successor_index);
                }
            }
        }

        return SearchResult::UNSOLVABLE;
    }

    BreadthFirstSearch create_breadth_first_search(const mimir::formalism::ProblemDescription& problem,
                                                   const mimir::planners::SuccessorGenerator& successor_generator)
    {
        return std::make_shared<BreadthFirstSearchImpl>(problem, successor_generator);
    }
}  // namespace planners
