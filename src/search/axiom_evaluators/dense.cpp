/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
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

#include "mimir/search/axiom_evaluators/dense.hpp"

#include "mimir/algorithms/kpkc.hpp"
#include "mimir/common/itertools.hpp"
#include "mimir/search/translations.hpp"

#include <boost/dynamic_bitset.hpp>

namespace mimir
{

bool AE<AEDispatcher<DenseStateTag>>::nullary_fluent_preconditions_hold(const Axiom& axiom, const FlatBitsetBuilder& state_atoms)
{
    for (const auto& literal : axiom->get_fluent_conditions())
    {
        if (literal->get_atom()->get_predicate()->get_arity() == 0)
        {
            const auto grounded_literal = m_pddl_factories.ground_fluent_literal(literal, {});

            if (state_atoms.get(grounded_literal->get_atom()->get_identifier()) == grounded_literal->is_negated())
            {
                return false;
            }
        }
    }

    return true;
}

void AE<AEDispatcher<DenseStateTag>>::nullary_case(const Axiom& axiom, const FlatBitsetBuilder& state_atoms, DenseGroundAxiomList& out_applicable_axioms)
{
    // There are no parameters, meaning that the preconditions are already fully ground. Simply check if the single ground axiom is applicable.

    const auto grounded_axiom = ground_axiom(axiom, {});

    if (grounded_axiom.is_applicable(state_atoms, m_problem->get_static_initial_positive_atoms_bitset()))
    {
        m_applicable_axioms.insert(grounded_axiom);
        out_applicable_axioms.emplace_back(grounded_axiom);
    }
    else
    {
        m_event_handler->on_ground_inapplicable_axiom(grounded_axiom, m_pddl_factories);
    }
}

void AE<AEDispatcher<DenseStateTag>>::unary_case(const AssignmentSet<Fluent>& assignment_sets,
                                                 const Axiom& axiom,
                                                 const FlatBitsetBuilder& state_atoms,
                                                 DenseGroundAxiomList& out_applicable_axioms)
{
    const auto& precondition_graph = m_static_consistency_graphs.at(axiom);

    for (const auto& vertex : precondition_graph.get_vertices())
    {
        if (assignment_sets.literal_all_consistent(axiom->get_fluent_conditions(), vertex))
        {
            auto grounded_axiom = ground_axiom(axiom, { m_pddl_factories.get_object(vertex.get_object_index()) });

            if (grounded_axiom.is_applicable(state_atoms, m_problem->get_static_initial_positive_atoms_bitset()))
            {
                m_applicable_axioms.insert(grounded_axiom);
                out_applicable_axioms.emplace_back(grounded_axiom);
            }
            else
            {
                m_event_handler->on_ground_inapplicable_axiom(grounded_axiom, m_pddl_factories);
            }
        }
    }
}

void AE<AEDispatcher<DenseStateTag>>::general_case(const AssignmentSet<Fluent>& assignment_sets,
                                                   const Axiom& axiom,
                                                   const FlatBitsetBuilder& state_atoms,
                                                   DenseGroundAxiomList& out_applicable_axioms)
{
    const auto& precondition_graph = m_static_consistency_graphs.at(axiom);
    const auto num_vertices = precondition_graph.get_vertices().size();

    std::vector<boost::dynamic_bitset<>> adjacency_matrix(num_vertices, boost::dynamic_bitset<>(num_vertices));

    // D: Restrict statically consistent assignments based on the assignments in the current state
    //    and build the consistency graph as an adjacency matrix
    // Only iterate the edges that are currently part of the graph
    // Edges that were already removed we do not need to check again.
    for (const auto& edge : precondition_graph.get_edges())
    {
        if (assignment_sets.literal_all_consistent(axiom->get_fluent_conditions(), edge))
        {
            const auto first_id = edge.get_src().get_id();
            const auto second_id = edge.get_dst().get_id();
            auto& first_row = adjacency_matrix[first_id];
            auto& second_row = adjacency_matrix[second_id];
            first_row[second_id] = 1;
            second_row[first_id] = 1;
        }
    }

    // Find all cliques of size num_parameters whose labels denote complete assignments that might yield an applicable precondition. The relatively few
    // atoms in the state (compared to the number of possible atoms) lead to very sparse graphs, so the number of maximal cliques of maximum size (#
    // parameters) tends to be very small.

    const auto& partitions = precondition_graph.get_vertices_by_parameter_index();
    std::vector<std::vector<std::size_t>> cliques;
    find_all_k_cliques_in_k_partite_graph(adjacency_matrix, partitions, cliques);

    for (const auto& clique : cliques)
    {
        auto terms = ObjectList(axiom->get_arity());

        for (std::size_t vertex_index = 0; vertex_index < axiom->get_arity(); ++vertex_index)
        {
            const auto vertex_id = clique[vertex_index];
            const auto& vertex = precondition_graph.get_vertices()[vertex_id];
            const auto parameter_index = vertex.get_param_index();
            const auto object_id = vertex.get_object_index();
            terms[parameter_index] = m_pddl_factories.get_object(object_id);
        }

        const auto grounded_axiom = ground_axiom(axiom, std::move(terms));

        // TODO: We do not need to check applicability if axiom consists of at most binary predicates in the precondition.
        // Add this information to the FlatAction struct.

        if (grounded_axiom.is_applicable(state_atoms, m_problem->get_static_initial_positive_atoms_bitset()))
        {
            m_applicable_axioms.insert(grounded_axiom);
            out_applicable_axioms.push_back(grounded_axiom);
        }
        else
        {
            m_event_handler->on_ground_inapplicable_axiom(grounded_axiom, m_pddl_factories);
        }
    }
}

void AE<AEDispatcher<DenseStateTag>>::generate_and_apply_axioms_impl(FlatBitsetBuilder& ref_state_atoms)
{
    /* 1. Initialize assignment set */

    m_event_handler->on_start_generating_applicable_axioms();

    auto ground_atoms = GroundAtomList<Fluent> {};
    for (const auto& atom_id : ref_state_atoms)
    {
        ground_atoms.push_back(m_pddl_factories.get_fluent_ground_atom(atom_id));
    }

    auto fluent_predicates = m_problem->get_domain()->get_fluent_predicates();
    const auto& domain_derived_predicates = m_problem->get_domain()->get_derived_predicates();
    fluent_predicates.insert(fluent_predicates.end(), domain_derived_predicates.begin(), domain_derived_predicates.end());
    const auto& problem_derived_predicates = m_problem->get_derived_predicates();
    fluent_predicates.insert(fluent_predicates.end(), problem_derived_predicates.begin(), problem_derived_predicates.end());

    auto assignment_sets = AssignmentSet<Fluent>(m_problem, fluent_predicates, ground_atoms);

    /* 2. Initialize bookkeeping */

    // Bookkeeping to readd removed statically consistent edges
    auto removed_statically_consistant_edges = std::unordered_map<Axiom, std::vector<consistency_graph::Edges>> {};
    // Note: when adding back edges, only strongly connected components readded edges must be considered for clique enumeration

    /* 3. Fixed point computation */

    auto applicable_axioms = DenseGroundAxiomList {};

    for (const auto& partition : m_partitioning)
    {
        bool reached_partition_fixed_point;

        // Optimization 1: Track new ground atoms to simplify the clique enumeration graph in the next iteration.
        auto new_ground_atoms = ground_atoms;

        // TODO: Optimization 4: Inductively compile away axioms with static bodies. (grounded in match tree?)

        // Optimization 2: Track axioms that might trigger in next iteration.
        // Optimization 3: Use precomputed set of axioms that might be applicable initially
        auto relevant_axioms = partition.get_initially_relevant_axioms();

        do
        {
            reached_partition_fixed_point = true;

            /* Compute applicable axioms */

            for (const auto& axiom : relevant_axioms)
            {
                if (nullary_fluent_preconditions_hold(axiom, ref_state_atoms))
                {
                    if (axiom->get_arity() == 0)
                    {
                        nullary_case(axiom, ref_state_atoms, applicable_axioms);
                    }
                    else if (axiom->get_arity() == 1)
                    {
                        unary_case(assignment_sets, axiom, ref_state_atoms, applicable_axioms);
                    }
                    else
                    {
                        general_case(assignment_sets, axiom, ref_state_atoms, applicable_axioms);
                    }
                }
            }

            /* Apply applicable axioms */

            new_ground_atoms.clear();
            relevant_axioms.clear();

            for (const auto& grounded_axiom : applicable_axioms)
            {
                assert(!grounded_axiom.get_simple_effect().is_negated);

                const auto grounded_atom_id = grounded_axiom.get_simple_effect().atom_id;

                if (!ref_state_atoms.get(grounded_atom_id))
                {
                    // GENERATED NEW DERIVED ATOM!
                    const auto new_ground_atom = m_pddl_factories.get_fluent_ground_atom(grounded_atom_id);
                    reached_partition_fixed_point = false;

                    // TODO: Optimization 5: Update new ground atoms to speed up successive iterations, i.e.,
                    // only cliques that takes these new atoms into account must be computed.
                    new_ground_atoms.push_back(new_ground_atom);

                    // Update the assignment set
                    assignment_sets.insert_ground_atom(new_ground_atom);

                    // Retrieve relevant axioms
                    partition.retrieve_axioms_with_same_body_predicate(new_ground_atom, relevant_axioms);
                }

                ref_state_atoms.set(grounded_atom_id);
            }

        } while (!reached_partition_fixed_point);
    }

    m_event_handler->on_end_generating_applicable_axioms(applicable_axioms, m_pddl_factories);
}

AE<AEDispatcher<DenseStateTag>>::AE(Problem problem, PDDLFactories& pddl_factories, std::shared_ptr<ILiftedAAGEventHandler> event_handler) :
    m_problem(problem),
    m_pddl_factories(pddl_factories),
    m_event_handler(std::move(event_handler))
{
    /* 1. Error checking */

    for (const auto& literal : problem->get_fluent_initial_literals())
    {
        if (literal->is_negated())
        {
            throw std::runtime_error("Negative literals in the initial state is not supported.");
        }
    }

    for (const auto& literal : problem->get_static_initial_literals())
    {
        if (literal->is_negated())
        {
            throw std::runtime_error("Negative literals in the initial state is not supported.");
        }
    }

    /* 2. Initialize axiom partitioning using stratification */

    auto axioms = m_problem->get_domain()->get_axioms();
    axioms.insert(axioms.end(), m_problem->get_axioms().begin(), m_problem->get_axioms().end());

    for (const auto& axiom : axioms)
    {
        if (axiom->get_literal()->is_negated())
        {
            throw std::runtime_error("Negative literals in axiom heads is not supported.");
        }
    }

    auto derived_predicates = m_problem->get_domain()->get_derived_predicates();
    derived_predicates.insert(derived_predicates.end(), m_problem->get_derived_predicates().begin(), m_problem->get_derived_predicates().end());

    m_partitioning = compute_axiom_partitioning(axioms, derived_predicates);

    /* 3. Initialize static consistency graph */

    auto static_initial_atoms = GroundAtomList<Static> {};
    to_ground_atoms(m_problem->get_static_initial_literals(), static_initial_atoms);
    const auto static_assignment_set = AssignmentSet<Static>(m_problem, m_problem->get_domain()->get_static_predicates(), static_initial_atoms);

    for (const auto& axiom : axioms)
    {
        m_static_consistency_graphs.emplace(
            axiom,
            consistency_graph::StaticConsistencyGraph(m_problem, 0, axiom->get_arity(), axiom->get_static_conditions(), static_assignment_set));
    }
}

const std::vector<AxiomPartition>& AE<AEDispatcher<DenseStateTag>>::get_axiom_partitioning() const { return m_partitioning; }

DenseGroundAxiom AE<AEDispatcher<DenseStateTag>>::ground_axiom(const Axiom& axiom, ObjectList&& binding)
{
    /* 1. Check if grounding is cached */

    auto& groundings = m_axiom_groundings[axiom];
    auto it = groundings.find(binding);
    if (it != groundings.end())
    {
        m_event_handler->on_ground_axiom_cache_hit(axiom, binding);

        return it->second;
    }

    m_event_handler->on_ground_axiom_cache_miss(axiom, binding);

    /* 2. Ground the axiom */

    m_event_handler->on_ground_axiom(axiom, binding);

    const auto fill_fluent_bitsets = [this](const std::vector<Literal<Fluent>>& literals,
                                            FlatBitsetBuilder& ref_positive_bitset,
                                            FlatBitsetBuilder& ref_negative_bitset,
                                            const auto& binding)
    {
        for (const auto& literal : literals)
        {
            const auto grounded_literal = m_pddl_factories.ground_fluent_literal(literal, binding);

            if (grounded_literal->is_negated())
            {
                ref_negative_bitset.set(grounded_literal->get_atom()->get_identifier());
            }
            else
            {
                ref_positive_bitset.set(grounded_literal->get_atom()->get_identifier());
            }
        }
    };

    const auto fill_static_bitsets = [this](const std::vector<Literal<Static>>& literals,
                                            FlatBitsetBuilder& ref_positive_bitset,
                                            FlatBitsetBuilder& ref_negative_bitset,
                                            const auto& binding)
    {
        for (const auto& literal : literals)
        {
            const auto grounded_literal = m_pddl_factories.ground_static_literal(literal, binding);

            if (grounded_literal->is_negated())
            {
                ref_negative_bitset.set(grounded_literal->get_atom()->get_identifier());
            }
            else
            {
                ref_positive_bitset.set(grounded_literal->get_atom()->get_identifier());
            }
        }
    };

    /* Header */

    m_axiom_builder.get_id() = m_axioms.size();
    m_axiom_builder.get_axiom() = axiom;
    auto& objects = m_axiom_builder.get_objects();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj);
    }

    /* Precondition */
    auto strips_precondition_proxy = DenseStripsActionPreconditionBuilderProxy(m_axiom_builder.get_strips_precondition());
    auto& positive_precondition = strips_precondition_proxy.get_positive_fluent_precondition();
    auto& negative_precondition = strips_precondition_proxy.get_negative_fluent_precondition();
    auto& positive_static_precondition = strips_precondition_proxy.get_positive_static_precondition();
    auto& negative_static_precondition = strips_precondition_proxy.get_negative_static_precondition();
    positive_precondition.unset_all();
    negative_precondition.unset_all();
    positive_static_precondition.unset_all();
    negative_static_precondition.unset_all();
    fill_fluent_bitsets(axiom->get_fluent_conditions(), positive_precondition, negative_precondition, binding);
    fill_static_bitsets(axiom->get_static_conditions(), positive_static_precondition, negative_static_precondition, binding);

    /* Effect */
    const auto grounded_literal = m_pddl_factories.ground_fluent_literal(axiom->get_literal(), binding);
    assert(!grounded_literal->is_negated());
    m_axiom_builder.get_simple_effect().is_negated = false;
    m_axiom_builder.get_simple_effect().atom_id = grounded_literal->get_atom()->get_identifier();

    auto& flatmemory_builder = m_axiom_builder.get_flatmemory_builder();
    flatmemory_builder.finish();

    const auto [iter, inserted] = m_axioms.insert(flatmemory_builder);
    const auto grounded_axiom = DenseGroundAxiom(*iter);

    if (inserted)
    {
        m_axioms_by_index.push_back(grounded_axiom);
    }

    // Ensure that buffer is interpretable back to same data as builder
    assert(flatmemory_builder == *iter);

    /* 3. Insert to groundings table */

    groundings.emplace(std::move(binding), DenseGroundAxiom(grounded_axiom));

    /* 4. Return the resulting ground axiom */

    return grounded_axiom;
}

const FlatDenseAxiomSet& AE<AEDispatcher<DenseStateTag>>::get_axioms() const { return m_axioms; }

const DenseGroundAxiomSet& AE<AEDispatcher<DenseStateTag>>::get_applicable_axioms() const { return m_applicable_axioms; }
}
