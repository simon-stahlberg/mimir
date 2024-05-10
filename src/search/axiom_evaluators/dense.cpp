#include "mimir/search/axiom_evaluators/dense.hpp"

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

    const auto assignment_sets = AssignmentSet(m_problem, ground_atoms);
}

AE<AEDispatcher<DenseStateTag>>::AE(Problem problem, PDDLFactories& pddl_factories) : m_problem(problem), m_pddl_factories(pddl_factories) {}
}
