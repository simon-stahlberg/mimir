#ifndef MIMIR_PLANNERS_COMPLETE_STATE_SPACE_HPP_
#define MIMIR_PLANNERS_COMPLETE_STATE_SPACE_HPP_

#include "state_space.hpp"

namespace mimir::planners
{
    class CompleteStateSpaceImpl;
    using CompleteStateSpace = std::shared_ptr<CompleteStateSpaceImpl>;

    struct StateInfo;

    class CompleteStateSpaceImpl : public StateSpaceImpl
    {
      private:
        std::vector<mimir::formalism::State> states_;
        std::vector<mimir::formalism::State> goal_states_;
        std::vector<StateInfo> state_infos_;
        std::vector<mimir::formalism::State> dead_end_states_;
        std::vector<std::vector<mimir::formalism::State>> states_by_distance_;
        std::vector<std::vector<mimir::formalism::Transition>> forward_transitions_;
        std::vector<std::vector<mimir::formalism::Transition>> backward_transitions_;
        mimir::tsl::robin_map<mimir::formalism::State, uint64_t> state_indices_;
        mutable std::vector<std::vector<int32_t>> state_distances_;

        // Since we return references of internal vectors, ensure that only create_statespaces can create this object.
        CompleteStateSpaceImpl(const mimir::formalism::ProblemDescription& problem);

        bool add_or_get_state(const mimir::formalism::State& state, uint64_t& out_index);

        void add_goal_state(const mimir::formalism::State& state);

        void add_transition(uint64_t from_state_index,
                            uint64_t to_state_index,
                            const mimir::formalism::Action& action,
                            uint64_t& out_from_forward_index,
                            uint64_t& out_to_backward_index);

        const mimir::formalism::Transition& get_forward_transition(uint64_t state_index, uint64_t transition_index) const;

        const mimir::formalism::Transition& get_backward_transition(uint64_t state_index, uint64_t transition_index) const;

        const mimir::formalism::TransitionList& get_forward(uint64_t state_index) const;

        const mimir::formalism::TransitionList& get_backward(uint64_t state_index) const;

        mimir::formalism::State get_state(uint64_t state_index) const;

        uint64_t get_state_index(const mimir::formalism::State& state) const;

        int32_t get_distance_to_goal(uint64_t state_index) const;

        int32_t get_distance_from_initial(uint64_t state_index) const;

        void set_distance_from_initial_state(uint64_t state_index, int32_t value);

        void set_distance_to_goal_state(uint64_t state_index, int32_t value);

      public:
        ~CompleteStateSpaceImpl() override;

        const std::vector<mimir::formalism::Transition>& get_forward_transitions(const mimir::formalism::State& state) const override;

        const std::vector<mimir::formalism::Transition>& get_backward_transitions(const mimir::formalism::State& state) const override;

        const std::vector<mimir::formalism::State>& get_states() const override;

        mimir::formalism::State get_initial_state() const override;

        uint64_t get_unique_index_of_state(const mimir::formalism::State& state) const override;

        bool is_dead_end_state(const mimir::formalism::State& state) const override;

        bool is_goal_state(const mimir::formalism::State& state) const override;

        int32_t get_distance_to_goal_state(const mimir::formalism::State& state) const override;

        int32_t get_distance_between_states(const mimir::formalism::State& from_state, const mimir::formalism::State& to_state) const override;

        int32_t get_longest_distance_to_goal_state() const override;

        int32_t get_distance_from_initial_state(const mimir::formalism::State& state) const override;

        mimir::formalism::State sample_state() const override;

        mimir::formalism::State sample_state_with_distance_to_goal(int32_t distance) const override;

        mimir::formalism::State sample_dead_end_state() const override;

        std::vector<mimir::formalism::State> get_goal_states() const override;

        uint64_t num_states() const override;

        uint64_t num_transitions() const override;

        uint64_t num_goal_states() const override;

        uint64_t num_dead_end_states() const override;

        friend CompleteStateSpace
        create_complete_state_space(const mimir::formalism::ProblemDescription&, const mimir::planners::SuccessorGenerator&, uint32_t);
    };

    CompleteStateSpace create_complete_state_space(const mimir::formalism::ProblemDescription& problem,
                                                   const mimir::planners::SuccessorGenerator& successor_generator,
                                                   uint32_t max_states = std::numeric_limits<uint32_t>::max());

}  // namespace mimir::planners

#endif  // MIMIR_PLANNERS_COMPLETE_STATE_SPACE_HPP_
