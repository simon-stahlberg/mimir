/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "../algorithms/kpkc.hpp"
#include "../formalism/atom.hpp"
#include "../formalism/help_functions.hpp"
#include "../formalism/type.hpp"
#include "lifted_schema_successor_generator.hpp"

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <limits>
#include <set>
#include <unordered_map>

std::size_t
get_assignment_position(int32_t first_position, int32_t first_object, int32_t second_position, int32_t second_object, int32_t arity, int32_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto rank = (first * (first_position + 1)) + (second * (second_position + 1)) + (third * (first_object + 1)) + (fourth * (second_object + 1));
    return (std::size_t) rank;
}

std::size_t num_assignments(int32_t arity, int32_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto max = (first * arity) + (second * arity) + (third * num_objects) + (fourth * num_objects);
    return (std::size_t)(max + 1);
}

namespace planners
{
    /**
     * @brief Builds a datastructure to efficiently test if an assignment is compatible with an atom in the given set.
     *
     * @return A set for each predicate that contains possible compatible assignments.
     */
    std::vector<std::vector<bool>> LiftedSchemaSuccessorGenerator::build_assignment_sets(const formalism::ProblemDescription& problem,
                                                                                         const std::vector<uint32_t>& ranks)
    {
        const auto num_objects = problem->objects.size();
        const auto& predicates = problem->domain->predicates;
        std::vector<std::vector<bool>> assignment_sets;
        assignment_sets.resize(predicates.size());

        for (const auto& predicate : predicates)
        {
            auto& assignment_set = assignment_sets[predicate->id];
            assignment_set.resize(num_assignments(predicate->arity, num_objects));
        }

        for (const auto& rank : ranks)
        {
            const auto& predicate_arity = problem->get_arity(rank);
            const auto& predicate_id = problem->get_predicate_id(rank);
            const auto& argument_ids = problem->get_argument_ids(rank);
            auto& assignment_set = assignment_sets[predicate_id];

            for (size_t first_position = 0; first_position < predicate_arity; ++first_position)
            {
                const auto& first_object_id = argument_ids[first_position];
                assignment_set[get_assignment_position(first_position, first_object_id, -1, -1, predicate_arity, num_objects)] = true;

                for (size_t second_position = first_position + 1; second_position < predicate_arity; ++second_position)
                {
                    const auto& second_object_id = argument_ids[second_position];
                    assignment_set[get_assignment_position(second_position, second_object_id, -1, -1, predicate_arity, num_objects)] = true;
                    assignment_set[get_assignment_position(first_position, first_object_id, second_position, second_object_id, predicate_arity, num_objects)] =
                        true;
                }
            }
        }

        return assignment_sets;
    }

    bool LiftedSchemaSuccessorGenerator::literal_all_consistent(const std::vector<std::vector<bool>>& assignment_sets,
                                                                const formalism::LiteralList& literals,
                                                                const Assignment& first_assignment,
                                                                const Assignment& second_assignment) const
    {
        // TODO: Profiling 18 %
        std::equal_to<formalism::Parameter> equals;

        for (const auto& literal : literals)
        {
            int32_t first_position = -1;
            int32_t second_position = -1;
            formalism::Object first_object = nullptr;
            formalism::Object second_object = nullptr;
            bool empty_assignment = true;
            const auto& arguments = literal->atom->arguments;
            const auto predicate_arity = literal->atom->predicate->arity;

            for (std::size_t index = 0; index < predicate_arity; ++index)
            {
                const auto& parameter = arguments[index];
                const auto is_constant = !parameter->is_free_variable();

                if (is_constant)
                {
                    if (predicate_arity <= 2)
                    {
                        if (first_position < 0)
                        {
                            first_position = index;
                            first_object = parameter;
                        }
                        else
                        {
                            second_position = index;
                            second_object = parameter;
                        }

                        empty_assignment = false;
                    }
                }
                else
                {
                    if (equals(first_assignment.parameter, parameter))
                    {
                        if (first_position < 0)
                        {
                            first_position = index;
                            first_object = first_assignment.object;
                        }
                        else
                        {
                            second_position = index;
                            second_object = first_assignment.object;
                            break;
                        }

                        empty_assignment = false;
                    }
                    else if (equals(second_assignment.parameter, parameter))
                    {
                        if (first_position < 0)
                        {
                            first_position = index;
                            first_object = second_assignment.object;
                        }
                        else
                        {
                            second_position = index;
                            second_object = second_assignment.object;
                            break;
                        }

                        empty_assignment = false;
                    }
                }
            }

            if (!empty_assignment)
            {
                const auto& predicate = literal->atom->predicate;
                const auto& assignment_set = assignment_sets[predicate->id];
                const auto assignment_rank = get_assignment_position(first_position,
                                                                     first_object ? first_object->id : -1,
                                                                     second_position,
                                                                     second_object ? second_object->id : -1,
                                                                     predicate->arity,
                                                                     problem_->objects.size());

                const auto consistent_with_state = assignment_set[assignment_rank];

                if (!literal->negated && !consistent_with_state)
                {
                    return false;
                }
                else if (literal->negated && consistent_with_state && ((predicate_arity == 1) || ((predicate_arity == 2) && (second_position >= 0))))
                {
                    return false;
                }
            }
        }

        return true;
    }

    LiftedSchemaSuccessorGenerator::LiftedSchemaSuccessorGenerator(const formalism::ActionSchema& action_schema, const formalism::ProblemDescription& problem) :
        problem_(problem),
        action_schema(action_schema),
        objects_by_parameter_type(),
        to_vertex_assignment(),
        statically_consistent_assignments(),
        static_precondition(),
        dynamic_precondition(),
        partitions_()
    {
        // Type information is used by the unary and general case

        if (action_schema->parameters.size() >= 1)
        {
            // Compatible objects by type

            for (const auto& parameter : action_schema->parameters)
            {
                std::unordered_set<formalism::Object> compatible_objects;

                for (const auto& object : problem->objects)
                {
                    if (formalism::is_subtype_of(object->type, parameter->type))
                    {
                        compatible_objects.insert(object);
                    }
                }

                objects_by_parameter_type.insert(std::make_pair(parameter, compatible_objects));
            }
        }

        // The following is only used by the general case

        if (action_schema->parameters.size() >= 2)
        {
            // Create a mapping between indices and parameter-object assignments

            for (const auto& parameter : action_schema->parameters)
            {
                const auto compatible_objects = objects_by_parameter_type.at(parameter);
                std::vector<std::size_t> partition;

                for (const auto& object : compatible_objects)
                {
                    partition.push_back(to_vertex_assignment.size());
                    to_vertex_assignment.push_back(Assignment(parameter, object));
                }

                partitions_.push_back(std::move(partition));
            }

            // Separate precondition based on static and dynamic literals
            const formalism::PredicateSet static_predicates(problem->domain->static_predicates.begin(), problem->domain->static_predicates.end());

            for (const auto& literal : action_schema->precondition)
            {
                if (static_predicates.find(literal->atom->predicate) != static_predicates.end())
                {
                    static_precondition.push_back(literal);
                }
                else
                {
                    dynamic_precondition.push_back(literal);
                }
            }

            // Filter assignment based on static atoms
            const auto assignment_sets = build_assignment_sets(problem_, problem->initial->get_static_ranks());

            for (size_t first_id = 0; first_id < to_vertex_assignment.size(); ++first_id)
            {
                for (size_t second_id = (first_id + 1); second_id < to_vertex_assignment.size(); ++second_id)
                {
                    const auto& first_assignment = to_vertex_assignment.at(first_id);
                    const auto& second_assignment = to_vertex_assignment.at(second_id);

                    if (first_assignment.parameter != second_assignment.parameter)
                    {
                        if (literal_all_consistent(assignment_sets, static_precondition, first_assignment, second_assignment))
                        {
                            statically_consistent_assignments.push_back(AssignmentPair(first_id, first_assignment, second_id, second_assignment));
                        }
                    }
                }
            }
        }
    }

    bool LiftedSchemaSuccessorGenerator::nullary_preconditions_hold(const formalism::State& state) const
    {
        for (const auto& literal : action_schema->precondition)
        {
            const auto predicate_arity = literal->atom->predicate->arity;

            if ((predicate_arity == 0) && !formalism::literal_holds(literal, state))
            {
                return false;
            }
        }

        return true;
    }

    bool LiftedSchemaSuccessorGenerator::nullary_case(const std::chrono::high_resolution_clock::time_point end_time,
                                                      const formalism::State& state,
                                                      formalism::ActionList& out_actions) const
    {
        if (std::chrono::high_resolution_clock::now() >= end_time)
        {
            return false;
        }

        const auto action = formalism::create_action(problem_, action_schema, formalism::ObjectList());

        if ((state == nullptr) || formalism::literals_hold(action->get_precondition(), state))
        {
            out_actions.push_back(action);
        }

        return true;
    }

    formalism::ActionList LiftedSchemaSuccessorGenerator::nullary_case(const formalism::State& state) const
    {
        formalism::ActionList applicable_actions;
        nullary_case(std::chrono::high_resolution_clock::time_point::max(), state, applicable_actions);
        return applicable_actions;
    }

    bool LiftedSchemaSuccessorGenerator::unary_case(const std::chrono::high_resolution_clock::time_point end_time,
                                                    const formalism::State& state,
                                                    formalism::ActionList& out_actions) const
    {
        const auto& parameter = action_schema->parameters.at(0);

        for (const auto& object : objects_by_parameter_type.at(parameter))
        {
            if (std::chrono::high_resolution_clock::now() >= end_time)
            {
                return false;
            }

            const auto action = formalism::create_action(problem_, action_schema, { std::make_pair(parameter, object) });

            if ((state == nullptr) || formalism::literals_hold(action->get_precondition(), state))
            {
                out_actions.push_back(action);
            }
        }

        return true;
    }

    formalism::ActionList LiftedSchemaSuccessorGenerator::unary_case(const formalism::State& state) const
    {
        formalism::ActionList applicable_actions;
        unary_case(std::chrono::high_resolution_clock::time_point::max(), state, applicable_actions);
        return applicable_actions;
    }

    bool LiftedSchemaSuccessorGenerator::general_case(const std::chrono::high_resolution_clock::time_point end_time,
                                                      const formalism::State& state,
                                                      const std::vector<std::vector<bool>>& assignment_sets,
                                                      formalism::ActionList& out_actions) const
    {
        const auto num_parameters = action_schema->parameters.size();
        const auto num_vertices = to_vertex_assignment.size();

        // Allocate memory that for both the outer array and the inner arrays.
        std::vector<boost::dynamic_bitset<>> adjacency_matrix(num_vertices, boost::dynamic_bitset<>(num_vertices));

        if (state)
        {
            for (const auto& assignment : statically_consistent_assignments)
            {
                if (std::chrono::high_resolution_clock::now() >= end_time)
                {
                    return false;
                }

                const auto& first_assignment = assignment.first_assignment;
                const auto& second_assignment = assignment.second_assignment;

                if (literal_all_consistent(assignment_sets, dynamic_precondition, first_assignment, second_assignment))
                {
                    const auto first_id = assignment.first_position;
                    const auto second_id = assignment.second_position;
                    auto& first_row = adjacency_matrix[first_id];
                    auto& second_row = adjacency_matrix[second_id];
                    first_row[second_id] = 1;
                    second_row[first_id] = 1;
                }
            }
        }
        else
        {
            for (const auto& assignment : statically_consistent_assignments)
            {
                if (std::chrono::high_resolution_clock::now() >= end_time)
                {
                    return false;
                }

                const auto first_id = assignment.first_position;
                const auto second_id = assignment.second_position;
                auto& first_row = adjacency_matrix[first_id];
                auto& second_row = adjacency_matrix[second_id];
                first_row[second_id] = 1;
                second_row[first_id] = 1;
            }
        }

        // Find all cliques of size num_parameters whose labels denote complete assignments that might yield an applicable precondition. The relatively few
        // atoms in the state (compared to the number of possible atoms) lead to very sparse graphs, so the number of maximal cliques of maximum size (#
        // parameters) tends to be very small.

        std::vector<std::vector<std::size_t>> cliques;
        if (!algorithms::find_all_k_cliques_in_k_partite_graph(end_time, adjacency_matrix, partitions_, cliques))
        {
            return false;
        }

        for (const auto& clique : cliques)
        {
            if (std::chrono::high_resolution_clock::now() >= end_time)
            {
                return false;
            }

            formalism::ParameterAssignment assignment(num_parameters);

            for (std::size_t vertex_index = 0; vertex_index < num_parameters; ++vertex_index)
            {
                const auto vertex_id = clique[vertex_index];
                const auto& vertex_assignment = to_vertex_assignment.at(vertex_id);
                assignment.insert(std::make_pair(vertex_assignment.parameter, vertex_assignment.object));
            }

            const auto action = formalism::create_action(problem_, action_schema, std::move(assignment));

            if ((state == nullptr) || formalism::literals_hold(action->get_precondition(), state, 3))
            {
                out_actions.push_back(action);
            }
        }

        return true;
    }

    formalism::ActionList LiftedSchemaSuccessorGenerator::general_case(const formalism::State& state,
                                                                       const std::vector<std::vector<bool>>& assignment_sets) const
    {
        formalism::ActionList applicable_actions;
        general_case(std::chrono::high_resolution_clock::time_point::max(), state, assignment_sets, applicable_actions);
        return applicable_actions;
    }

    formalism::ActionList LiftedSchemaSuccessorGenerator::get_applicable_actions(const formalism::State& state,
                                                                                 const std::vector<std::vector<bool>>& assignment_sets) const
    {
        if (!nullary_preconditions_hold(state))
        {
            return formalism::ActionList();
        }

        const auto num_parameters = action_schema->parameters.size();

        if (num_parameters == 0)
        {
            return nullary_case(state);
        }
        else if (num_parameters == 1)
        {
            return unary_case(state);
        }
        else
        {
            return general_case(state, assignment_sets);
        }
    }

    formalism::ActionList LiftedSchemaSuccessorGenerator::get_applicable_actions(const formalism::State& state) const
    {
        const auto assignment_sets = build_assignment_sets(problem_, state->get_dynamic_ranks());
        return get_applicable_actions(state, assignment_sets);
    }

    formalism::ActionList LiftedSchemaSuccessorGenerator::get_actions() const
    {
        const auto num_parameters = action_schema->parameters.size();

        if (num_parameters == 0)
        {
            return nullary_case(nullptr);
        }
        else if (num_parameters == 1)
        {
            return unary_case(nullptr);
        }
        else
        {
            return general_case(nullptr, {});
        }
    }

    bool LiftedSchemaSuccessorGenerator::get_actions(const std::chrono::high_resolution_clock::time_point end_time, formalism::ActionList& out_actions) const
    {
        const auto num_parameters = action_schema->parameters.size();

        if (num_parameters == 0)
        {
            if (!nullary_case(end_time, nullptr, out_actions))
            {
                return false;
            }
        }
        else if (num_parameters == 1)
        {
            if (!unary_case(end_time, nullptr, out_actions))
            {
                return false;
            }
        }
        else if (!general_case(end_time, nullptr, {}, out_actions))
        {
            return false;
        }

        return true;
    }
}  // namespace planners
