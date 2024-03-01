#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_BITSET_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_BITSET_LIFTED_HPP_

#include "../../algorithms/kpkc.hpp"
#include "../../formalism/problem/declarations.hpp"
#include "../../formalism/problem/ground_action.hpp"
#include "interface.hpp"
#include "internal_functions.hpp"
#include "internal_representation.hpp"

#include <boost/dynamic_bitset.hpp>
#include <unordered_map>
#include <vector>

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
    std::vector<FlatAction> m_flat_actions;
    std::unordered_map<Action, std::vector<std::vector<size_t>>> m_partitions;
    std::unordered_map<Action, std::vector<Assignment>> m_to_vertex_assignment;
    std::unordered_map<Action, std::vector<AssignmentPair>> m_statically_consistent_assignments;
    std::unordered_map<Action, std::unordered_map<size_t, std::vector<int>>> m_objects_by_parameter_type;

    ConstActionView create_ground_action(const FlatAction& flat_flat_action, ObjectList&& terms, PDDLFactories& ref_factories) const
    {
        throw std::runtime_error("not implemented");
    }

    GroundLiteral ground_literal(const Literal& literal, const ObjectList& terms, PDDLFactories& ref_factories) const
    {
        assert(literal->get_atom()->get_terms().size() == terms.size());
        auto grounded_atom = ref_factories.get_or_create_ground_atom(literal->get_atom()->get_predicate(), terms);
        auto grounded_literal = ref_factories.get_or_create_ground_literal(literal->is_negated(), grounded_atom);
        return grounded_literal;
    }

    bool nullary_literal_holds(const FlatLiteral& literal, ConstStateView state, PDDLFactories& ref_factories) const
    {
        assert(literal.arity == 0);
        return state.literal_holds(ground_literal(literal.source, {}, ref_factories));
    }

    bool nullary_preconditions_hold(const FlatAction& flat_action, ConstStateView state, PDDLFactories& ref_factories) const
    {
        for (const auto& literal : flat_action.fluent_precondition)
        {
            if (literal.arity == 0 && nullary_literal_holds(literal, state, ref_factories))
            {
                return false;
            }
        }

        return true;
    }

    void
    nullary_case(const FlatAction& flat_action, ConstStateView state, PDDLFactories& ref_factories, std::vector<ConstActionView>& out_applicable_actions) const
    {
        // There are no parameters, meaning that the preconditions are already fully ground. Simply check if the single ground action is applicable.

        const auto ground_action = create_ground_action(flat_action, {}, ref_factories);

        if (ground_action.is_applicable(state))
        {
            out_applicable_actions.emplace_back(ground_action);
        }
    }

    void
    unary_case(const FlatAction& flat_action, ConstStateView state, PDDLFactories& ref_factories, std::vector<ConstActionView>& out_applicable_actions) const
    {
        const auto& objects_by_parameter_type = m_objects_by_parameter_type.at(flat_action.source);

        for (const auto& object_id : objects_by_parameter_type.at(0))
        {
            auto ground_action = create_ground_action(flat_action, { ref_factories.objects.get(object_id) }, ref_factories);

            if (ground_action.is_applicable(state))
            {
                out_applicable_actions.emplace_back(ground_action);
            }
        }
    }

    void general_case(const std::vector<std::vector<bool>>& assignment_sets,
                      FlatAction flat_action,
                      ConstStateView state,
                      PDDLFactories& ref_factories,
                      std::vector<ConstActionView>& out_applicable_actions) const
    {
        const auto& to_vertex_assignment = m_to_vertex_assignment.at(flat_action.source);
        const auto& statically_consistent_assignments = m_statically_consistent_assignments.at(flat_action.source);
        const auto num_vertices = to_vertex_assignment.size();

        std::vector<boost::dynamic_bitset<>> adjacency_matrix(num_vertices, boost::dynamic_bitset<>(num_vertices));

        for (const auto& assignment : statically_consistent_assignments)
        {
            const auto& first_assignment = assignment.first_assignment;
            const auto& second_assignment = assignment.second_assignment;

            if (literal_all_consistent(assignment_sets, flat_action.fluent_precondition, first_assignment, second_assignment, m_problem))
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

        const auto& partitions = m_partitions.at(flat_action.source);
        std::vector<std::vector<std::size_t>> cliques;
        find_all_k_cliques_in_k_partite_graph(adjacency_matrix, partitions, cliques);

        for (const auto& clique : cliques)
        {
            auto terms = ObjectList(flat_action.arity);

            for (std::size_t vertex_index = 0; vertex_index < flat_action.arity; ++vertex_index)
            {
                const auto vertex_id = clique[vertex_index];
                const auto& vertex_assignment = to_vertex_assignment.at(vertex_id);
                const auto parameter_index = vertex_assignment.parameter_index;
                const auto object_id = vertex_assignment.object_id;
                terms[parameter_index] = ref_factories.objects.get(object_id);
            }

            const auto ground_action = create_ground_action(flat_action, std::move(terms), ref_factories);

            if (ground_action.is_applicable(state, 3))
            {
                out_applicable_actions.push_back(ground_action);
            }
        }
    }

    void generate_applicable_actions_impl(ConstStateView state, std::vector<ConstActionView>& out_applicable_actions)
    {
        out_applicable_actions.clear();

        // Build the assignment sets, which is shared between all action schemas

        std::vector<size_t> atom_ids;

        for (const auto& atom_id : state.get_atoms_bitset())
        {
            atom_ids.emplace_back(atom_id);
        }

        const auto assignment_sets = build_assignment_sets(m_problem, atom_ids, m_pddl_factories);

        // Get the applicable ground actions for all action schemas

        for (const auto& flat_action : m_flat_actions)
        {
            if (!nullary_preconditions_hold(flat_action, state, m_pddl_factories))
            {
                return;
            }

            if (flat_action.arity == 0)
            {
                return nullary_case(flat_action, state, m_pddl_factories, out_applicable_actions);
            }
            else if (flat_action.arity == 1)
            {
                return unary_case(flat_action, state, m_pddl_factories, out_applicable_actions);
            }
            else
            {
                return general_case(assignment_sets, flat_action, state, m_pddl_factories, out_applicable_actions);
            }
        }
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class IAAG;

public:
    AAG(Problem problem, PDDLFactories& pddl_factories) : m_problem(problem), m_pddl_factories(pddl_factories)
    {
        // Type information is used by the unary and general case

        for (const auto& flat_action : m_flat_actions)
        {
            std::vector<std::vector<size_t>> partitions;
            std::vector<Assignment> to_vertex_assignment;
            std::vector<AssignmentPair> statically_consistent_assignments;
            std::unordered_map<size_t, std::vector<int>> objects_by_parameter_type;

            if (flat_action.arity >= 1)
            {
                // Compatible objects by type

                for (const auto& parameter : flat_action.get_parameters())
                {
                    std::vector<int> compatible_objects;

                    for (const auto& object : problem->get_objects())
                    {
                        if (is_any_subtype_of(object->get_bases(), parameter->get_bases()))
                        {
                            compatible_objects.emplace_back(object->get_identifier());
                        }
                    }

                    objects_by_parameter_type.emplace(flat_action.get_parameter_index(parameter), std::move(compatible_objects));
                }
            }

            // The following is only used by the general case

            if (flat_action.arity >= 2)
            {
                // Create a mapping between indices and parameter-object assignments

                for (uint32_t parameter_index = 0; parameter_index < flat_action.arity; ++parameter_index)
                {
                    const auto& compatible_objects = objects_by_parameter_type.at(parameter_index);
                    std::vector<std::size_t> partition;

                    for (const auto& object_id : compatible_objects)
                    {
                        partition.push_back(to_vertex_assignment.size());
                        to_vertex_assignment.push_back(Assignment(parameter_index, object_id));
                    }

                    partitions.push_back(std::move(partition));
                }

                // Filter assignment based on static atoms
                const auto& static_predicates = problem->get_domain()->get_static_predicates();
                std::vector<size_t> static_atom_ids;

                for (const auto& literal : problem->get_initial_literals())
                {
                    if (literal->is_negated())
                    {
                        throw std::runtime_error("negative literals in the initial state is not supported");
                    }

                    const auto& atom = literal->get_atom();

                    if (std::count(static_predicates.begin(), static_predicates.end(), atom->get_predicate()))
                    {
                        static_atom_ids.emplace_back(atom->get_identifier());
                    }
                }

                const auto assignment_sets = build_assignment_sets(problem, static_atom_ids, pddl_factories);

                for (size_t first_id = 0; first_id < to_vertex_assignment.size(); ++first_id)
                {
                    for (size_t second_id = (first_id + 1); second_id < to_vertex_assignment.size(); ++second_id)
                    {
                        const auto& first_assignment = to_vertex_assignment.at(first_id);
                        const auto& second_assignment = to_vertex_assignment.at(second_id);

                        if (first_assignment.parameter_index != second_assignment.parameter_index)
                        {
                            if (literal_all_consistent(assignment_sets, flat_action.static_precondition, first_assignment, second_assignment, problem))
                            {
                                statically_consistent_assignments.push_back(AssignmentPair(first_id, first_assignment, second_id, second_assignment));
                            }
                        }
                    }
                }

                for (const auto& literal : flat_action.static_precondition)
                {
                    if (literal.arity == 0)
                    {
                        const auto negated = literal.source->is_negated();
                        const auto& atom = literal.source->get_atom();
                        auto contains = false;

                        for (const auto& initial_literal : problem->get_initial_literals())
                        {
                            if (atom->get_predicate() == initial_literal->get_atom()->get_predicate())
                            {
                                contains = true;
                                break;
                            }
                        }

                        if (contains == negated)
                        {
                            statically_consistent_assignments.clear();
                            break;
                        }
                    }
                }
            }

            m_partitions.emplace(flat_action.source, std::move(partitions));
            m_to_vertex_assignment.emplace(flat_action.source, std::move(to_vertex_assignment));
            m_objects_by_parameter_type.emplace(flat_action.source, std::move(objects_by_parameter_type));
            m_statically_consistent_assignments.emplace(flat_action.source, std::move(statically_consistent_assignments));
        }
    }
};

}  // namespace mimir

#endif
