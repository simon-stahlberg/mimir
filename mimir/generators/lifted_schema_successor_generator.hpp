#if !defined(PLANNERS_LIFTED_SCHEMA_SUCCESSOR_GENERATOR_HPP_)
#define PLANNERS_LIFTED_SCHEMA_SUCCESSOR_GENERATOR_HPP_

#include "../formalism/action.hpp"
#include "../formalism/action_schema.hpp"
#include "../formalism/problem.hpp"
#include "../formalism/state.hpp"
#include "successor_generator.hpp"

#include <algorithm>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace planners
{
    // TODO: Optimize this by translating all action schemas into more efficient representations.
    // Instead of using an Object-Oriented representation, make it flatter and refer to parameters by their position rather than the object.
    // Additionally, flatten literals to eliminate the need to follow pointers.

    struct Assignment
    {
        formalism::Parameter parameter;
        formalism::Object object;

        Assignment(formalism::Parameter parameter, formalism::Object object) : parameter(parameter), object(object) {}
    };

    struct AssignmentPair
    {
        std::size_t first_position;
        std::size_t second_position;
        Assignment first_assignment;
        Assignment second_assignment;

        AssignmentPair(std::size_t first_position, const Assignment& first_assignment, std::size_t second_position, const Assignment& second_assignment) :
            first_position(first_position),
            second_position(second_position),
            first_assignment(first_assignment),
            second_assignment(second_assignment)
        {
        }
    };

    class LiftedSchemaSuccessorGenerator
    {
      private:
        using ParameterAssignmentsMap = std::unordered_map<formalism::Parameter, std::unordered_set<formalism::Object>>;

        formalism::ProblemDescription problem_;
        formalism::ActionSchema action_schema;
        ParameterAssignmentsMap objects_by_parameter_type;

        std::vector<Assignment> to_vertex_assignment;
        std::vector<AssignmentPair> statically_consistent_assignments;
        std::vector<formalism::Literal> static_precondition;
        std::vector<formalism::Literal> dynamic_precondition;
        std::vector<std::vector<std::size_t>> partitions_;

        static std::vector<std::vector<bool>> build_assignment_sets(const formalism::ProblemDescription& problem, const std::vector<uint32_t>& ranks);

        bool literal_all_consistent(const std::vector<std::vector<bool>>& assignment_sets,
                                    const formalism::LiteralList& literals,
                                    const Assignment& first_assignment,
                                    const Assignment& second_assignment) const;

        bool nullary_preconditions_hold(const formalism::State& state) const;

        bool has_consistent_effect(const formalism::Action& action) const;

        bool
        nullary_case(const std::chrono::high_resolution_clock::time_point end_time, const formalism::State& state, formalism::ActionList& out_actions) const;

        bool unary_case(const std::chrono::high_resolution_clock::time_point end_time, const formalism::State& state, formalism::ActionList& out_actions) const;

        bool general_case(const std::chrono::high_resolution_clock::time_point end_time,
                          const formalism::State& state,
                          const std::vector<std::vector<bool>>& assignment_sets,
                          formalism::ActionList& out_actions) const;

        formalism::ActionList nullary_case(const formalism::State& state) const;

        formalism::ActionList unary_case(const formalism::State& state) const;

        formalism::ActionList general_case(const formalism::State& state, const std::vector<std::vector<bool>>& assignment_sets) const;

        formalism::ActionList get_applicable_actions(const formalism::State& state, const std::vector<std::vector<bool>>& assignment_sets) const;

        friend class LiftedSuccessorGenerator;

      public:
        LiftedSchemaSuccessorGenerator(const formalism::ActionSchema& action_schema, const formalism::ProblemDescription& problem);

        formalism::ActionList get_applicable_actions(const formalism::State& state) const;

        formalism::ActionList get_actions() const;

        bool get_actions(const std::chrono::high_resolution_clock::time_point end_time, formalism::ActionList& out_actions) const;
    };
}  // namespace planners

#endif  // PLANNERS_LIFTED_SCHEMA_SUCCESSOR_GENERATOR_HPP_
