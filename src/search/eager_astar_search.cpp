#include "../../include/mimir/datastructures/robin_map.hpp"
#include "../../include/mimir/search/eager_astar_search.hpp"

#include <algorithm>
#include <deque>

namespace mimir::planners
{
    EagerAStarSearchImpl::EagerAStarSearchImpl(const mimir::formalism::ProblemDescription& problem,
                                               const mimir::planners::SuccessorGenerator& successor_generator,
                                               const mimir::planners::Heuristic& heuristic,
                                               const mimir::planners::OpenList& open_list) :
        SearchBase(problem),
        problem_(problem),
        successor_generator_(successor_generator),
        heuristic_(heuristic),
        open_list_(open_list),
        max_g_value_(-1),
        max_f_value_(-1),
        max_depth_(-1),
        expanded_(0),
        generated_(0),
        evaluated_(0)
    {
    }

    void EagerAStarSearchImpl::reset_statistics()
    {
        max_g_value_ = -1;
        max_f_value_ = -1;
        max_depth_ = -1;
        expanded_ = 0;
        generated_ = 0;
        evaluated_ = 0;
    }

    std::map<std::string, std::variant<int32_t, double>> EagerAStarSearchImpl::get_statistics() const
    {
        std::map<std::string, std::variant<int32_t, double>> statistics;
        statistics["expanded"] = expanded_;
        statistics["generated"] = generated_;
        statistics["evaluated"] = evaluated_;
        statistics["max_depth"] = max_depth_;
        statistics["max_g_value"] = max_g_value_;
        statistics["max_f_value"] = max_f_value_;
        return statistics;
    }

    SearchResult EagerAStarSearchImpl::plan(mimir::formalism::ActionList& out_plan)
    {
        if (open_list_->size() > 0)
        {
            throw std::runtime_error("open list is not initially empty");
        }

        reset_statistics();
        int32_t last_f_value = -1;  // Used to notify handlers

        struct Frame
        {
            mimir::formalism::State state;
            mimir::formalism::Action predecessor_action;
            int32_t predecessor_index;
            int32_t depth;
            double g_value;
            double h_value;
            bool closed;
        };

        mimir::tsl::robin_map<mimir::formalism::State, int32_t> state_indices;
        std::deque<Frame> frame_list;

        {  // Initialize data-structures
            // We want the index of the initial state to be 1 for convenience.
            frame_list.emplace_back(Frame { nullptr, nullptr, -1, 0, 0.0, 0.0, true });

            // Add the initial state to the data-structures
            const int32_t initial_index = static_cast<int32_t>(frame_list.size());
            const auto initial_state = this->initial_state;
            const auto initial_h_value = heuristic_->evaluate(initial_state);
            state_indices[initial_state] = initial_index;
            frame_list.emplace_back(Frame { initial_state, nullptr, -1, 0, 0.0, initial_h_value, false });
            open_list_->insert(initial_index, 0.0);
            ++evaluated_;
        }

        while (open_list_->size() > 0)
        {
            const auto index = open_list_->pop();
            auto& frame = frame_list[index];

            if (frame.closed)
            {
                continue;
            }

            frame.closed = true;
            const auto f_value = frame.g_value + frame.h_value;
            max_depth_ = std::max(max_depth_, frame.depth);
            max_g_value_ = std::max(max_g_value_, frame.g_value);
            max_f_value_ = std::max(max_f_value_, f_value);

            if (last_f_value < f_value)
            {
                last_f_value = f_value;
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
                const auto succ_state = mimir::formalism::apply(action, frame.state);
                auto& succ_index = state_indices[succ_state];  // Reference is used to update state_indices

                if (succ_index == 0)
                {
                    // If succ_index is 0, then we haven't seen the state as it is reserved by the dummy frame that we added earlier

                    succ_index = static_cast<int32_t>(frame_list.size());
                    const auto succ_g_value = frame.g_value + action->cost;
                    const auto succ_h_value = heuristic_->evaluate(succ_state);
                    const auto succ_f_value = succ_g_value + succ_h_value;
                    const auto succ_dead_end = HeuristicBase::is_dead_end(succ_h_value);
                    ++evaluated_;

                    frame_list.emplace_back(Frame { succ_state, action, index, frame.depth + 1, succ_g_value, succ_h_value, succ_dead_end });

                    if (!succ_dead_end)
                    {
                        open_list_->insert(succ_index, succ_f_value);
                        ++generated_;
                    }
                }
                else
                {
                    auto& succ_frame = frame_list[succ_index];
                    const auto succ_g_value = frame.g_value + action->cost;

                    if (!succ_frame.closed && (succ_g_value < succ_frame.g_value))
                    {
                        // We have found a better way to the next state; update the frame

                        succ_frame.predecessor_action = action;
                        succ_frame.predecessor_index = index;
                        succ_frame.depth = frame.depth + 1;
                        succ_frame.g_value = succ_g_value;

                        if (!HeuristicBase::is_dead_end(succ_frame.h_value))
                        {
                            // Instead of updating, we rely on the closed flag to ignore multiple entries in the same successor state

                            const auto succ_f_value = succ_g_value + succ_frame.h_value;
                            open_list_->insert(succ_index, succ_f_value);
                            ++generated_;
                        }
                    }
                }
            }
        }

        return SearchResult::UNSOLVABLE;
    }

    EagerAStarSearch create_eager_astar(const mimir::formalism::ProblemDescription& problem,
                                        const mimir::planners::SuccessorGenerator& successor_generator,
                                        const mimir::planners::Heuristic& heuristic,
                                        const mimir::planners::OpenList& open_list)
    {
        return std::make_shared<EagerAStarSearchImpl>(problem, successor_generator, heuristic, open_list);
    }
}  // namespace planners
