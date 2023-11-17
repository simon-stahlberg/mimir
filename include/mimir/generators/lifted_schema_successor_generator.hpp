#ifndef MIMIR_PLANNERS_LIFTED_SCHEMA_SUCCESSOR_GENERATOR_HPP_
#define MIMIR_PLANNERS_LIFTED_SCHEMA_SUCCESSOR_GENERATOR_HPP_

#include "../datastructures/robin_map.hpp"
#include "../formalism/action.hpp"
#include "../formalism/action_schema.hpp"
#include "../formalism/problem.hpp"
#include "../formalism/state.hpp"
#include "flat_action_schema.hpp"
#include "successor_generator.hpp"

#include <algorithm>
#include <chrono>
#include <memory>
#include <vector>

namespace mimir::planners
{
    struct Assignment
    {
        uint32_t parameter_index;
        uint32_t object_id;

        Assignment(uint32_t parameter_index, uint32_t object_id) : parameter_index(parameter_index), object_id(object_id) {}
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
        mimir::formalism::DomainDescription domain_;
        mimir::formalism::ProblemDescription problem_;
        mimir::planners::FlatActionSchema flat_action_schema_;
        mimir::tsl::robin_map<uint32_t, std::vector<uint32_t>> objects_by_parameter_type;

        std::vector<Assignment> to_vertex_assignment;
        std::vector<AssignmentPair> statically_consistent_assignments;
        std::vector<std::vector<std::size_t>> partitions_;

        static std::vector<std::vector<bool>> build_assignment_sets(const mimir::formalism::DomainDescription& domain,
                                                                    const mimir::formalism::ProblemDescription& problem,
                                                                    const std::vector<uint32_t>& ranks);

        bool literal_all_consistent(const std::vector<std::vector<bool>>& assignment_sets,
                                    const std::vector<mimir::planners::FlatLiteral>& literals,
                                    const Assignment& first_assignment,
                                    const Assignment& second_assignment) const;

        mimir::formalism::ObjectList ground_parameters(const std::vector<ParameterIndexOrConstantId>& parameters,
                                                       const mimir::formalism::ObjectList& terms) const;

        mimir::formalism::Literal ground_literal(const FlatLiteral& literal, const mimir::formalism::ObjectList& terms) const;

        mimir::formalism::Action create_action(mimir::formalism::ObjectList&& terms) const;

        bool nullary_preconditions_hold(const mimir::formalism::State& state) const;

        bool has_consistent_effect(const mimir::formalism::Action& action) const;

        bool nullary_case(const std::chrono::high_resolution_clock::time_point end_time,
                          const mimir::formalism::State& state,
                          mimir::formalism::ActionList& out_actions) const;

        bool unary_case(const std::chrono::high_resolution_clock::time_point end_time,
                        const mimir::formalism::State& state,
                        mimir::formalism::ActionList& out_actions) const;

        bool general_case(const std::chrono::high_resolution_clock::time_point end_time,
                          const mimir::formalism::State& state,
                          const std::vector<std::vector<bool>>& assignment_sets,
                          mimir::formalism::ActionList& out_actions) const;

        mimir::formalism::ActionList nullary_case(const mimir::formalism::State& state) const;

        mimir::formalism::ActionList unary_case(const mimir::formalism::State& state) const;

        mimir::formalism::ActionList general_case(const mimir::formalism::State& state, const std::vector<std::vector<bool>>& assignment_sets) const;

        mimir::formalism::ActionList get_applicable_actions(const mimir::formalism::State& state, const std::vector<std::vector<bool>>& assignment_sets) const;

        friend class LiftedSuccessorGenerator;

      public:
        LiftedSchemaSuccessorGenerator(const mimir::formalism::ActionSchema& action_schema, const mimir::formalism::ProblemDescription& problem);

        mimir::formalism::ActionList get_applicable_actions(const mimir::formalism::State& state) const;

        bool get_applicable_actions(const std::chrono::high_resolution_clock::time_point end_time,
                                    const mimir::formalism::State& state,
                                    mimir::formalism::ActionList& out_actions) const;
    };
}  // namespace planners

#endif  // MIMIR_PLANNERS_LIFTED_SCHEMA_SUCCESSOR_GENERATOR_HPP_
