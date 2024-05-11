#include "mimir/search/axiom_evaluators/dense.hpp"

#include "mimir/search/axiom_evaluators/axiom_stratification.hpp"

namespace mimir
{
void AE<AEDispatcher<DenseStateTag>>::nullary_case(const Axiom& axiom, DenseState state, GroundAxiomList& out_applicable_axioms) {}

void AE<AEDispatcher<DenseStateTag>>::unary_case(const Axiom& axiom, DenseState state, GroundAxiomList& out_applicable_axioms) {}

void AE<AEDispatcher<DenseStateTag>>::general_case(const AssignmentSet& assignment_sets,
                                                   const Axiom& axiom,
                                                   DenseState state,
                                                   GroundAxiomList& out_applicable_axioms)
{
}

void AE<AEDispatcher<DenseStateTag>>::generate_and_apply_axioms_impl(FlatBitsetBuilder& ref_ground_atoms)
{
    // The more ground atoms we add, the more positions in the assignment set are being set to true.
    // Hence, one can probably do better bookkeeping for edge removal in the consistency graph
    // And iteratively add back previously removed edges
    // Lets start with a naive implementation where we compute an overapproximation of the ground axioms
    // to ensure that the translation yields correct axioms.

    std::cout << "generate_and_apply_axioms_impl" << std::endl;

    auto ground_atoms = GroundAtomList {};
    for (const auto& atom_id : ref_ground_atoms)
    {
        ground_atoms.push_back(m_pddl_factories.get_ground_atom(atom_id));
    }

    const auto assignment_sets = AssignmentSet(m_problem, ground_atoms);

    // Bookkeeping to avoid grounding identical bindings
    auto instantiated_groundings = std::unordered_map<Axiom, std::unordered_set<ObjectList, loki::hash_container_type<ObjectList>>> {};
    // Bookkeeping to readd removed statically consistent edges
    auto removed_statically_consistant_edges = std::unordered_map<Axiom, std::vector<consistency_graph::Edges>> {};
    // Note: when adding back edges, only strongly connected components readded edges must be considered for clique enumeration

    /* Fixed point computation */
    auto added_ground_atoms = GroundAtomList {};

    // Compute fixed point for partitioning
    do
    {
        added_ground_atoms.clear();

        for (const auto& partition : m_partitioning)
        {
            // Compute fixed point for partition
            auto before_processing_partition = added_ground_atoms.size();
            do
            {
                before_processing_partition = added_ground_atoms.size();

                for (const auto& axiom : partition)
                {
                    // TODO: Compute applicable axioms, apply them, update bookkeeping
                }

            } while (before_processing_partition != added_ground_atoms.size());
        }

    } while (!added_ground_atoms.empty());

    for (const auto& axiom : m_problem->get_axioms())
    {
        std::cout << "axiom: " << *axiom->get_literal() << std::endl;
        std::cout << "parameters: [";
        for (const auto& parameter : axiom->get_parameters())
        {
            std::cout << *parameter << " ,";
        }
        std::cout << "]" << std::endl;
        std::cout << "conditions: [";
        for (const auto& literal : axiom->get_conditions())
        {
            std::cout << *literal << " ,";
        }
        std::cout << "]" << std::endl;
    }
}

AE<AEDispatcher<DenseStateTag>>::AE(Problem problem, PDDLFactories& pddl_factories) : m_problem(problem), m_pddl_factories(pddl_factories)
{
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
}

}
