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

#include "../../include/mimir/algorithms/kpkc.hpp"
#include "../../include/mimir/formalism/atom.hpp"
#include "../../include/mimir/formalism/type.hpp"
#include "../../include/mimir/generators/lifted_schema_successor_generator.hpp"
#include "../formalism/help_functions.hpp"

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <limits>
#include <set>

namespace mimir::planners
{
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

    /**
     * @brief Builds a datastructure to efficiently test if an assignment is compatible with an atom in the given set.
     *
     * @return A set for each predicate that contains possible compatible assignments.
     */
    std::vector<std::vector<bool>> LiftedSchemaSuccessorGenerator::build_assignment_sets(const mimir::formalism::DomainDescription& domain,
                                                                                         const mimir::formalism::ProblemDescription& problem,
                                                                                         const std::vector<uint32_t>& ranks)
    {
        const auto num_objects = problem->objects.size();
        const auto& predicates = domain->predicates;
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
                                                                const std::vector<mimir::planners::FlatLiteral>& literals,
                                                                const Assignment& first_assignment,
                                                                const Assignment& second_assignment) const
    {
        for (const auto& literal : literals)
        {
            int32_t first_position = -1;
            int32_t second_position = -1;
            int32_t first_object_id = -1;
            int32_t second_object_id = -1;
            bool empty_assignment = true;

            for (std::size_t index = 0; index < literal.arity; ++index)
            {
                const auto& term = literal.arguments[index];

                if (term.is_constant())
                {
                    if (literal.arity <= 2)
                    {
                        const auto term_id = term.get_value();

                        if (first_position < 0)
                        {
                            first_position = index;
                            first_object_id = static_cast<int32_t>(term_id);
                        }
                        else
                        {
                            second_position = index;
                            second_object_id = static_cast<int32_t>(term_id);
                        }

                        empty_assignment = false;
                    }
                }
                else
                {
                    const auto term_index = term.get_value();

                    if (first_assignment.parameter_index == term_index)
                    {
                        if (first_position < 0)
                        {
                            first_position = index;
                            first_object_id = static_cast<int32_t>(first_assignment.object_id);
                        }
                        else
                        {
                            second_position = index;
                            second_object_id = static_cast<int32_t>(first_assignment.object_id);
                            break;
                        }

                        empty_assignment = false;
                    }
                    else if (second_assignment.parameter_index == term_index)
                    {
                        if (first_position < 0)
                        {
                            first_position = index;
                            first_object_id = static_cast<int32_t>(second_assignment.object_id);
                        }
                        else
                        {
                            second_position = index;
                            second_object_id = static_cast<int32_t>(second_assignment.object_id);
                            break;
                        }

                        empty_assignment = false;
                    }
                }
            }

            if (!empty_assignment)
            {
                const auto& assignment_set = assignment_sets[literal.predicate_id];
                const auto assignment_rank = get_assignment_position(first_position,
                                                                     first_object_id,
                                                                     second_position,
                                                                     second_object_id,
                                                                     static_cast<int32_t>(literal.arity),
                                                                     static_cast<int32_t>(problem_->objects.size()));

                const auto consistent_with_state = assignment_set[assignment_rank];

                if (!literal.negated && !consistent_with_state)
                {
                    return false;
                }
                else if (literal.negated && consistent_with_state && ((literal.arity == 1) || ((literal.arity == 2) && (second_position >= 0))))
                {
                    return false;
                }
            }
        }

        return true;
    }

    mimir::formalism::ObjectList LiftedSchemaSuccessorGenerator::ground_parameters(const std::vector<ParameterIndexOrConstantId>& parameters,
                                                                                   const mimir::formalism::ObjectList& terms) const
    {
        mimir::formalism::ObjectList atom_terms;

        for (const auto& term : parameters)
        {
            if (term.is_constant())
            {
                atom_terms.emplace_back(problem_->get_object(term.get_value()));
            }
            else
            {
                atom_terms.emplace_back(terms[term.get_value()]);
            }
        }

        return atom_terms;
    }

    mimir::formalism::Literal LiftedSchemaSuccessorGenerator::ground_literal(const FlatLiteral& literal, const mimir::formalism::ObjectList& terms) const
    {
        const auto& atom_predicate = literal.source->atom->predicate;
        const auto ground_atom = mimir::formalism::create_atom(atom_predicate, ground_parameters(literal.arguments, terms));
        return mimir::formalism::create_literal(ground_atom, literal.negated);
    }

    mimir::formalism::Action LiftedSchemaSuccessorGenerator::create_action(mimir::formalism::ObjectList&& terms) const
    {
        // Get the precondition of the ground action

        mimir::formalism::LiteralList precondition;

        for (const auto& literal : flat_action_schema_.static_precondition)
        {
            precondition.emplace_back(ground_literal(literal, terms));
        }

        for (const auto& literal : flat_action_schema_.fluent_precondition)
        {
            precondition.emplace_back(ground_literal(literal, terms));
        }

        // Get the effect of the ground action

        mimir::formalism::LiteralList unconditional_effect;

        for (const auto& literal : flat_action_schema_.unconditional_effect)
        {
            unconditional_effect.emplace_back(ground_literal(literal, terms));
        }

        mimir::formalism::ImplicationList conditional_effect;

        for (const auto& flat_implication : flat_action_schema_.conditional_effect)
        {
            mimir::formalism::LiteralList antecedent;
            mimir::formalism::LiteralList consequence;

            for (const auto& literal : flat_implication.antecedent)
            {
                antecedent.emplace_back(ground_literal(literal, terms));
            }

            for (const auto& literal : flat_implication.consequence)
            {
                consequence.emplace_back(ground_literal(literal, terms));
            }

            conditional_effect.emplace_back(std::move(antecedent), std::move(consequence));
        }

        // Get the cost of the ground action

        double cost;
        const auto cost_function = flat_action_schema_.source->cost;

        if (cost_function->is_constant())
        {
            cost = cost_function->get_constant();
        }
        else
        {
            const auto cost_atom_schema = flat_action_schema_.source->cost->get_atom();
            const auto cost_atom = mimir::formalism::create_atom(cost_atom_schema->predicate, ground_parameters(flat_action_schema_.cost_arguments, terms));
            cost = problem_->atom_costs.at(cost_atom);
        }

        if (cost_function->get_operation() == mimir::formalism::FunctionOperation::DECREASE)
        {
            cost = -cost;
        }

        // Finally, create the ground action

        return mimir::formalism::create_action(problem_,
                                               flat_action_schema_.source,
                                               std::move(terms),
                                               std::move(precondition),
                                               std::move(unconditional_effect),
                                               std::move(conditional_effect),
                                               cost);
    }

    LiftedSchemaSuccessorGenerator::LiftedSchemaSuccessorGenerator(const mimir::formalism::ActionSchema& action_schema,
                                                                   const mimir::formalism::ProblemDescription& problem) :
        domain_(problem->domain),
        problem_(problem),
        flat_action_schema_(FlatActionSchema(problem->domain, action_schema)),
        objects_by_parameter_type(),
        to_vertex_assignment(),
        statically_consistent_assignments(),
        partitions_()
    {
        // Type information is used by the unary and general case

        if (flat_action_schema_.arity >= 1)
        {
            // Compatible objects by type

            for (const auto& parameter : flat_action_schema_.get_parameters())
            {
                std::vector<uint32_t> compatible_objects;

                for (const auto& object : problem->objects)
                {
                    if (mimir::formalism::is_subtype_of(object->type, parameter->type))
                    {
                        compatible_objects.emplace_back(object->id);
                    }
                }

                objects_by_parameter_type.emplace(flat_action_schema_.get_parameter_index(parameter), std::move(compatible_objects));
            }
        }

        // The following is only used by the general case

        if (flat_action_schema_.arity >= 2)
        {
            // Create a mapping between indices and parameter-object assignments

            for (uint32_t parameter_index = 0; parameter_index < flat_action_schema_.arity; ++parameter_index)
            {
                const auto& compatible_objects = objects_by_parameter_type.at(parameter_index);
                std::vector<std::size_t> partition;

                for (const auto& object_id : compatible_objects)
                {
                    partition.push_back(to_vertex_assignment.size());
                    to_vertex_assignment.push_back(Assignment(parameter_index, object_id));
                }

                partitions_.push_back(std::move(partition));
            }

            // Filter assignment based on static atoms
            const auto initial_state = mimir::formalism::create_state(problem->initial, problem);
            const auto assignment_sets = build_assignment_sets(domain_, problem_, initial_state->get_static_ranks());

            for (size_t first_id = 0; first_id < to_vertex_assignment.size(); ++first_id)
            {
                for (size_t second_id = (first_id + 1); second_id < to_vertex_assignment.size(); ++second_id)
                {
                    const auto& first_assignment = to_vertex_assignment.at(first_id);
                    const auto& second_assignment = to_vertex_assignment.at(second_id);

                    if (first_assignment.parameter_index != second_assignment.parameter_index)
                    {
                        if (literal_all_consistent(assignment_sets, flat_action_schema_.static_precondition, first_assignment, second_assignment))
                        {
                            statically_consistent_assignments.push_back(AssignmentPair(first_id, first_assignment, second_id, second_assignment));
                        }
                    }
                }
            }

            // The previous code does not handle static nullary atoms correctly

            for (const auto& literal : flat_action_schema_.static_precondition)
            {
                if (literal.arity == 0)
                {
                    const auto negated = literal.source->negated;
                    const auto& atom = literal.source->atom;
                    const auto contains = static_cast<bool>(std::count(problem_->initial.cbegin(), problem_->initial.cend(), atom));

                    if (contains == negated)
                    {
                        statically_consistent_assignments.clear();
                        break;
                    }
                }
            }
        }
    }

    bool LiftedSchemaSuccessorGenerator::nullary_preconditions_hold(const mimir::formalism::State& state) const
    {
        for (const auto& literal : flat_action_schema_.fluent_precondition)
        {
            if ((literal.arity == 0) && !mimir::formalism::literal_holds(literal.source, state))
            {
                return false;
            }
        }

        return true;
    }

    bool LiftedSchemaSuccessorGenerator::nullary_case(const std::chrono::high_resolution_clock::time_point end_time,
                                                      const mimir::formalism::State& state,
                                                      mimir::formalism::ActionList& out_actions) const
    {
        if (std::chrono::high_resolution_clock::now() >= end_time)
        {
            return false;
        }

        const auto action = create_action(mimir::formalism::ObjectList {});

        if (mimir::formalism::literals_hold(action->get_precondition(), state))
        {
            out_actions.push_back(action);
        }

        return true;
    }

    mimir::formalism::ActionList LiftedSchemaSuccessorGenerator::nullary_case(const mimir::formalism::State& state) const
    {
        mimir::formalism::ActionList applicable_actions;
        nullary_case(std::chrono::high_resolution_clock::time_point::max(), state, applicable_actions);
        return applicable_actions;
    }

    bool LiftedSchemaSuccessorGenerator::unary_case(const std::chrono::high_resolution_clock::time_point end_time,
                                                    const mimir::formalism::State& state,
                                                    mimir::formalism::ActionList& out_actions) const
    {
        for (const auto& object_id : objects_by_parameter_type.at(0))
        {
            if (std::chrono::high_resolution_clock::now() >= end_time)
            {
                return false;
            }

            const auto action = create_action({ problem_->get_object(object_id) });

            if (mimir::formalism::literals_hold(action->get_precondition(), state))
            {
                out_actions.push_back(action);
            }
        }

        return true;
    }

    mimir::formalism::ActionList LiftedSchemaSuccessorGenerator::unary_case(const mimir::formalism::State& state) const
    {
        mimir::formalism::ActionList applicable_actions;
        unary_case(std::chrono::high_resolution_clock::time_point::max(), state, applicable_actions);
        return applicable_actions;
    }

    bool LiftedSchemaSuccessorGenerator::general_case(const std::chrono::high_resolution_clock::time_point end_time,
                                                      const mimir::formalism::State& state,
                                                      const std::vector<std::vector<bool>>& assignment_sets,
                                                      mimir::formalism::ActionList& out_actions) const
    {
        assert(state);

        if (std::chrono::high_resolution_clock::now() >= end_time)
        {
            return false;
        }

        const auto num_vertices = to_vertex_assignment.size();

        std::vector<boost::dynamic_bitset<>> adjacency_matrix(num_vertices, boost::dynamic_bitset<>(num_vertices));

        for (const auto& assignment : statically_consistent_assignments)
        {
            const auto& first_assignment = assignment.first_assignment;
            const auto& second_assignment = assignment.second_assignment;

            if (literal_all_consistent(assignment_sets, flat_action_schema_.fluent_precondition, first_assignment, second_assignment))
            {
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

            mimir::formalism::ObjectList terms(flat_action_schema_.arity);

            for (std::size_t vertex_index = 0; vertex_index < flat_action_schema_.arity; ++vertex_index)
            {
                const auto vertex_id = clique[vertex_index];
                const auto& vertex_assignment = to_vertex_assignment.at(vertex_id);
                const auto parameter_index = vertex_assignment.parameter_index;
                const auto object_id = vertex_assignment.object_id;
                terms[parameter_index] = problem_->get_object(object_id);
            }

            const auto action = create_action(std::move(terms));

            if (mimir::formalism::literals_hold(action->get_precondition(), state, 3))
            {
                out_actions.push_back(action);
            }
        }

        return true;
    }

    mimir::formalism::ActionList LiftedSchemaSuccessorGenerator::general_case(const mimir::formalism::State& state,
                                                                              const std::vector<std::vector<bool>>& assignment_sets) const
    {
        mimir::formalism::ActionList applicable_actions;
        general_case(std::chrono::high_resolution_clock::time_point::max(), state, assignment_sets, applicable_actions);
        return applicable_actions;
    }

    mimir::formalism::ActionList LiftedSchemaSuccessorGenerator::get_applicable_actions(const mimir::formalism::State& state,
                                                                                        const std::vector<std::vector<bool>>& assignment_sets) const
    {
        if (!nullary_preconditions_hold(state))
        {
            return mimir::formalism::ActionList();
        }

        if (flat_action_schema_.arity == 0)
        {
            return nullary_case(state);
        }
        else if (flat_action_schema_.arity == 1)
        {
            return unary_case(state);
        }
        else
        {
            return general_case(state, assignment_sets);
        }
    }

    mimir::formalism::ActionList LiftedSchemaSuccessorGenerator::get_applicable_actions(const mimir::formalism::State& state) const
    {
        const auto assignment_sets = build_assignment_sets(domain_, problem_, state->get_dynamic_ranks());
        return get_applicable_actions(state, assignment_sets);
    }

    bool LiftedSchemaSuccessorGenerator::get_applicable_actions(const std::chrono::high_resolution_clock::time_point end_time,
                                                                const mimir::formalism::State& state,
                                                                mimir::formalism::ActionList& out_actions) const
    {
        if (flat_action_schema_.arity == 0)
        {
            if (!nullary_case(end_time, state, out_actions))
            {
                return false;
            }
        }

        if (flat_action_schema_.arity == 1)
        {
            if (!unary_case(end_time, state, out_actions))
            {
                return false;
            }
        }

        if (flat_action_schema_.arity > 1)
        {
            const auto assignment_sets = build_assignment_sets(domain_, problem_, state->get_dynamic_ranks());

            if (!general_case(end_time, state, assignment_sets, out_actions))
            {
                return false;
            }
        }

        return true;
    }
}  // namespace planners
