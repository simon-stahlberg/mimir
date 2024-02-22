#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_BITSET_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_BITSET_LIFTED_HPP_

#include "../../formalism/problem/declarations.hpp"
#include "interface.hpp"
#include "internal_functions.hpp"

namespace mimir
{

/**
 * Fully specialized implementation class.
 */
template<>
class AAG<AAGDispatcher<LiftedTag, BitsetStateTag>> : public IAAG<AAG<AAGDispatcher<LiftedTag, BitsetStateTag>>>
{
private:
    using ConstStateView = ConstView<StateDispatcher<BitsetStateTag, LiftedTag>>;
    using ConstActionView = ConstView<ActionDispatcher<LiftedTag, BitsetStateTag>>;

    Problem m_problem;
    PDDLFactories& m_pddl_factories;

    bool nullary_preconditions_hold(ConstStateView state)
    {
        throw std::runtime_error("not implemented");

        // for (const auto& literal : flat_action_schema_.fluent_precondition)
        // {
        //     if ((literal.arity == 0) && !mimir::formalism::literal_holds(literal.source, state))
        //     {
        //         return false;
        //     }
        // }

        // return true;
    }

    bool nullary_case(ConstStateView state, std::vector<ConstActionView>& out_applicable_actions) const
    {
        throw std::runtime_error("not implemented");

        // const auto action = create_action(mimir::formalism::ObjectList {});

        // if (mimir::formalism::literals_hold(action->get_precondition(), state))
        // {
        //     out_applicable_actions.push_back(action);
        // }

        // return true;
    }

    void unary_case(ConstStateView state, std::vector<ConstActionView>& out_applicable_actions) const
    {
        throw std::runtime_error("not implemented");

        // for (const auto& object_id : objects_by_parameter_type.at(0))
        // {
        //     if (std::chrono::high_resolution_clock::now() >= end_time)
        //     {
        //         return false;
        //     }

        //     const auto action = create_action({ problem_->get_object(object_id) });

        //     if (mimir::formalism::literals_hold(action->get_precondition(), state))
        //     {
        //         out_actions.push_back(action);
        //     }
        // }

        // return true;
    }

    void general_case(ConstStateView state, const std::vector<std::vector<bool>>& assignment_sets, std::vector<ConstActionView>& out_applicable_actions) const
    {
        throw std::runtime_error("not implemented");

        // const auto num_vertices = to_vertex_assignment.size();

        // std::vector<boost::dynamic_bitset<>> adjacency_matrix(num_vertices, boost::dynamic_bitset<>(num_vertices));

        // for (const auto& assignment : statically_consistent_assignments)
        // {
        //     const auto& first_assignment = assignment.first_assignment;
        //     const auto& second_assignment = assignment.second_assignment;

        //     if (literal_all_consistent(assignment_sets, flat_action_schema_.fluent_precondition, first_assignment, second_assignment))
        //     {
        //         const auto first_id = assignment.first_position;
        //         const auto second_id = assignment.second_position;
        //         auto& first_row = adjacency_matrix[first_id];
        //         auto& second_row = adjacency_matrix[second_id];
        //         first_row[second_id] = 1;
        //         second_row[first_id] = 1;
        //     }
        // }

        // // Find all cliques of size num_parameters whose labels denote complete assignments that might yield an applicable precondition. The relatively few
        // // atoms in the state (compared to the number of possible atoms) lead to very sparse graphs, so the number of maximal cliques of maximum size (#
        // // parameters) tends to be very small.

        // std::vector<std::vector<std::size_t>> cliques;
        // if (!algorithms::find_all_k_cliques_in_k_partite_graph(end_time, adjacency_matrix, partitions_, cliques))
        // {
        //     return false;
        // }

        // for (const auto& clique : cliques)
        // {
        //     if (std::chrono::high_resolution_clock::now() >= end_time)
        //     {
        //         return false;
        //     }

        //     mimir::formalism::ObjectList terms(flat_action_schema_.arity);

        //     for (std::size_t vertex_index = 0; vertex_index < flat_action_schema_.arity; ++vertex_index)
        //     {
        //         const auto vertex_id = clique[vertex_index];
        //         const auto& vertex_assignment = to_vertex_assignment.at(vertex_id);
        //         const auto parameter_index = vertex_assignment.parameter_index;
        //         const auto object_id = vertex_assignment.object_id;
        //         terms[parameter_index] = problem_->get_object(object_id);
        //     }

        //     const auto action = create_action(std::move(terms));

        //     if (mimir::formalism::literals_hold(action->get_precondition(), state, 3))
        //     {
        //         out_actions.push_back(action);
        //     }
        // }

        // return true;
    }

    void generate_applicable_actions_impl(ConstStateView state, std::vector<ConstActionView>& out_applicable_actions)
    {
        out_applicable_actions.clear();

        throw std::runtime_error("not implemented");

        // TODO: Loop over all action schemas, rather than assuming a specific one.

        // if (!nullary_preconditions_hold(state))
        // {
        //     return;
        // }

        // if (flat_action_schema_.arity == 0)
        // {
        //     return nullary_case(state, out_applicable_actions);
        // }
        // else if (flat_action_schema_.arity == 1)
        // {
        //     return unary_case(state, out_applicable_actions);
        // }
        // else
        // {
        //     const auto assignment_sets = build_assignment_sets(m_problem, state, m_pddl_factories);
        //     return general_case(state, assignment_sets, out_applicable_actions);
        // }
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class IAAG;

public:
    AAG(Problem problem, PDDLFactories& pddl_factories) : m_problem(problem), m_pddl_factories(pddl_factories)
    {
        throw std::runtime_error("not implemented");

        // // Type information is used by the unary and general case

        // if (flat_action_schema_.arity >= 1)
        // {
        //     // Compatible objects by type

        //     for (const auto& parameter : flat_action_schema_.get_parameters())
        //     {
        //         std::vector<uint32_t> compatible_objects;

        //         for (const auto& object : problem->objects)
        //         {
        //             if (mimir::formalism::is_subtype_of(object->type, parameter->type))
        //             {
        //                 compatible_objects.emplace_back(object->id);
        //             }
        //         }

        //         objects_by_parameter_type.emplace(flat_action_schema_.get_parameter_index(parameter), std::move(compatible_objects));
        //     }
        // }

        // // The following is only used by the general case

        // if (flat_action_schema_.arity >= 2)
        // {
        //     // Create a mapping between indices and parameter-object assignments

        //     for (uint32_t parameter_index = 0; parameter_index < flat_action_schema_.arity; ++parameter_index)
        //     {
        //         const auto& compatible_objects = objects_by_parameter_type.at(parameter_index);
        //         std::vector<std::size_t> partition;

        //         for (const auto& object_id : compatible_objects)
        //         {
        //             partition.push_back(to_vertex_assignment.size());
        //             to_vertex_assignment.push_back(Assignment(parameter_index, object_id));
        //         }

        //         partitions_.push_back(std::move(partition));
        //     }

        //     // Filter assignment based on static atoms
        //     const auto initial_state = mimir::formalism::create_state(problem->initial, problem);
        //     const auto assignment_sets = build_assignment_sets(domain_, problem_, initial_state->get_static_ranks());

        //     for (size_t first_id = 0; first_id < to_vertex_assignment.size(); ++first_id)
        //     {
        //         for (size_t second_id = (first_id + 1); second_id < to_vertex_assignment.size(); ++second_id)
        //         {
        //             const auto& first_assignment = to_vertex_assignment.at(first_id);
        //             const auto& second_assignment = to_vertex_assignment.at(second_id);

        //             if (first_assignment.parameter_index != second_assignment.parameter_index)
        //             {
        //                 if (literal_all_consistent(assignment_sets, flat_action_schema_.static_precondition, first_assignment, second_assignment))
        //                 {
        //                     statically_consistent_assignments.push_back(AssignmentPair(first_id, first_assignment, second_id, second_assignment));
        //                 }
        //             }
        //         }
        //     }

        //     // The previous code does not handle static nullary atoms correctly

        //     for (const auto& literal : flat_action_schema_.static_precondition)
        //     {
        //         if (literal.arity == 0)
        //         {
        //             const auto negated = literal.source->negated;
        //             const auto& atom = literal.source->atom;
        //             const auto contains = static_cast<bool>(std::count(problem_->initial.cbegin(), problem_->initial.cend(), atom));

        //             if (contains == negated)
        //             {
        //                 statically_consistent_assignments.clear();
        //                 break;
        //             }
        //         }
        //     }
        // }
    }
};

}  // namespace mimir

#endif
